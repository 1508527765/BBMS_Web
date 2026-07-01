# BSP子目录CMakeLists.txt
# 用于编译libcrtdb.so共享库

# 强制设置crtdb库为Release构建类型
set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build type for crtdb library" FORCE)

# 设置库名称
set(LIB_NAME "crtdb")

# 3. 设置生成共享库
set(CMAKE_POSITION_INDEPENDENT_CODE ON) # 开启位置无关代码，用于动态库

# 创建共享库
add_library(${LIB_NAME} SHARED
    bsp_shm.c
    bsp_rtdb.c
    bsp_redis.c
    ${CMAKE_SOURCE_DIR}/kernel/kit_log.c
)

# 设置库的输出目录到build/bsp目录
set_target_properties(${LIB_NAME} PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bsp
    OUTPUT_NAME "crtdb"
)

# 添加编译选项
target_compile_options(${LIB_NAME} PRIVATE -fno-stack-protector)

# 为crtdb库设置Release构建选项，移除调试信息
target_compile_options(${LIB_NAME} PRIVATE -O2 -DNDEBUG -g0)
target_compile_definitions(${LIB_NAME} PRIVATE -DRELEASE_BUILD)

# 设置目标属性，确保不包含调试信息
set_target_properties(${LIB_NAME} PROPERTIES
    COMPILE_FLAGS "-g0"  # 强制不生成调试信息
    LINK_FLAGS "-s"      # 链接时strip调试信息
)

# 使用生成器表达式精确控制编译选项
target_compile_options(${LIB_NAME} PRIVATE 
    $<$<CONFIG:Debug>:-g0>      # Debug配置时也不生成调试信息
    $<$<CONFIG:Release>:-g0>    # Release配置时不生成调试信息
    $<$<CONFIG:RelWithDebInfo>:-g0>  # RelWithDebInfo配置时也不生成调试信息
)

# 链接时strip调试信息
set_target_properties(${LIB_NAME} PROPERTIES
    LINK_FLAGS "-s"
)

# # 链接必要的库
# target_link_libraries(${LIB_NAME} 
#     sqlite3 
#     m 
#     pthread 
#     cjson
# )

# 设置包含目录
target_include_directories(${LIB_NAME} PRIVATE
    # /usr/include/libxml2
    # /usr/include/libxml2/libxml
    # /usr/include/cjson
    # /usr/local/include
    # ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/app
    ${CMAKE_SOURCE_DIR}/kernel
    # ${CMAKE_SOURCE_DIR}/bsp
    # ${CMAKE_SOURCE_DIR}/driver
    # ${CMAKE_SOURCE_DIR}/lib
    ${CMAKE_SOURCE_DIR}/lib/utarray
    # ${CMAKE_SOURCE_DIR}/logic
)

# 添加编译定义
target_compile_definitions(${LIB_NAME} PRIVATE -DHARDWARE_TYPE=1)
