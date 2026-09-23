#这部分一般不改动

# usb头文件/
include_directories(BMS)
include_directories(BMS/AFE)
include_directories(BMS/MCU)
# include_directories(BMS/RTC)

aux_source_directory("BMS" PROJECT_SOURCES)
aux_source_directory("BMS/AFE" PROJECT_SOURCES)
aux_source_directory("BMS/MCU" PROJECT_SOURCES)
# aux_source_directory("BMS/RTC" PROJECT_SOURCES)

list(REMOVE_ITEM PROJECT_SOURCES "BMS/logsave.c" "BMS/CanBW.c" "BMS/UartCT.c"
        "BMS/SW_CAN_Protocol.c" "BMS/SW_Protocol_CRC.c" "BMS/UpgradeCan.c"
        )
# 安全库
# include_directories(N32_SelfTest_Library/inc)
# aux_source_directory("N32_SelfTest_Library/src" SelfTest_SRC)
# 库文件源码
# 可执行文件
add_executable(${PROJECT_NAME}.elf  
        ${STD_SOURCES}
        ${startup}
        ${ldscript}
        ${PROJECT_SOURCES}
        # ${SelfTest_SRC}
        )
# target_link_libraries(${PROJECT_NAME}.elf ${CMAKE_SOURCE_DIR}/Projects/ES32F0283//Customer/bms_api_CM0.lib)