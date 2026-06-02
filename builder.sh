#!/bin/bash
#
# NNG + AUX 一键编译脚本
# 支持多命令链式调用，预设/裁剪与构建类型正交组合
#
# 用法示例:
#   ./build.sh                              # 完整功能 Debug
#   ./build.sh release                      # 完整功能 Release
#   ./build.sh minimal release              # 最小化 + Release
#   ./build.sh minimal release aux_msg      # 最小化 + Release NNG + AUX
#   ./build.sh clean                        # 清理
#   ./build.sh config                       # 交互式配置
#

set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
BUILD_DIR="${SCRIPT_DIR}/build"
INSTALL_DIR="${SCRIPT_DIR}/install"
AUX_BUILD_DIR="${SCRIPT_DIR}/aux_msg/build"

# ============================================================
# 默认配置（可通过环境变量覆盖）
# ============================================================

# 收集所有参数
ALL_ARGS=("$@")
if [ ${#ALL_ARGS[@]} -eq 0 ]; then
    ALL_ARGS=("debug")
fi

# 当前构建类型（debug/release），aux_msg/install 自动跟随
CURRENT_BUILD_TYPE="debug"

# ---- 工具函数 ----

# 获取并行编译的 job 数
get_jobs() {
    nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4
}

# build_type 映射到 CMake 构建类型
cmake_build_type() {
    [ "$1" = "release" ] && echo "Release" || echo "Debug"
}

# 打印当前裁剪/选项配置摘要
print_config_summary() {
    echo "    TLS=$NNG_ENABLE_TLS($NNG_TLS_ENGINE) HTTP=$NNG_ENABLE_HTTP IPv6=$NNG_ENABLE_IPV6 Stats=$NNG_ENABLE_STATS Tests=$NNG_TESTS Tools=$NNG_TOOLS"
    echo "    协议: BUS0=$NNG_PROTO_BUS0 PAIR0=$NNG_PROTO_PAIR0 PAIR1=$NNG_PROTO_PAIR1 PUB0=$NNG_PROTO_PUB0 SUB0=$NNG_PROTO_SUB0 PUSH0=$NNG_PROTO_PUSH0 PULL0=$NNG_PROTO_PULL0 REQ0=$NNG_PROTO_REQ0 REP0=$NNG_PROTO_REP0 SURVEYOR0=$NNG_PROTO_SURVEYOR0 RESPONDENT0=$NNG_PROTO_RESPONDENT0"
    echo "    传输: TCP=$NNG_TRANSPORT_TCP TLS=$NNG_TRANSPORT_TLS WS=$NNG_TRANSPORT_WS WSS=$NNG_TRANSPORT_WSS UDP=$NNG_TRANSPORT_UDP DTLS=$NNG_TRANSPORT_DTLS IPC=$NNG_TRANSPORT_IPC INPROC=$NNG_TRANSPORT_INPROC FDC=$NNG_TRANSPORT_FDC"
}

# ---- 基础选项 ----
NNG_ENABLE_TLS="${NNG_ENABLE_TLS:-ON}"
NNG_TLS_ENGINE="${NNG_TLS_ENGINE:-mbed}"
NNG_ENABLE_HTTP="${NNG_ENABLE_HTTP:-ON}"
NNG_ENABLE_IPV6="${NNG_ENABLE_IPV6:-ON}"
NNG_ENABLE_STATS="${NNG_ENABLE_STATS:-ON}"
NNG_TESTS="${NNG_TESTS:-ON}"
NNG_TOOLS="${NNG_TOOLS:-ON}"
NNG_ELIDE_DEPRECATED="${NNG_ELIDE_DEPRECATED:-OFF}"

# ---- 协议选项 ----
NNG_PROTO_BUS0="${NNG_PROTO_BUS0:-ON}"
NNG_PROTO_PAIR0="${NNG_PROTO_PAIR0:-ON}"
NNG_PROTO_PAIR1="${NNG_PROTO_PAIR1:-ON}"
NNG_PROTO_PUB0="${NNG_PROTO_PUB0:-ON}"
NNG_PROTO_SUB0="${NNG_PROTO_SUB0:-ON}"
NNG_PROTO_PUSH0="${NNG_PROTO_PUSH0:-ON}"
NNG_PROTO_PULL0="${NNG_PROTO_PULL0:-ON}"
NNG_PROTO_REQ0="${NNG_PROTO_REQ0:-ON}"
NNG_PROTO_REP0="${NNG_PROTO_REP0:-ON}"
NNG_PROTO_SURVEYOR0="${NNG_PROTO_SURVEYOR0:-ON}"
NNG_PROTO_RESPONDENT0="${NNG_PROTO_RESPONDENT0:-ON}"

# ---- 传输选项 ----
NNG_TRANSPORT_TCP="${NNG_TRANSPORT_TCP:-ON}"
NNG_TRANSPORT_TLS="${NNG_TRANSPORT_TLS:-ON}"
NNG_TRANSPORT_WS="${NNG_TRANSPORT_WS:-ON}"
NNG_TRANSPORT_WSS="${NNG_TRANSPORT_WSS:-ON}"
NNG_TRANSPORT_UDP="${NNG_TRANSPORT_UDP:-ON}"
NNG_TRANSPORT_DTLS="${NNG_TRANSPORT_DTLS:-ON}"
NNG_TRANSPORT_IPC="${NNG_TRANSPORT_IPC:-ON}"
NNG_TRANSPORT_INPROC="${NNG_TRANSPORT_INPROC:-ON}"
NNG_TRANSPORT_FDC="${NNG_TRANSPORT_FDC:-ON}"

# ============================================================

show_help() {
    cat << EOF
NNG 快速编译脚本

用法: ./build.sh [预设] [命令...]

预设（可与其他命令组合）:
  minimal     最小化配置（仅 TCP+IPC，pair+reqrep 协议）

命令:
  debug       Debug 编译 NNG，完整功能（默认）
  release     Release 优化编译 NNG，完整功能
  clean       清理构建目录
  install     编译并安装 NNG 到 ./install 目录
  config      交互式选择编译选项
  aux_msg     编译 AUX 封装库（自动跟随 debug/release，自动先编译底层库）
  help        显示此帮助信息

环境变量（可直接设置，如 NNG_ENABLE_TLS=OFF ./build.sh）:

  ---- 基础选项 ----
  NNG_ENABLE_TLS         是否启用TLS [ON|OFF]          默认 ON
  NNG_TLS_ENGINE         TLS引擎 [mbed|wolf]           默认 mbed
  NNG_ENABLE_HTTP        是否启用HTTP API [ON|OFF]      默认 ON
  NNG_ENABLE_IPV6        是否启用IPv6 [ON|OFF]          默认 ON
  NNG_ENABLE_STATS       是否启用统计 [ON|OFF]          默认 ON
  NNG_TESTS              是否构建测试 [ON|OFF]          默认 ON
  NNG_TOOLS              是否构建工具 [ON|OFF]          默认 ON
  NNG_ELIDE_DEPRECATED   裁剪废弃API [ON|OFF]           默认 OFF

  ---- 协议选项 (ON/OFF) ----
  NNG_PROTO_BUS0          总线协议                      默认 ON
  NNG_PROTO_PAIR0         一对一 v0                     默认 ON
  NNG_PROTO_PAIR1         一对一 v1                     默认 ON
  NNG_PROTO_PUB0          发布端                        默认 ON
  NNG_PROTO_SUB0          订阅端                        默认 ON
  NNG_PROTO_PUSH0         推端                          默认 ON
  NNG_PROTO_PULL0         拉端                          默认 ON
  NNG_PROTO_REQ0          请求端                        默认 ON
  NNG_PROTO_REP0          回复端                        默认 ON
  NNG_PROTO_SURVEYOR0     调查端                        默认 ON
  NNG_PROTO_RESPONDENT0   响应端                        默认 ON

  ---- 传输选项 (ON/OFF) ----
  NNG_TRANSPORT_TCP       TCP 传输                      默认 ON
  NNG_TRANSPORT_TLS       TLS 加密传输                  默认 ON
  NNG_TRANSPORT_WS        WebSocket 传输                默认 ON
  NNG_TRANSPORT_WSS       加密 WebSocket 传输           默认 ON
  NNG_TRANSPORT_UDP       UDP 传输（实验性）            默认 ON
  NNG_TRANSPORT_DTLS      DTLS 传输（实验性）           默认 ON
  NNG_TRANSPORT_IPC       IPC 进程间通信                默认 ON
  NNG_TRANSPORT_INPROC    进程内通信                    默认 ON
  NNG_TRANSPORT_FDC       文件描述符传输（实验性）      默认 ON

示例:
  ./build.sh                                    # 完整功能 Debug 编译
  ./build.sh release                            # 完整功能 Release 编译
  ./build.sh minimal release                    # 最小化 + Release 编译
  ./build.sh minimal release aux_msg            # 最小化 + Release NNG + AUX
  ./build.sh minimal debug aux_msg              # 最小化 + Debug NNG + AUX
  ./build.sh config                             # 交互式选择选项
  NNG_ENABLE_TLS=OFF ./build.sh                 # 不启用 TLS
  ./build.sh clean && ./build.sh                # 清理后重新编译
EOF
}

clean() {
    echo "==> 清理构建目录..."
    rm -rf "${BUILD_DIR}" "${INSTALL_DIR}" "${AUX_BUILD_DIR}"
    echo "==> 清理完成"
}

apply_minimal() {
    echo "==> 应用最小化配置..."
    NNG_ENABLE_TLS="OFF"
    NNG_ENABLE_HTTP="OFF"
    NNG_ENABLE_IPV6="OFF"
    NNG_ENABLE_STATS="OFF"
    NNG_ELIDE_DEPRECATED="ON"
    NNG_TESTS="OFF"
    NNG_TOOLS="OFF"

    NNG_PROTO_BUS0="OFF"
    NNG_PROTO_PUB0="OFF"
    NNG_PROTO_SUB0="OFF"
    NNG_PROTO_PUSH0="OFF"
    NNG_PROTO_PULL0="OFF"
    NNG_PROTO_SURVEYOR0="OFF"
    NNG_PROTO_RESPONDENT0="OFF"
    # 保留: PAIR0, PAIR1, REQ0, REP0

    NNG_TRANSPORT_TLS="OFF"
    NNG_TRANSPORT_WS="OFF"
    NNG_TRANSPORT_WSS="OFF"
    NNG_TRANSPORT_UDP="OFF"
    NNG_TRANSPORT_DTLS="OFF"
    NNG_TRANSPORT_FDC="OFF"
    # 保留: TCP, IPC, INPROC
}

interactive_config() {
    echo "============================================"
    echo "  NNG 交互式编译配置"
    echo "============================================"
    echo ""

    # 预设选择
    echo "选择预设配置:"
    echo "  1) 完整功能（默认）"
    echo "  2) 最小化（TCP+IPC，pair+reqrep）"
    echo "  3) 自定义"
    read -r -p "请选择 [1-3]: " preset
    echo ""

    case "$preset" in
        2) apply_minimal; return ;;
        3) ;;
        *) return ;;
    esac

    # 基础选项
    echo "--- 基础选项 ---"
    read -r -p "启用 TLS? [Y/n]: " ans; NNG_ENABLE_TLS=$( [[ ! "$ans" =~ ^[Nn] ]] && echo "ON" || echo "OFF" )
    read -r -p "启用 HTTP API? [Y/n]: " ans; NNG_ENABLE_HTTP=$( [[ ! "$ans" =~ ^[Nn] ]] && echo "ON" || echo "OFF" )
    read -r -p "启用 IPv6? [Y/n]: " ans; NNG_ENABLE_IPV6=$( [[ ! "$ans" =~ ^[Nn] ]] && echo "ON" || echo "OFF" )
    read -r -p "启用统计? [Y/n]: " ans; NNG_ENABLE_STATS=$( [[ ! "$ans" =~ ^[Nn] ]] && echo "ON" || echo "OFF" )
    read -r -p "构建测试? [Y/n]: " ans; NNG_TESTS=$( [[ ! "$ans" =~ ^[Nn] ]] && echo "ON" || echo "OFF" )
    read -r -p "构建工具? [Y/n]: " ans; NNG_TOOLS=$( [[ ! "$ans" =~ ^[Nn] ]] && echo "ON" || echo "OFF" )
    read -r -p "裁剪废弃API? [y/N]: " ans; NNG_ELIDE_DEPRECATED=$( [[ "$ans" =~ ^[Yy] ]] && echo "ON" || echo "OFF" )
    echo ""

    # 协议选项
    echo "--- 协议选项 ---"
    read -r -p "BUS0 (总线)? [Y/n]: " ans; NNG_PROTO_BUS0=$( [[ ! "$ans" =~ ^[Nn] ]] && echo "ON" || echo "OFF" )
    read -r -p "PAIR0 (一对一v0)? [Y/n]: " ans; NNG_PROTO_PAIR0=$( [[ ! "$ans" =~ ^[Nn] ]] && echo "ON" || echo "OFF" )
    read -r -p "PAIR1 (一对一v1)? [Y/n]: " ans; NNG_PROTO_PAIR1=$( [[ ! "$ans" =~ ^[Nn] ]] && echo "ON" || echo "OFF" )
    read -r -p "PUB0 (发布)? [Y/n]: " ans; NNG_PROTO_PUB0=$( [[ ! "$ans" =~ ^[Nn] ]] && echo "ON" || echo "OFF" )
    read -r -p "SUB0 (订阅)? [Y/n]: " ans; NNG_PROTO_SUB0=$( [[ ! "$ans" =~ ^[Nn] ]] && echo "ON" || echo "OFF" )
    read -r -p "PUSH0 (推)? [Y/n]: " ans; NNG_PROTO_PUSH0=$( [[ ! "$ans" =~ ^[Nn] ]] && echo "ON" || echo "OFF" )
    read -r -p "PULL0 (拉)? [Y/n]: " ans; NNG_PROTO_PULL0=$( [[ ! "$ans" =~ ^[Nn] ]] && echo "ON" || echo "OFF" )
    read -r -p "REQ0 (请求)? [Y/n]: " ans; NNG_PROTO_REQ0=$( [[ ! "$ans" =~ ^[Nn] ]] && echo "ON" || echo "OFF" )
    read -r -p "REP0 (回复)? [Y/n]: " ans; NNG_PROTO_REP0=$( [[ ! "$ans" =~ ^[Nn] ]] && echo "ON" || echo "OFF" )
    read -r -p "SURVEYOR0 (调查)? [Y/n]: " ans; NNG_PROTO_SURVEYOR0=$( [[ ! "$ans" =~ ^[Nn] ]] && echo "ON" || echo "OFF" )
    read -r -p "RESPONDENT0 (响应)? [Y/n]: " ans; NNG_PROTO_RESPONDENT0=$( [[ ! "$ans" =~ ^[Nn] ]] && echo "ON" || echo "OFF" )
    echo ""

    # 传输选项
    echo "--- 传输选项 ---"
    read -r -p "TCP? [Y/n]: " ans; NNG_TRANSPORT_TCP=$( [[ ! "$ans" =~ ^[Nn] ]] && echo "ON" || echo "OFF" )
    read -r -p "TLS? [Y/n]: " ans; NNG_TRANSPORT_TLS=$( [[ ! "$ans" =~ ^[Nn] ]] && echo "ON" || echo "OFF" )
    read -r -p "WebSocket? [Y/n]: " ans; NNG_TRANSPORT_WS=$( [[ ! "$ans" =~ ^[Nn] ]] && echo "ON" || echo "OFF" )
    read -r -p "WSS (加密WebSocket)? [Y/n]: " ans; NNG_TRANSPORT_WSS=$( [[ ! "$ans" =~ ^[Nn] ]] && echo "ON" || echo "OFF" )
    read -r -p "UDP? [Y/n]: " ans; NNG_TRANSPORT_UDP=$( [[ ! "$ans" =~ ^[Nn] ]] && echo "ON" || echo "OFF" )
    read -r -p "DTLS? [Y/n]: " ans; NNG_TRANSPORT_DTLS=$( [[ ! "$ans" =~ ^[Nn] ]] && echo "ON" || echo "OFF" )
    read -r -p "IPC (进程间)? [Y/n]: " ans; NNG_TRANSPORT_IPC=$( [[ ! "$ans" =~ ^[Nn] ]] && echo "ON" || echo "OFF" )
    read -r -p "INPROC (进程内)? [Y/n]: " ans; NNG_TRANSPORT_INPROC=$( [[ ! "$ans" =~ ^[Nn] ]] && echo "ON" || echo "OFF" )
    read -r -p "FDC (文件描述符)? [Y/n]: " ans; NNG_TRANSPORT_FDC=$( [[ ! "$ans" =~ ^[Nn] ]] && echo "ON" || echo "OFF" )
    echo ""

    echo "==> 配置完成，确认选项:"
    print_config_summary
    echo ""
    read -r -p "按回车继续编译，Ctrl+C 取消..."
}

configure() {
    local build_type="$1"
    local cbt
    cbt=$(cmake_build_type "$build_type")

    echo "==> 配置 ${cbt} 编译..."
    echo "==> 当前配置摘要:"
    print_config_summary

    mkdir -p "${BUILD_DIR}"

    local generator="Unix Makefiles"
    if command -v ninja &> /dev/null; then
        generator="Ninja"
        echo "==> 检测到 Ninja，使用 Ninja 构建"
    else
        echo "==> 未检测到 Ninja，使用 Make 构建（建议安装 Ninja 以获得更快的构建速度）"
    fi

    cmake -S "${SCRIPT_DIR}" \
          -B "${BUILD_DIR}" \
          -G "${generator}" \
          -DCMAKE_BUILD_TYPE="${cbt}" \
          -DCMAKE_INSTALL_PREFIX="${INSTALL_DIR}" \
          -DCMAKE_POSITION_INDEPENDENT_CODE=ON \
          -DBUILD_SHARED_LIBS=OFF \
          -DNNG_ENABLE_TLS="${NNG_ENABLE_TLS}" \
          -DNNG_TLS_ENGINE="${NNG_TLS_ENGINE}" \
          -DNNG_ENABLE_HTTP="${NNG_ENABLE_HTTP}" \
          -DNNG_ENABLE_IPV6="${NNG_ENABLE_IPV6}" \
          -DNNG_ENABLE_STATS="${NNG_ENABLE_STATS}" \
          -DNNG_TESTS="${NNG_TESTS}" \
          -DNNG_TOOLS="${NNG_TOOLS}" \
          -DNNG_ELIDE_DEPRECATED="${NNG_ELIDE_DEPRECATED}" \
          -DNNG_PROTO_BUS0="${NNG_PROTO_BUS0}" \
          -DNNG_PROTO_PAIR0="${NNG_PROTO_PAIR0}" \
          -DNNG_PROTO_PAIR1="${NNG_PROTO_PAIR1}" \
          -DNNG_PROTO_PUB0="${NNG_PROTO_PUB0}" \
          -DNNG_PROTO_SUB0="${NNG_PROTO_SUB0}" \
          -DNNG_PROTO_PUSH0="${NNG_PROTO_PUSH0}" \
          -DNNG_PROTO_PULL0="${NNG_PROTO_PULL0}" \
          -DNNG_PROTO_REQ0="${NNG_PROTO_REQ0}" \
          -DNNG_PROTO_REP0="${NNG_PROTO_REP0}" \
          -DNNG_PROTO_SURVEYOR0="${NNG_PROTO_SURVEYOR0}" \
          -DNNG_PROTO_RESPONDENT0="${NNG_PROTO_RESPONDENT0}" \
          -DNNG_TRANSPORT_TCP="${NNG_TRANSPORT_TCP}" \
          -DNNG_TRANSPORT_TLS="${NNG_TRANSPORT_TLS}" \
          -DNNG_TRANSPORT_WS="${NNG_TRANSPORT_WS}" \
          -DNNG_TRANSPORT_WSS="${NNG_TRANSPORT_WSS}" \
          -DNNG_TRANSPORT_UDP="${NNG_TRANSPORT_UDP}" \
          -DNNG_TRANSPORT_DTLS="${NNG_TRANSPORT_DTLS}" \
          -DNNG_TRANSPORT_IPC="${NNG_TRANSPORT_IPC}" \
          -DNNG_TRANSPORT_INPROC="${NNG_TRANSPORT_INPROC}" \
          -DNNG_TRANSPORT_FDC="${NNG_TRANSPORT_FDC}"

    echo "==> 配置完成"
}

build() {
    local build_type="$1"

    # 仅在 CMakeCache 不存在或 build type 变化时重新配置
    local need_configure=false
    if [ ! -f "${BUILD_DIR}/CMakeCache.txt" ]; then
        need_configure=true
    else
        local cached_type
        cached_type=$(grep 'CMAKE_BUILD_TYPE:' "${BUILD_DIR}/CMakeCache.txt" 2>/dev/null | cut -d= -f2)
        if [ "$cached_type" != "$(cmake_build_type "$build_type")" ]; then
            need_configure=true
        fi
    fi

    if $need_configure; then
        configure "${build_type}"
    fi

    echo "==> 开始编译..."

    local jobs
    jobs=$(get_jobs)

    if [ -f "${BUILD_DIR}/build.ninja" ]; then
        ninja -C "${BUILD_DIR}" -j"${jobs}"
    else
        cmake --build "${BUILD_DIR}" -j"${jobs}"
    fi

    echo "==> 编译完成"
    echo "==> 库文件位于: ${BUILD_DIR}/libnng.a"
}

do_install() {
    local build_type="${1:-${CURRENT_BUILD_TYPE}}"

    build "${build_type}"

    echo "==> 安装到 ${INSTALL_DIR} ..."
    cmake --install "${BUILD_DIR}"
    echo "==> 安装完成"
}

run_tests() {
    echo "==> 运行测试..."
    cd "${BUILD_DIR}"
    ctest --output-on-failure
    echo "==> 测试完成"
}

# ============================================================
# AUX 封装库编译
# ============================================================

build_aux() {
    local build_type="${1:-${CURRENT_BUILD_TYPE}}"
    local cbt
    cbt=$(cmake_build_type "$build_type")

    # 先确保 NNG 已编译（强制用静态库，见 configure() 中的 BUILD_SHARED_LIBS=OFF）
    if [ ! -f "${BUILD_DIR}/libnng.a" ]; then
        echo "==> NNG 尚未编译，先编译 NNG..."
        build "$build_type"
    fi

    echo "==> 编译 AUX 封装库 (${cbt})..."

    mkdir -p "${AUX_BUILD_DIR}"

    # 传递 NNG 协议宏定义，用于条件编译裁剪 aux_msg
    local nng_defines=""
    for proto in PUB0 SUB0 REQ0 REP0 PUSH0 PULL0 BUS0 PAIR0 PAIR1 SURVEYOR0 RESPONDENT0; do
        local var="NNG_PROTO_${proto}"
        [ "${!var}" = "ON" ] && nng_defines="${nng_defines} -DNNG_HAVE_${proto}"
    done

    cmake -S "${SCRIPT_DIR}/aux_msg" \
          -B "${AUX_BUILD_DIR}" \
          -DCMAKE_BUILD_TYPE="${cbt}" \
          -DCMAKE_C_FLAGS="${nng_defines}" \
          -DNNG_INCLUDE_DIRS="${SCRIPT_DIR}/include" \
          -DNNG_LIBRARIES="${BUILD_DIR}/libnng.a"

    cmake --build "${AUX_BUILD_DIR}" -j"$(get_jobs)"

    echo "==> AUX 封装库编译完成"
    echo "==> 库文件位于: ${AUX_BUILD_DIR}/libaux_msg.*"
    echo "==> 示例程序位于: ${AUX_BUILD_DIR}/examples/"
}

# 执行单个命令
run_cmd() {
    local cmd="$1"
    case "${cmd}" in
        debug)
            CURRENT_BUILD_TYPE="debug"
            build "${CURRENT_BUILD_TYPE}"
            ;;
        release)
            CURRENT_BUILD_TYPE="release"
            build "${CURRENT_BUILD_TYPE}"
            ;;
        minimal)
            apply_minimal
            ;;
        clean)
            clean
            ;;
        install)
            do_install "${CURRENT_BUILD_TYPE}"
            ;;
        config)
            interactive_config
            build "${CURRENT_BUILD_TYPE}"
            ;;
        test)
            build "${CURRENT_BUILD_TYPE}"
            run_tests
            ;;
        aux_msg)
            build_aux "${CURRENT_BUILD_TYPE}"
            ;;
        help|--help|-h)
            show_help
            ;;
        *)
            echo "未知命令: ${cmd}"
            show_help
            exit 1
            ;;
    esac
}

# 第一个参数可能是预设（minimal），后续是 debug/release/aux_msg 等
# 逐个执行
for cmd in "${ALL_ARGS[@]}"; do
    echo ""
    echo "==> 执行: ${cmd}"
    run_cmd "${cmd}"
done
