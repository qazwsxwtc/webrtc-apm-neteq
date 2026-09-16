# cmake/webrtc.cmake — WebRTC APM/NetEQ 构建公共层
# ============================================================
# 提供：
#   - webrtc_filter_standard(var)      统一排除 unittest / _test / mips / mock
#   - webrtc_filter_platform_simd(var)  NEON / SSE 平台相关排除
#   - webrtc_source_summary(label var)  configure 时打印文件数，便于发现上游变化
#   - webrtc_finalize_target(target)    注入编译定义 + 平台链接库
#
# 设计原则：
#   1. 路径白名单优于 GLOB_RECURSE 全盘扫 —— 上游加了新子目录不会静默被编进
#   2. 特殊排除规则（list(FILTER ...) EXCLUDE REGEX ...）**必须带注释**说明原因
#   3. configure 时每个 target 打印 source/header 数量，便于人工审查

# ------------------------------------------------------------------
# 统一过滤标准（排除测试 / mock / 平台不相关的目录）
# 所有模块共用，避免散落 20+ 处重复 list(FILTER ...)
# ------------------------------------------------------------------
function(webrtc_filter_standard _var)
    list(FILTER ${_var} EXCLUDE REGEX "unittest")
    list(FILTER ${_var} EXCLUDE REGEX "_test")
    list(FILTER ${_var} EXCLUDE REGEX "mips")
    list(FILTER ${_var} EXCLUDE REGEX "/mock/")
    set(${_var} ${${_var}} PARENT_SCOPE)
endfunction()

# ------------------------------------------------------------------
# SIMD 平台相关排除 —— 当 WEBRTC_USE_NEON / WEBRTC_USE_SSE2 关闭时
# 注意：用简单子串匹配 "neon" / "sse" —— 与原始 CMakeLists.txt 保持一致，
# 避免正则过于精确反而漏匹配（例如 ooura_fft_neon.cc / fir_filter_neon.cc）
# ------------------------------------------------------------------
function(webrtc_filter_platform_simd _var)
    if(NOT WEBRTC_USE_NEON)
        list(FILTER ${_var} EXCLUDE REGEX "neon")
    endif()
    if(NOT WEBRTC_USE_SSE2)
        list(FILTER ${_var} EXCLUDE REGEX "sse")
    endif()
    set(${_var} ${${_var}} PARENT_SCOPE)
endfunction()

# ------------------------------------------------------------------
# configure 时打印 "label: N sources, M headers"
# 如果上游加了意外文件（比如扫进了 video 相关的东西），这里的数字会异常跳变
# ------------------------------------------------------------------
function(webrtc_source_summary _label _var)
    list(FIND ${_var} "MODULE_NOT_FOUND" _idx)
    if(_idx LESS 0)
        list(LENGTH ${_var} _count)
        message(STATUS "  ${_label}: ${_count} file(s)")
    else()
        message(STATUS "  ${_label}: (empty — no files matched)")
    endif()
endfunction()

# ------------------------------------------------------------------
# 把一个 target 加上通用 compile definitions + 平台 link libs
# ------------------------------------------------------------------
function(webrtc_finalize_target _target)
    target_compile_definitions(${_target} PRIVATE
        WEBRTC_APM_DEBUG_DUMP=0
        $<$<CONFIG:Debug>:RTC_DCHECK_IS_ON=1>
        $<$<CONFIG:Release>:RTC_DCHECK_IS_ON=0>
    )
    if(WIN32)
        target_link_libraries(${_target} PRIVATE winmm ws2_32)
    elseif(ANDROID)
        target_link_libraries(${_target} PRIVATE log)
    elseif(APPLE)
        find_library(_cf CoreFoundation)
        target_link_libraries(${_target} PRIVATE pthread ${_cf})
    elseif(UNIX)
        target_link_libraries(${_target} PRIVATE pthread dl resolv rt m)
    endif()
    set_target_properties(${_target} PROPERTIES FOLDER "webrtc")
endfunction()