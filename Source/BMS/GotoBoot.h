#ifndef __GOTOBOOT_H__
#define __GOTOBOOT_H__

#define FLASH_BOOT_BWK      			0x11335577UL
#define FLASH_BOOT_KEY					0x2A57F814UL
#define FLASH_REBOOT_OTA				0xA55A081FUL

#define APP_UPGRADE_MAX_TIME			(10000U)		// 10S, unit:1ms

extern void BootLoaderActive(void);
extern void BootGoto(void);
extern void BootJudge(void);
extern void BootReceive(uint8_t rxData);
extern void Bootload_WriteUpgradeMark(void);

#endif /* __GOTOBOOT_H__ */
