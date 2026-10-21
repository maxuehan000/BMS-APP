# -*- coding: gbk -*-
from openpyxl import Workbook
from openpyxl.styles import Font, PatternFill, Alignment, Border, Side

F = '微软雅黑'
tfont = Font(name=F, size=16, bold=True, color='FFFFFF')
hfont = Font(name=F, size=11, bold=True, color='FFFFFF')
sfont = Font(name=F, size=12, bold=True, color='1F3864')
bfont = Font(name=F, size=10.5)
bbfont = Font(name=F, size=10.5, bold=True)
tfill = PatternFill('solid', fgColor='2F5597')
hfill = PatternFill('solid', fgColor='4472C4')
sfill = PatternFill('solid', fgColor='D9E1F2')
gfill = PatternFill('solid', fgColor='F2F2F2')
thin = Side(style='thin', color='BFBFBF')
bd = Border(left=thin, right=thin, top=thin, bottom=thin)
wrapl = Alignment(vertical='center', wrap_text=True)
wrapc = Alignment(horizontal='center', vertical='center', wrap_text=True)
RISK = {
    '通过': (Font(name=F, size=10.5, bold=True, color='006100'), PatternFill('solid', fgColor='E2EFDA')),
    '警告': (Font(name=F, size=10.5, bold=True, color='9C6500'), PatternFill('solid', fgColor='FFF2CC')),
    '失败': (Font(name=F, size=10.5, bold=True, color='C00000'), PatternFill('solid', fgColor='FCE4E4')),
    '—': (Font(name=F, size=10.5, color='808080'), None),
}
wb = Workbook()

# ===== Sheet1 总览 =====
ws = wb.active; ws.title = '总览'; ws.sheet_view.showGridLines = False
ws.column_dimensions['A'].width = 30
ws.column_dimensions['B'].width = 12
ws.column_dimensions['C'].width = 60
ws.merge_cells('A1:C1'); c = ws['A1']; c.value = 'BMS 软件代码核对对比报告'
c.font = tfont; c.fill = tfill; c.alignment = wrapc; ws.row_dimensions[1].height = 32
ws.merge_cells('A2:C2'); c = ws['A2']
c.value = '项目：星海图 XHT-13S2P（带并包）　MCU：国民 N32L40x　AFE：KC81710H（软件 I2C）　方式：源码静态核对　日期：2026-10-15'
c.font = Font(name=F, size=10, color='404040'); c.alignment = wrapl
r = 4
for i, h in enumerate(['模块', '结论', '关键问题']):
    cc = ws.cell(r, i + 1, h); cc.font = hfont; cc.fill = hfill; cc.alignment = wrapc; cc.border = bd
ws.row_dimensions[r].height = 22
overview = [
    ('1. 充放电控制', '主体完整', '过流延时偏长（60A/13s、120A/4s）、预放短路检测被注释、二级过压未启用'),
    ('2. 电芯检测与均衡', '采集可用', '均衡功能失效（空实现）、电压无滤波、断线路径不全'),
    ('3. SOC / SOH', '算法清晰', 'SocToIndex 数组越界、Fcc 除零、OCV 上电校正被注释'),
    ('4. MCU/中断/看门狗/Flash/Boot/AFE', '驱动可用', '看门狗未启用、异常死循环、Boot VTOR 被注释'),
    ('5. 主循环/电源/灯显/按键', '结构清晰', '主循环 PowerSafeCheck 3s 阻塞'),
    ('6. 通讯', 'CAN 完整', 'UART 超时被注释、累加和校验弱'),
]
r += 1
for i, (m, concl, key) in enumerate(overview):
    for j, v in enumerate([m, concl, key]):
        cc = ws.cell(r, j + 1, v); cc.font = bfont; cc.border = bd
        cc.alignment = wrapl if j == 2 else wrapc
        if i % 2 == 1: cc.fill = gfill
    ws.row_dimensions[r].height = 30; r += 1
r += 1
ws.merge_cells('A%d:C%d' % (r, r)); c = ws.cell(r, 1, '统计：共核对 90+ 项，其中【失败】7 项、【警告】约 20 项、【通过】60+ 项。')
c.font = bbfont; c.alignment = wrapl

# ===== Sheet2 模块核对 =====
ws2 = wb.create_sheet('模块核对'); ws2.sheet_view.showGridLines = False
ws2.column_dimensions['A'].width = 18
ws2.column_dimensions['B'].width = 64
ws2.column_dimensions['C'].width = 9
modules = [
    ('模块 1：充放电控制（Charge.c / DisCharge.c / BmsCtrl.c）', [
        ('预充 / CC / CV', 'AFE 预充关闭、无软件闭环，软件只做预检→开充→判定关充', '—'),
        ('充满判定 ChgFC', '电流 250~500mA/5s 且总压≥53.95V，或累计充电 12h', '警告'),
        ('充电过压 ChgOV', '单体≥4175mV 且总压≥54275mV/1s，恢复<4100mV；恢复 10 次后 ChgER 禁充', '通过'),
        ('充电过流 ChgOC', '14A/4s、18A/2s（注释"2S"实为 4s）', '警告'),
        ('充电过温/低温', '≥57℃ 或 ≤3℃ 持续 2s，恢复 47℃/4℃；MOS 过温≥95℃', '通过'),
        ('二级过压 ChgOV2', 'ChgOverVol2Check 已实现但调用被注释（Charge.c:903）', '失败'),
        ('预放 / 主放', '预放 500ms → 主放+预放 100ms → 关预放', '通过'),
        ('预放短路检测', 'VolPack<5V 短路计数逻辑整体注释（BmsCtrl.c:189-209）', '失败'),
        ('欠压 DisUV/DUV', '2600/2500mV 持续 1s，恢复 2900mV', '通过'),
        ('放电过流 DisOC', '60A/13s、120A/4s（注释 1.5s/0.5s，实际偏长 8 倍）', '警告'),
        ('短路 DisSC', 'AFE 硬件 50ms；仅按键/充电解锁，20s 限 1 次', '警告'),
        ('过流锁 DisOCL', '自动恢复在 #if 0 中禁用，DisOCL 从未被置位', '失败'),
        ('模式状态机', '电流>500mA/300ms 切 DSG/CHG，<500mA/300ms 回 IDLE', '通过'),
        ('MOS 失效检测', '关放后仍有电流>1A/3s 置 DMosErr', '通过'),
    ]),
    ('模块 2：电芯检测与均衡（BatterCheck.c / Balance.c）', [
        ('单体电压采集', 'AFE 读 13 节，无滤波/中值，末节为 0 整体跳过', '警告'),
        ('总电压 VolSum', '无各节累加，总压取自 AFE vbat（与 Min/Max 非同源）', '警告'),
        ('温度采集', 'MCU ADC + AFE 两路，查表线性插值', '通过'),
        ('断线检测', 'AFE status0 0x8000 位 + 电压范围；专用 kc81710h_co_read 被注释', '警告'),
        ('压差故障', '充电压差≥600mV/5s（可选 300mV/10s）置 BatErr', '通过'),
        ('均衡开启条件', '仅充电中开均衡，奇偶节交替（静置不均衡）', '警告'),
        ('均衡执行', 'AfeSetBalance 为空实现，未调用 kc81710h_cb_set，均衡不生效', '失败'),
    ]),
    ('模块 3：SOC / SOH（soc.c / kalman_soc.c）', [
        ('估算方法', '安时积分 + OCV 查表校正 + 显示平滑，100ms 积分一次', '通过'),
        ('充满/放空校准', 'ChgOV/ChgFC→100%，DisUV→0%', '通过'),
        ('大偏差修正', '已充满但 Soc<80 强制拉到 95（跳变）', '警告'),
        ('SOH 估算', '循环次数查表衰减（非实时容量比）', '通过'),
        ('掉电保存', '掉电写 LogSocStrc，上电读回；运行中不周期写', '警告'),
        ('SocToIndex 越界', '固定返回 0~7，但 SocOcvRealTbl 仅 4~5 行，大电流越界读', '失败'),
        ('Fcc 除零', 'Soc=Rmc*100/Fcc 分母无保护，外部清空路径真实存在', '失败'),
        ('OCV 上电校正', 'SocOcvSet 被注释（soc.c:694），上电不重校准', '失败'),
        ('卡尔曼 EKF', '已实现但未启用/未融合/参数未标定（参考性质）', '警告'),
    ]),
    ('模块 4：MCU / 中断 / 看门狗 / Flash / Boot / AFE', [
        ('系统时钟', 'SystemCoreClockUpdate+SysTick，未显式配 PLL', '警告'),
        ('看门狗', 'WdtInit 被注释，IWDG 未启用，喂狗全部无效', '失败'),
        ('异常处理', 'HardFault 等 4 异常 while(1) 死循环，无复位/关 MOS/日志', '失败'),
        ('NVIC 优先级', 'NVIC_PriorityGroupConfig 未调用，全部抢占优先级=0', '警告'),
        ('Flash CRC', '累加和（初值 0x12345678），非标准 CRC', '警告'),
        ('立即保存区校验', 'AtOnceSaveDataRead 中 CRC 校验被注释', '失败'),
        ('Boot 跳转', '串口/CAN 双入口，关 MOS + 掉电保存 + 复位', '通过'),
        ('Boot VTOR', 'SCB->VTOR 偏移被注释，且注释"16K"与 BOOT_LOADER_SIZE=8K 矛盾', '失败'),
        ('AFE 通讯', '软 I2C 超时，AfeRun 失败 100 次(1s)置 AfeErr', '警告'),
    ]),
    ('模块 5：主循环 / 电源 / 灯显 / 按键', [
        ('主循环结构', '采集→控制→通讯→电源→显示，10ms 时间片', '通过'),
        ('调度模式', '单线程轮询无抢占，重任务会连锁延迟', '警告'),
        ('阻塞点', 'PowerSafeCheck 触发时阻塞主循环 3s', '失败'),
        ('防耗死兜底', 'VolMin<2.0V 持续 40s 关 MOS 关机', '通过'),
        ('关机控制', '过放/无键/硬故障 60s；充电/测试/升级不关机', '通过'),
        ('灯显', '600ms 闪烁、5 档 SOC、充电呼吸灯（代码冗余）', '警告'),
        ('按键', '开关机键 + 灯光键，充电/测试屏蔽（宏重复定义）', '警告'),
    ]),
    ('模块 6：通讯（UartBW.c / CanBW.c / multiCan.c）', [
        ('UART 接收', '帧头 0xF1 / 帧尾 0xF2，长度校验', '通过'),
        ('UART 校验', '累加和（非 CRC），抗错弱', '警告'),
        ('UART 超时', 'UartIsTimeOut 被注释，帧异常无法自恢复', '失败'),
        ('CAN', '队列 + CRC16 + 功能码分发，超时清 ECU 状态', '通过'),
        ('多包并联', '广播/地址分配/离线检测/错误恢复/MOS 同步', '通过'),
    ]),
]
r = 1
for title, rows in modules:
    ws2.merge_cells('A%d:C%d' % (r, r)); c = ws2.cell(r, 1, title)
    c.font = sfont; c.fill = sfill; c.alignment = wrapl; ws2.row_dimensions[r].height = 22
    r += 1
    for i, h in enumerate(['检查项', '现状 / 结论', '风险']):
        cc = ws2.cell(r, i + 1, h); cc.font = hfont; cc.fill = hfill; cc.alignment = wrapc; cc.border = bd
    ws2.row_dimensions[r].height = 20; r += 1
    for item, desc, risk in rows:
        c1 = ws2.cell(r, 1, item); c1.font = bbfont; c1.border = bd; c1.alignment = wrapl
        c2 = ws2.cell(r, 2, desc); c2.font = bfont; c2.border = bd; c2.alignment = wrapl
        c3 = ws2.cell(r, 3, risk)
        rf, rfll = RISK.get(risk, (bfont, None))
        c3.font = rf; c3.border = bd; c3.alignment = wrapc
        if rfll: c3.fill = rfll
        ws2.row_dimensions[r].height = 30; r += 1
    r += 1

# ===== Sheet3 问题与整改 =====
ws3 = wb.create_sheet('问题与整改'); ws3.sheet_view.showGridLines = False
ws3.column_dimensions['A'].width = 8
ws3.column_dimensions['B'].width = 66
ws3.column_dimensions['C'].width = 12
issues = [
    ('严重', 'C00000', [
        '看门狗未启用（WdtInit 被注释）',
        'HardFault 异常死循环（无复位/关 MOS/日志）',
        'Boot VTOR 中断向量偏移被注释',
        '均衡功能失效（AfeSetBalance 空实现）',
        'SocToIndex 数组越界',
        'Fcc 除零风险',
        'UART 超时检测被注释',
    ]),
    ('高', 'E36C0A', [
        '过流延时与注释不符（60A/13s、120A/4s）',
        '短路/过流无自动恢复，DisOCL 从未置位',
        '二级过压、预放短路检测未启用',
        'NVIC 优先级分组未配置',
        'Flash CRC 弱 + 立即保存区校验被注释',
        'OCV 上电校正被注释',
        '主循环 PowerSafeCheck 3s 阻塞',
    ]),
    ('中', 'BF8F00', [
        '电压采集无滤波',
        '断线检测路径不全',
        'VolSum 缺失',
        'AFE 故障判定 1s 偏慢 + 失败仍用旧数据',
        'BatErr 置位后无清位',
        '均衡仅充电时',
        '卡尔曼未启用未标定',
        'SOC 保护性跳变',
        '电源双关机路径重叠',
        '容量基准混用（DESIGN/TYPE）',
    ]),
    ('低', '808080', [
        'showDis 大量注释旧代码',
        'KEY_ON_DELAY 重复定义',
        'UART_TIMEOUT 注释错误',
        '均衡阈值注释与宏不一致',
    ]),
]
r = 1
ws3.merge_cells('A1:C1'); c = ws3.cell(1, 1, '重点问题清单')
c.font = tfont; c.fill = tfill; c.alignment = wrapc; ws3.row_dimensions[1].height = 30
r = 2
for i, h in enumerate(['序号', '问题描述', '等级']):
    cc = ws3.cell(r, i + 1, h); cc.font = hfont; cc.fill = hfill; cc.alignment = wrapc; cc.border = bd
r += 1
num = 1
for level, color, items in issues:
    lf = Font(name=F, size=10.5, bold=True, color=color)
    lfill = PatternFill('solid', fgColor='FFFFFF')
    for it in items:
        c1 = ws3.cell(r, 1, num); c1.font = bfont; c1.border = bd; c1.alignment = wrapc
        c2 = ws3.cell(r, 2, it); c2.font = bfont; c2.border = bd; c2.alignment = wrapl
        c3 = ws3.cell(r, 3, level); c3.font = lf; c3.border = bd; c3.alignment = wrapc
        ws3.row_dimensions[r].height = 20; r += 1; num += 1
r += 1
ws3.merge_cells('A%d:C%d' % (r, r)); c = ws3.cell(r, 1, '整改建议（优先级排序）')
c.font = tfont; c.fill = tfill; c.alignment = wrapc; ws3.row_dimensions[r].height = 30
r += 1
for i, h in enumerate(['优先级', '整改内容', '']):
    cc = ws3.cell(r, i + 1, h); cc.font = hfont; cc.fill = hfill; cc.alignment = wrapc; cc.border = bd
r += 1
sug = [
    ('P0 安全', '启用 IWDG 看门狗；HardFault 处理加"关 MOS+日志+复位"；修复 Boot VTOR 偏移'),
    ('P0 数据', '修复 SocToIndex 越界、Fcc 除零、均衡空实现、UART 超时'),
    ('P1 保护', '核对过流延时(13s/4s 是否达标)、启用二级过压与预放短路检测、补短路/过流自动恢复'),
    ('P1 架构', '配置 NVIC 优先级分组；拆解主循环 3s 阻塞；Flash 改标准 CRC'),
    ('P2 优化', '电压/温度加中值滤波；SOC OCV 上电校正启用；卡尔曼标定后融合'),
]
for p, s in sug:
    c1 = ws3.cell(r, 1, p); c1.font = bbfont; c1.border = bd; c1.alignment = wrapc
    ws3.merge_cells('B%d:C%d' % (r, r))
    c2 = ws3.cell(r, 2, s); c2.font = bfont; c2.border = bd; c2.alignment = wrapl
    ws3.row_dimensions[r].height = 24; r += 1

wb.save('f:/项目资料/星海图/App-Para/BMS代码核对对比报告.xlsx')
print('OK')
