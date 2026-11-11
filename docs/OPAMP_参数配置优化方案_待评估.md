# XHT-Nexo-13S2P 内部运放（OPAMP）参数配置优化方案（待评估）

> 状态：**仅方案，未修改任何代码文件**
> 依据：`F:\ProjDoc\XHT\doc\原理图\XHT-Nexo-13S2P-MAIN-A.pdf`（A 版）、N32G45x《用户手册》第 27 章（印刷 p730–741）、`Source/` 现有固件
> 生成日期：2026-11-09

---

## 0. 结论摘要

**现有运放模式配置是对的，不要改。**

固件用 `OPAMP_CS_EXT_OPAMP`（MOD=00，外部放大模式），配合原理图上的 R57/R62/R63/R64 外部闭环网络与 PC4(COMM) 中点偏置，这与手册 27.2.1 及硬件完全吻合。**改成手册 27.2.3 的内部 PGA 模式是错的**（PGA 要求 VM 引脚浮空，本板 VM=PA3 接了外部网络，且中点偏置会丢失）。

真正值得改的是 4 件事，按优先级：

| 优先级 | 事项 | 性质 | 风险 |
|---|---|---|---|
| P0 | 显式配置全部 OPAMPx_CS 字段 + 主/从 MUX 一致 | 防御性、零行为变化 | 极低 |
| P0 | 初始化时序：先使能运放并等待建立，再启动 ADC / 校零点 | 修正潜在零点偏差 | 低 |
| P1 | 休眠时 `OPAMP_Enable(DISABLE)`，唤醒由 `HwCompOpamInit()` 恢复 | 降低静态功耗 | 低 |
| P2 | 换算系数定点化 + 收窄零点窗口 + 补 OPA 增益标定 | 提升精度 | 中（需同步改 7 处耦合点） |

---

## 1. 现状盘点（逐项核对）

### 1.1 运放寄存器配置 — `Source/BMS/MCU/McuHal_N32G45X.c:1129–1143`

```c
void HwCompOpamInit(void)
{
    OPAMP_InitType OPAMP_Initial;
    OPAMP_StructInit(&OPAMP_Initial);
    RCC_EnableAPB1PeriphClk(RCC_APB1_PERIPH_OPAMP, ENABLE);

    OPAMP_Initial.Mod = OPAMP_CS_EXT_OPAMP;        // MOD = 00 外部放大模式
    OPAMP_Init(OPAMP1, &OPAMP_Initial);

    OPAMP_SetVpSel(OPAMP1, OPAMP1_CS_VPSEL_PA7);   // V+ = PA7 (C_VINP)
    OPAMP_SetVmSel(OPAMP1, OPAMP1_CS_VMSEL_PA3);   // V- = PA3 (C_VINM)
    OPAMP_Enable(OPAMP1, ENABLE);                  // 输出 PA6 -> ADC1_CH3
}
```

`OPAMP_StructInit()` 的默认值（`n32g45x_opamp.c:96–102`）：

| 字段 | 默认值 | 手册位域 | 评价 |
|---|---|---|---|
| `Gain` | `OPAMP_CS_PGA_GAIN_2` | PGAGAN[2:0] | 外部模式下**无效**，但隐式依赖默认值 |
| `HighVolRangeEn` | `ENABLE` | RANGE=1 | ✅ 正确（VDDA=3.3V ≥ 2.4V，手册 p739） |
| `TimeAutoMuxEn` | `DISABLE` | TCMEN=0 | ✅ 正确（未用 TIM1_CC6 切换） |
| `Mod` | `PGA_EN`，被显式覆盖为 `EXT_OPAMP` | MOD[1:0]=00 | ✅ 正确（见第 2 节） |
| VPSSEL / VMSSEL | **未配置**，保持复位值 PA1 / PA3 | 从 MUX | ⚠️ 隐患：一旦误开 TCMEN，输入会切到 PA1/PA3 |
| CALON | 未调用 | 校准 | 依赖出厂校准，未做运行时校准 |
| OPAMP_LOCK | 未调用 | 写保护 | 未加锁（当前合理，见 3.1） |

### 1.2 硬件与外设映射（已从原理图 A 版逐点核对）

| 项目 | 实际值 | 代码位置 |
|---|---|---|
| OPAMP 实例 | OPAMP1 | `McuHal_N32G45X.c:1137` |
| V+ / V- / OUT | PA7(C_VINP) / PA3(C_VINM) / PA6(VOUT) | 同上 :1139–1140 |
| ADC 通道 | `ADC1_Channel_03_PA6` = 0x03（即 ADC1_CH3，手册 27.1.1 规定的 OPAMP1 输出通道） | `AdcM.h:14`、`AdcM.c:28` |
| GPIO 模拟输入 | PA 口 `0x00DC` = PA2/3/4/6/7 | `McuHal_N32G45X.c:395` |
| 采样电阻 | 0.33 mΩ = 1mΩ∥1mΩ∥1mΩ（R90/R91/R92） | `board.h:80` 注释 |
| 输入 RC | R63/R64 = 1K，C101 = 0.1 µF（C79 为 NC） | 原理图「MCU电流检测」块 |
| 闭环/偏置网络 | R57 = 51K±1%、R62 = 24K±1%、C116 = 1nF、C124 = 0.01 µF | 同上 |
| 中点偏置 | `OPA_BIAS` = **PC4 (COMM)**，推挽输出高 → VDDA | `board.h:117–118,178–179`；`McuHal_N32G45X.c:536`；`PowerCtrl.c:124/226` 休眠关/唤醒开 |
| ADC 采样时间 | `ADC_SAMP_TIME_239CYCLES5`（最长档） | `AdcM.c:77` ✅ |

### 1.3 电流换算与标定链

| 环节 | 代码 | 说明 |
|---|---|---|
| 零点默认值 | `CHG_CUR_ZERO_DEFAULT 2034`，窗口 ±516 | `User.h:239–241`；2034/4096×3.3≈1.64 V 中点 |
| 零点实测 | `CalibCurBias()`：8 次 `ADVal[CH_OPAM_CUR]` 平均，越界回退 DEFAULT | `Calib.c:159–178`，`CalibInit():214` 调用 |
| 换算系数 | `OPA_CUR_GAIN 117` （mA/LSB） | `board.h:80`；注释 `=AD/4096*3300*1000/(0.33mR*20.8)` |
| 电流计算 | `CurData = ChgCurZero - avgAD; OpaCurrent = CurData * 117` (mA) | `BatterCheck.c:394–404`（1 ms 调度，`Main.c:132–134` `#ifdef OPA_COC_EN`） |
| 结果使用 | `BatOpaCurGet()` → 并包策略 ±1000 mA 判断、快速 MOS 控制、上报 | `multiCan.c:1078/1229/1541/2010/2019/2114/2123` |
| 过流保护 | `Charge.c:546–567` 整段 `#if 0` **已禁用** | `User.h:99 CHG_OVER_CUR_AD 480` 目前不生效 |
| 上报换算 | `ChgCurAvgAD * OPA_CUR_GAIN` | `CanBW.c:379`、`UartBW.c:596/1333` |

> 注：主电流精度、库仑计与硬件过流保护由 AFE（KC81710H）负责；OPA 这路用于**软件复核 + 并包策略判断 + 上报**，精度要求约 1 A 量级。

---

## 2. 为什么必须保持「外部放大模式」（关键论证）

**手册约束（27.2.3 / p736）**：内部 PGA 模式要求 `MOD=10` 且「`VMSSEL`/`VMSEL` 引脚必须设置为**浮空**」，增益由 `PGAGAN` 取 2/4/8/16/32。
**本板实际**：VM = PA3 接了 R63(1K)→ISP1 与 R62(24K)→VOUT，并非浮空；中点偏置由 PC4(COMM) 经 R57(51K) 注入。改 PGA 会同时丢掉闭环反馈与中点偏置，运放输出只能单向摆动（手册 p730：输出 0.1 V ~ VDDA-0.1 V），**无法测双向电流**。

**按外部网络反推（验证现有 20.8 倍的合理性）**：

- V+ 由 ISN1 经 R64(1K)、COMM 经 R57(51K) 分压 → V+ ≈ (51·V_ISN1 + 1·VDDA)/52
- V− 由 ISP1 经 R63(1K)、VOUT 经 R62(24K) 分压 → V− ≈ (24·V_ISP1 + 1·VOUT)/25
- 虚短得：`VOUT ≈ 24.5·V_ISN1 − 24·V_ISP1 + 0.48·VDDA`

即：**差分增益约 24，零点 ≈ 0.48×3.3 = 1.59 V**（代码 1.64 V / 2034 LSB，吻合）。
固件采用 **20.8 倍**（经验/实测修正，含电阻公差、PCB 与运放非理想），故 1 LSB = 3300/4096 mV ÷ (20.8×0.33 mΩ) ≈ **117.4 mA**，取整为 117 mA/LSB。

> 结论：增益由**外部电阻网络**决定，不在寄存器里。想改增益必须改硬件电阻（R57/R62/R63/R64），并同步重算第 5 节全部耦合点。

---

## 3. 推荐方案（A 档：最小改动）

### 3.1 `HwCompOpamInit()` 显式化配置（建议值）

| 参数 | 当前 | **建议值** | 理由 |
|---|---|---|---|
| `RCC_APB1_PERIPH_OPAMP` | ENABLE | **ENABLE**（保持） | 手册要求先开时钟再写寄存器 |
| `Mod` | `OPAMP_CS_EXT_OPAMP` | **保持 `OPAMP_CS_EXT_OPAMP` (MOD=00)** | 与硬件匹配，见第 2 节 |
| `Gain` | 隐式默认 `PGA_GAIN_2` | **显式赋 `OPAMP_CS_PGA_GAIN_2`** | 外部模式下无效，但显式化可防止日后误改 Mod 时增益突变 |
| `HighVolRangeEn` | ENABLE | **ENABLE**（保持） | VDDA=3.3 V，手册 p739 RANGE=1 高电压区间 |
| `TimeAutoMuxEn` | DISABLE | **DISABLE**（保持） | 未使用 TIM1_CC6 输入切换 |
| `VPSEL` / `VMSEL` | PA7 / PA3 | **PA7 / PA3**（保持） | 手册 p740 枚举：OPAMP1 VPSEL=011→PA7、VMSEL=00→PA3 |
| `VPSSEL` / `VMSSEL` | 未配置（复位 PA1/PA3） | **新增 `OPAMP1_CS_VPSSEL_PA7` / `OPAMP1_CS_VMSSEL_PA3`** | 主/从 MUX 一致，防止误开 TCMEN 时输入错切；TCMEN=0 时无行为变化 |
| `CALON` | 未使用 | **保持不用** | 手册 27.2.5：出厂已校正，具体方法需联系原厂；零点靠 `CalibCurBias()` 软件补偿更可控 |
| `OPAMP_LOCK` | 未加锁 | **保持不加锁** | 手册 27.2.6：复位后仅能写一次且不可撤销；`HwSleepExit():1684` 会重新 Init，加锁会冲突 |
| 顺序 | Init→Vp/Vm→Enable | **Init→Vp/Vm→VpSSel/VmSSel→Enable→延时** | 见 3.2 |

示意（**仅方案，未应用**）：

```c
void HwCompOpamInit(void)
{
    OPAMP_InitType op;
    OPAMP_StructInit(&op);
    RCC_EnableAPB1PeriphClk(RCC_APB1_PERIPH_OPAMP, ENABLE);

    op.Mod              = OPAMP_CS_EXT_OPAMP;      /* 外部电阻闭环，勿改为 PGA */
    op.Gain             = OPAMP_CS_PGA_GAIN_2;     /* 显式；EXT 模式下不生效 */
    op.HighVolRangeEn   = ENABLE;                  /* VDDA=3.3V */
    op.TimeAutoMuxEn    = DISABLE;
    OPAMP_Init(OPAMP1, &op);

    OPAMP_SetVpSel(OPAMP1, OPAMP1_CS_VPSEL_PA7);          /* C_VINP */
    OPAMP_SetVmSel(OPAMP1, OPAMP1_CS_VMSEL_PA3);          /* C_VINM */
    OPAMP_SetVpSecondSel(OPAMP1, OPAMP1_CS_VPSSEL_PA7);   /* 主/从一致 */
    OPAMP_SetVmSecondSel(OPAMP1, OPAMP1_CS_VMSSEL_PA3);

    OPAMP_Enable(OPAMP1, ENABLE);
    Delay1ms(2);                                          /* 等待闭环建立 + 输出稳定 */
}
```

### 3.2 初始化时序修正（P0）

现状 `HwMcuInit()`（`:1704–1709`）为 `ADCInit()` → … → `HwCompOpamInit()`；而 `ADCInit()` 内部 `ADC_InitInstance()` 末尾即 `ADC_EnableSoftwareStartConv()`（AdcM.c:106），**ADC 在运放使能前已开始连续转换**。随后 `Main.c:240 CalibInit()` → `CalibCurBias()` 取零点。

建议：
1. 将 `HwCompOpamInit()` 提前到 `ADCInit()` **之前**（或至少在 `ADC_EnableSoftwareStartConv()` 之前）；
2. 在 `HwCompOpamInit()` 末尾加 `Delay1ms(2)`（R64×C101 = 0.1 ms 量级，反馈电容 C116/C124 需更长建立时间，2 ms 有充足余量）；
3. `CalibCurBias()` 丢弃前 2 次采样再平均（现有 8 次 ×2 ms 已足够，此项为可选加固）。

风险：极低；收益：消除上电/唤醒后首帧零点偏差。

### 3.3 低功耗（P1）

现状：`PowerCtrl.c:124` 休眠前已 `OPA_BIAS_OFF()`，但
- `McuClkLpConfig():1498` 中 `RCC_EnableAPB1PeriphClk(RCC_APB1_PERIPH_OPAMP, cmdstate)` **被注释**；
- `HwSleepEnter():1653` 中 `OPAMP_DeInit()` **被注释**；
→ 休眠时运放仍处于使能状态，持续耗电。

建议：在 `HwSleepEnter()` 的 `OPA_BIAS_OFF()` 之后加 `OPAMP_Enable(OPAMP1, DISABLE)`；唤醒侧 `HwSleepExit():1684` 已调用 `HwCompOpamInit()`，会自动重新使能，无需额外处理。若还要进一步省电，可再补时钟开关（注意须在 Enable 之前打开）。

---

## 4. 可选增强（B 档：改动大，需单独评估）

1. **OPA 这路没有增益标定**：现有 `CalibDsgCur/CalibChgCur`（`Calib.c:121–157`）只校准 AFE 的 `DATA_CUR_K_*`，OPA 仅有零点 `ChgCurZero`。建议增加两点标定，Flash 存 `K_OPA`（定点 mA/LSB）与 `CODE_0`，固件用 `I = K_OPA × (CODE − CODE_0)`，一次性消除 1% 电阻公差、采样电阻误差与 VDDA 偏差。
2. **用 VREFINT 实测 VDDA**：偏置 COMM 来自 VDDA、ADC 参考也是 VDDA，故**零点 AD 值与 VDDA 无关**（比率测量，优点），但**灵敏度与 VDDA 成反比**，VDDA ±2% 会带来 ±2% 电流误差。`BatterCheck.c:55 BatReadVref2Vdd()` 有骨架但实现被注释且 `MCU_VDD_CHECK` 未启用，可考虑启用后修正 K。
3. **系数定点化**：`OPA_CUR_GAIN 117` 由理论 117.4 截断而来，误差 −0.32%。可改为 `1174`（0.1 mA/LSB）并在计算末尾 `/10`。**必须同步修改所有 7 处引用**（见第 5 节）。
4. **收窄零点合法窗口**：`CHG_CUR_ZERO_MAX/MIN = 2034±516` 对应 ±516 LSB×117 mA ≈ **±60 A**。窗口过宽，偏置异常/运放失效时可能把错误值当合法零点，掩盖故障。建议按手册失调量级与电阻公差收窄到 **±150~200 LSB**（≈±18~23 A），并把越界处理从「回退默认值」升级为「上报故障」。

---

## 5. 不推荐方案（C 档）及理由

**改为内部 PGA 模式（`MOD=10`, `PGAGAN=16/32`）**：

- 手册 p736 明确要求 VM 引脚浮空，本板 VM=PA3 有外部闭环网络 → 违反约束；
- 外部电阻提供的 **PC4(COMM) 中点偏置**将失效，输出无法表达充放电双向电流；
- 增益从 20.8 变为 16/32 后，以下耦合点**全部需要重标定**：`OPA_CUR_GAIN`、`CHG_CUR_ZERO_*`、`CHG_OVER_CUR_AD`、`multiCan.c` 的 ±1000 mA 判断与 `opacur10` 上报、`CanBW.c`/`UartBW.c` 上报换算；
- 收益（略高的分辨率）远小于风险，且当前 OPA 只用于 1 A 量级的策略判断，分辨率并非瓶颈。

---

## 6. 影响面清单（改任何增益/系数前必须同步复核）

| # | 文件:行 | 内容 |
|---|---|---|
| 1 | `BMS/board.h:80` | `OPA_CUR_GAIN 117`（mA/LSB） |
| 2 | `BMS/User.h:239–241` | `CHG_CUR_ZERO_DEFAULT 2034`、`±516` |
| 3 | `BMS/User.h:99` | `CHG_OVER_CUR_AD 480`（注释写「16A / 7mV*25」，按 117 mA/LSB 实为 **≈56 A**，注释与实现不符，建议先修注释） |
| 4 | `BMS/BatterCheck.c:394–404` | 主换算 `(ChgCurZero − avgAD) × OPA_CUR_GAIN` |
| 5 | `BMS/Charge.c:546–567` | `#if 0` 的 MCU 运放过流保护段（当前禁用；启用需重设阈值） |
| 6 | `BMS/multiCan.c:1078/1229/1541/2010/2019/2114/2123` | `BatOpaCurGet()` 的 ±1000 mA 阈值与 10 mA/LSB 上报 |
| 7 | `BMS/CanBW.c:379`、`BMS/UartBW.c:596/1333` | 上位机/CAN 上报换算 |
| 8 | `BMS/Calib.c:159–178` | `CalibCurBias()` 零点采样 |
| 9 | `BMS/MCU/McuHal_N32G45X.c:1129/1684/1709` | 初始化、休眠退出、上电顺序 |

---

## 7. 验证清单（建议评审通过后按此验收）

1. **零点**：0 A 时 `AdcData.ADVal[CH_OPAM_CUR]` ≈ 2034（1.64 V），`ChgCurZero` 与之一致，`BatOpaCurGet()` ≈ 0 mA。
2. **线性**：施加 +10 A 充 / −10 A 放，AD 变化应约 ±85 LSB，`BatOpaCurGet()` 应约 ±10000 mA；正反向对称。
3. **满量程**：135 A 峰值时 AD 变化约 1154 LSB，VOUT 距 0.1 V / (VDDA−0.1 V) 仍有余量，不饱和。
4. **温漂**：−10 / 25 / 60 ℃ 下零点漂移落在收窄后的窗口内；记录 `ChgCurZero` 变化量。
5. **休眠/唤醒**：`PowerLowEnter/Exit` 后零点不跳变，验证重新 `HwCompOpamInit()` 有效。
6. **上电首帧**：冷启动后立即读 `BatOpaCurGet()`，不应出现瞬时大电流（验证 3.2 时序修正）。
7. **与 AFE 交叉比对**：稳态恒流下 OPA 读数与 `AfeGetCurrentFilt()` 偏差应在可接受范围（建议 ±5% 或 ±1 A）。

---

## 8. 声明

本文件为**评估用方案**，未修改 `Source/` 下任何 `.c/.h`、未改动工程配置、未提交 git。工作区原有的未提交改动（`git status` 中 12 个文件）均为本次操作之前已存在，未被触碰。
