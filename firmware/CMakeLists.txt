cmake_minimum_required(VERSION 3.10)

# x86平台
# set(CMAKE_C_COMPILER gcc)
# set(CMAKE_CXX_COMPILER g++)
# 交叉编译
set(CMAKE_C_COMPILER aarch64-linux-gnu-gcc)
set(CMAKE_CXX_COMPILER aarch64-linux-gnu-g++)

# 发布类型（Debug/Release）
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O0 -Wall -g")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -Wall -g")

# 读取 version 文件
file(READ "${CMAKE_SOURCE_DIR}/app/app_comm.h" VERSION_HEADER_CONTENT)

# 使用正则表达式提取版本号信息
string(REGEX MATCH "MAJOR_VERSION[ ]+([0-9]+)" MAJOR_VERSION ${VERSION_HEADER_CONTENT})
string(REGEX MATCH "MINOR_VERSION[ ]+([0-9]+)" MINOR_VERSION ${VERSION_HEADER_CONTENT})
string(REGEX MATCH "BUILD_VERSION[ ]+([0-9]+)" BUILD_VERSION ${VERSION_HEADER_CONTENT})

# 从匹配结果提取出数字
string(REGEX REPLACE "MAJOR_VERSION[ ]+([0-9]+)" "\\1" MAJOR_VERSION "${MAJOR_VERSION}")
string(REGEX REPLACE "MINOR_VERSION[ ]+([0-9]+)" "\\1" MINOR_VERSION "${MINOR_VERSION}")
string(REGEX REPLACE "BUILD_VERSION[ ]+([0-9]+)" "\\1" BUILD_VERSION "${BUILD_VERSION}")

# 生成版本号字符串
set(VERSION_STRING "${MAJOR_VERSION}.${MINOR_VERSION}.${BUILD_VERSION}")

# 设置ProjectName和DEMS_FIRMWARE
set(ProjectName "odin_bbms_c_v${VERSION_STRING}")
project(${ProjectName})

# 启用编译数据库（用于 VS Code）
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# 添加包含的头文件目录
include_directories(
    /usr/include/libxml2
    /usr/include/libxml2/libxml
    /usr/include/cjson
    /usr/local/include 

    # ${CMAKE_SOURCE_DIR}/clib
    ${PROJECT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/app
    ${PROJECT_SOURCE_DIR}/kernel
    ${PROJECT_SOURCE_DIR}/bsp
    ${PROJECT_SOURCE_DIR}/driver
    ${PROJECT_SOURCE_DIR}/lib/utarray
    ${PROJECT_SOURCE_DIR}/logic
    ${PROJECT_SOURCE_DIR}/protocol
)

# 连接库的位置
link_directories(/usr/aarch64-linux-gnu/lib)

# link_directories(/usr/local/lib)

# link_directories(${CMAKE_SOURCE_DIR}/clib)

# link_directories(${CMAKE_SOURCE_DIR}/clib/aarch64)
add_executable(${ProjectName}
    main.c
    app/app_cool_ctrl.c
    app/app_task_regedit.c
    app/app_data_storage.c
    app/app_timer.c
    app/app_bms_statistics.c
    app/app_bms_ctrl.c
    app/app_bms_fault.c

    bsp/bsp_can_sched.c
    bsp/bsp_modbusSlave.c
    bsp/bsp_modbus.c
    bsp/bsp_bms_com.c

    bsp/bsp_redis.c
    bsp/bsp_rtdb.c
    bsp/bsp_shm.c

    bsp/bsp_parse.c
    bsp/bsp_ntp.c
    protocol/modbus_default_server.c
    protocol/protocol_ems_modbus_map.c
    protocol/protocol_ems_modbus_server.c
    protocol/protocol_bms_hmi.c
    protocol/protocol_bms_rbms.c
    protocol/protocol_can_tms.c

    logic/logic_do.c
    logic/logic_di.c
    

    driver/drv_uart.c
    driver/drv_can.c
    driver/drv_gpio.c

    driver/forlinux_fcu2601/drv_gpio_forlinux_fcu2601.c
    driver/drv_tcp_client.c
    driver/drv_tcp_server.c

    kernel/kit_math.c
    kernel/kit_string.c
    kernel/kit_log.c
    kernel/kit_db.c
    kernel/kit_core.c
    kernel/kit_time.c
    kernel/kit_queue.c
)

# 连接库的位置
link_directories(/usr/local/lib ${CMAKE_BINARY_DIR}/lib)

# 设置SQL初始化使能
add_definitions(-DINIT_DB)

# 设置使用EMS设备类型(1:FCU2601)
add_definitions(-DHARDWARE_TYPE=1)

# 设置日志使能
# add_definitions(-DRELEASE_DEBUG)

# 添加编译选项
target_compile_options(${ProjectName} PRIVATE -fno-stack-protector)

# 连接库
target_link_libraries(${ProjectName} sqlite3 pthread m cjson)
