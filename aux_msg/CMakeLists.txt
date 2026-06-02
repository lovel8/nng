#
# aux_msg - DDD 装饰器模式消息通信库二次封装
#
# 此 CMakeLists.txt 构建 aux_msg 静态/共享库。
# 底层通信库可替换（当前基于 NNG）。
#

cmake_minimum_required(VERSION 3.15)
project(aux_msg C)

option(BUILD_SHARED_LIBS "Build shared library" ON)
option(AUX_BUILD_EXAMPLE "Build example programs" ON)
option(AUX_USE_FIND_PACKAGE "Use find_package for nng" OFF)

# ================================================================
# 查找 NNG
# ================================================================

if(AUX_USE_FIND_PACKAGE)
    find_package(nng REQUIRED)
    set(NNG_INCLUDE_DIRS "")
    set(NNG_LIBRARIES nng::nng)
    # find_package 模式：尝试获取 nng 库的实际路径
    get_target_property(NNG_LIB nng::nng IMPORTED_LOCATION)
    if(NOT NNG_LIB)
        get_target_property(NNG_LIB nng::nng IMPORTED_LOCATION_RELEASE)
    endif()
    if(NOT NNG_LIB)
        get_target_property(NNG_LIB nng::nng IMPORTED_LOCATION_DEBUG)
    endif()
else()
    # 默认假设 NNG 在父项目中已构建或已安装
    if(NOT NNG_INCLUDE_DIRS)
        # 尝试查找 nng 头文件
        find_path(NNG_INCLUDE_DIR nng/nng.h
            HINTS ${CMAKE_SOURCE_DIR}/../include
                  ${CMAKE_INSTALL_PREFIX}/include
                  /usr/local/include
                  /usr/include
        )
        if(NOT NNG_INCLUDE_DIR)
            # 回退：假设在父目录
            set(NNG_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/../include")
        endif()
        set(NNG_INCLUDE_DIRS ${NNG_INCLUDE_DIR})
    endif()

    if(NOT NNG_LIBRARIES)
        find_library(NNG_LIB nng
            HINTS ${CMAKE_SOURCE_DIR}/../build
                  ${CMAKE_INSTALL_PREFIX}/lib
                  /usr/local/lib
                  /usr/lib
        )
        if(NNG_LIB)
            set(NNG_LIBRARIES ${NNG_LIB})
        else()
            # 如果没有预编译的 nng，尝试作为子目录构建
            if(EXISTS "${CMAKE_SOURCE_DIR}/../CMakeLists.txt")
                message(STATUS "Building NNG as subdirectory...")
                add_subdirectory(${CMAKE_SOURCE_DIR}/.. ${CMAKE_BINARY_DIR}/nng)
                set(NNG_LIBRARIES nng)
            endif()
        endif()
    else()
        # 外部已传入 NNG_LIBRARIES，记录路径用于 force_load
        set(NNG_LIB ${NNG_LIBRARIES})
    endif()
endif()

message(STATUS "NNG include dirs: ${NNG_INCLUDE_DIRS}")
message(STATUS "NNG libraries: ${NNG_LIBRARIES}")

# ================================================================
# aux_msg 库源文件
# ================================================================

set(AUX_SOURCES
    adapter/nng/aux_channel.c
    adapter/nng/aux_connector.c
    adapter/nng/aux_connection.c
    adapter/nng/aux_envelope.c
    adapter/nng/aux_future.c
    adapter/nng/aux_runtime.c
    adapter/nng/aux_session.c
    adapter/nng/aux_subscriber.c
)

set(AUX_HEADERS
    aux_msg.h
    interface/aux_types.h
    interface/aux_runtime.h
    interface/aux_channel.h
    interface/aux_connector.h
    interface/aux_connection.h
    interface/aux_envelope.h
    interface/aux_future.h
    interface/aux_session.h
    interface/aux_subscriber.h
    adapter/nng/aux_types.h
)

# ================================================================
# 构建库
# ================================================================

add_library(aux_msg ${AUX_SOURCES})

target_include_directories(aux_msg
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/interface>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/adapter/nng>
        $<INSTALL_INTERFACE:include/aux_msg>
        ${NNG_INCLUDE_DIRS}
)

# 将 NNG 静态链接进 aux_msg，使用者只需 -laux_msg 即可
# 注意：当 NNG 以 minimal 裁剪时（如 HTTP=OFF），libnng.a 中部分 .o 文件
# 可能残留对未编译符号的引用。这里仅通过 force_load 方式嵌入（见下方），
# 不在 target_link_libraries 中直接链接，避免重复链接和符号冲突。
if(NOT (BUILD_SHARED_LIBS AND APPLE))
    target_link_libraries(aux_msg PRIVATE ${NNG_LIBRARIES})
endif()

# NNG 静态库可能依赖 mbedTLS（TLS 支持），自动查找并链接
if(NOT AUX_USE_FIND_PACKAGE)
    find_library(MBEDTLS_LIB mbedtls
        HINTS /usr/local/lib /opt/homebrew/lib /usr/lib
    )
    find_library(MBEDX509_LIB mbedx509
        HINTS /usr/local/lib /opt/homebrew/lib /usr/lib
    )
    find_library(MBEDCRYPTO_LIB mbedcrypto
        HINTS /usr/local/lib /opt/homebrew/lib /usr/lib
    )
    if(MBEDTLS_LIB AND MBEDX509_LIB AND MBEDCRYPTO_LIB)
        message(STATUS "Found mbedTLS: ${MBEDTLS_LIB}")
        target_link_libraries(aux_msg PRIVATE ${MBEDTLS_LIB} ${MBEDX509_LIB} ${MBEDCRYPTO_LIB})
    else()
        message(STATUS "mbedTLS not found - TLS features will be unavailable")
    endif()
endif()

# C11 标准
target_compile_features(aux_msg PUBLIC c_std_11)

# 设置库属性
# 符号可见性通过链接器导出列表控制（见下方 BUILD_SHARED_LIBS 部分）
# macOS: -exported_symbols_list 只导出 aux_*
# Linux: --version-script 只导出 aux_*
set_target_properties(aux_msg PROPERTIES
    VERSION 1.0.0
    SOVERSION 1
    PUBLIC_HEADER "${AUX_HEADERS}"
)

# 构建共享库时，将 NNG 静态库完整嵌入（避免使用者需要 -lnng）
# macOS: 用 -exported_symbols_list 只导出 aux_*，nng_* 通过 force_load 嵌入但隐藏
# Linux: whole-archive + version-script
if(BUILD_SHARED_LIBS AND NNG_LIB)
    if(APPLE)
        # 生成导出符号列表：只导出 aux_* 符号
        set(AUX_EXPORTS_FILE "${CMAKE_CURRENT_BINARY_DIR}/aux_exports.txt")
        file(WRITE ${AUX_EXPORTS_FILE} "_aux_*\n")
        target_link_options(aux_msg PRIVATE
            -Wl,-force_load ${NNG_LIB}
            -Wl,-exported_symbols_list,${AUX_EXPORTS_FILE}
            -Wl,-undefined,dynamic_lookup
        )
    else()
        # Linux: whole-archive 嵌入 + version-script 隐藏 nng 符号，仅暴露 aux_*
        target_link_options(aux_msg PRIVATE
            -Wl,--whole-archive ${NNG_LIB} -Wl,--no-whole-archive
            -Wl,--version-script=${CMAKE_CURRENT_SOURCE_DIR}/aux_exports.ver
        )
    endif()
endif()

# ================================================================
# 安装规则
# ================================================================

include(GNUInstallDirs)

install(TARGETS aux_msg
    EXPORT aux_msg-targets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/aux_msg
)

install(EXPORT aux_msg-targets
    FILE aux_msg-targets.cmake
    NAMESPACE aux_msg::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/aux_msg
)

# ================================================================
# 示例程序
# ================================================================

if(AUX_BUILD_EXAMPLE)
    add_subdirectory(examples)
endif()
