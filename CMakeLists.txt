cmake_minimum_required(VERSION 3.16)
project(webrtc_apm_neteq LANGUAGES C CXX VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ========================= 用户可选项 =========================
option(APM_BUILD_EXAMPLES "Build example programs" ON)
option(APM_BUILD_SHARED   "Build shared libraries instead of static" OFF)
option(APM_FORCE_STATIC_CRT "Force static CRT (/MT) on MSVC" OFF)

if(APM_BUILD_SHARED)
    set(LIB_TYPE SHARED)
else()
    set(LIB_TYPE STATIC)
endif()

# ========================= 编译选项 =========================
if(MSVC)
    add_compile_options(/W0 /permissive- /MP)
    add_compile_definitions(NOMINMAX WIN32_LEAN_AND_MEAN _CRT_SECURE_NO_WARNINGS _USE_MATH_DEFINES)
    if(APM_FORCE_STATIC_CRT)
        set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    else()
        set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
    endif()
else()
    add_compile_options(-w -fPIC)
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        add_compile_options(-Wall -Wno-unused-parameter -Wno-unused-variable -Wno-unused-but-set-variable)
    endif()
endif()

# ========================= 平台 + 架构检测 =========================
if(WIN32)
    add_compile_definitions(WEBRTC_WIN)
elseif(APPLE)
    add_compile_definitions(WEBRTC_APPLE WEBRTC_MAC WEBRTC_POSIX)
    if(IOS OR CMAKE_SYSTEM_NAME STREQUAL "iOS")
        add_compile_definitions(WEBRTC_IOS)
    else()
        add_compile_definitions(WEBRTC_OSX)
    endif()
elseif(ANDROID)
    add_compile_definitions(WEBRTC_ANDROID WEBRTC_POSIX)
elseif(UNIX)
    add_compile_definitions(WEBRTC_LINUX WEBRTC_POSIX)
endif()

# Apple 平台用 CMAKE_OSX_ARCHITECTURES（交叉编译时 SYSTEM_PROCESSOR 是宿主机架构）
if(APPLE AND CMAKE_OSX_ARCHITECTURES)
    set(_arch_check "${CMAKE_OSX_ARCHITECTURES}")
else()
    set(_arch_check "${CMAKE_SYSTEM_PROCESSOR}")
endif()
if(_arch_check MATCHES "x86_64|AMD64|amd64|X86_64")
    set(WEBRTC_ARCH_X86_64 TRUE)
    add_compile_definitions(WEBRTC_ARCH_X86_64 WEBRTC_ARCH_X86_FAMILY)
elseif(_arch_check MATCHES "x86|i[3-6]86")
    set(WEBRTC_ARCH_X86 TRUE)
    add_compile_definitions(WEBRTC_ARCH_X86 WEBRTC_ARCH_X86_FAMILY)
elseif(_arch_check MATCHES "aarch64|arm64|ARM64|arm64-v8a")
    set(WEBRTC_ARCH_ARM64 TRUE)
    add_compile_definitions(WEBRTC_ARCH_ARM64 WEBRTC_ARCH_ARM_FAMILY)
elseif(_arch_check MATCHES "arm|ARM|armeabi-v7a|armv7")
    set(WEBRTC_ARCH_ARM TRUE)
    add_compile_definitions(WEBRTC_ARCH_ARM WEBRTC_ARCH_ARM_FAMILY)
else()
    message(FATAL_ERROR
        "Unknown architecture '${_arch_check}'. "
        "WebRTC APM + NetEq only supports: x86 x86_64 armv7 aarch64. "
        "Check CMAKE_SYSTEM_PROCESSOR or CMAKE_OSX_ARCHITECTURES.")
endif()

# SIMD 开关：ARM 自动 NEON，x86 自动 SSE2+
set(WEBRTC_USE_NEON FALSE)
set(WEBRTC_USE_SSE2 FALSE)
if(WEBRTC_ARCH_ARM OR WEBRTC_ARCH_ARM64)
    set(WEBRTC_USE_NEON TRUE)
    add_compile_definitions(WEBRTC_HAS_NEON)
elseif(WEBRTC_ARCH_X86 OR WEBRTC_ARCH_X86_64)
    set(WEBRTC_USE_SSE2 TRUE)
    add_compile_definitions(WEBRTC_HAS_SSE2)
endif()

# 架构特定编译选项
if(MSVC)
    if(WEBRTC_ARCH_X86)
        add_compile_options(/arch:SSE2 /fp:fast)
    elseif(WEBRTC_ARCH_X86_64)
        add_compile_options(/arch:AVX2 /fp:fast)
    elseif(WEBRTC_ARCH_ARM)
        add_compile_options(/arch:ARMV7)
    elseif(WEBRTC_ARCH_ARM64)
        add_compile_options(/arch:ARM64)
    endif()
else()
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        if(WEBRTC_ARCH_X86)
            add_compile_options(-msse2 -mfpmath=sse -ffast-math)
        elseif(WEBRTC_ARCH_X86_64)
            add_compile_options(-msse2 -msse3 -mssse3 -msse4.1 -msse4.2 -mfpmath=sse -ffast-math)
        elseif(WEBRTC_ARCH_ARM)
            if(NOT ANDROID)
                add_compile_options(-mfpu=neon -mfloat-abi=hard)
            endif()
        elseif(WEBRTC_ARCH_ARM64)
            add_compile_options(-march=armv8-a+simd)
        endif()
    endif()
endif()

# ========================= 头文件搜索路径 =========================
# 通用路径 —— 所有平台共享
set(_common_include_dirs
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/api
    ${CMAKE_CURRENT_SOURCE_DIR}/common_audio/include
    ${CMAKE_CURRENT_SOURCE_DIR}/common_audio/resampler/include
    ${CMAKE_CURRENT_SOURCE_DIR}/common_audio/signal_processing/include
    ${CMAKE_CURRENT_SOURCE_DIR}/common_audio/vad/include
    ${CMAKE_CURRENT_SOURCE_DIR}/modules/audio_processing/include
    ${CMAKE_CURRENT_SOURCE_DIR}/modules/audio_coding/codecs/isac/main/include
    ${CMAKE_CURRENT_SOURCE_DIR}/modules/audio_coding/codecs/isac/fix/include
    ${CMAKE_CURRENT_SOURCE_DIR}/rtc_base
    ${CMAKE_CURRENT_SOURCE_DIR}/system_wrappers/include
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/abseil-cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/pffft/src
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/rnnoise/src
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/webrtc_overrides
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/jsoncpp/source/include
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/jsoncpp/source/src/lib_json
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/jsoncpp/generated
    ${CMAKE_CURRENT_SOURCE_DIR}/modules/third_party/fft
)
include_directories(${_common_include_dirs})

# Android 专属 —— stacktrace 提供 unwind.h，只有 Android NDK 有这套
if(ANDROID)
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/sdk/android/native_api/stacktrace)
endif()

# ========================= 模块加载 =========================
# 源文件收集逻辑全在 cmake/*.cmake 里 —— 顶层不做 file(GLOB / list(FILTER)
include(cmake/webrtc.cmake)
include(cmake/apm.cmake)
include(cmake/rtc_base.cmake)
include(cmake/targets.cmake)

# ========================= Example =========================
if(APM_BUILD_EXAMPLES AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/example")
    file(GLOB EXAMPLE_SOURCES CONFIGURE_DEPENDS "example/*.cc")
    file(GLOB EXAMPLE_HEADERS CONFIGURE_DEPENDS "example/*.h")
    foreach(example_src ${EXAMPLE_SOURCES})
        get_filename_component(example_name ${example_src} NAME_WE)
        add_executable(${example_name} ${example_src} ${EXAMPLE_HEADERS})
        if(UNIX AND NOT APPLE)
            target_link_libraries(${example_name} PRIVATE
                "-Wl--start-group"
                webrtc_apm webrtc_neteq
                "-Wl--end-group"
                pthread dl resolv rt m
            )
        else()
            target_link_libraries(${example_name} PRIVATE webrtc_apm webrtc_neteq)
            if(WIN32)
                target_link_libraries(${example_name} PRIVATE winmm ws2_32)
            elseif(ANDROID)
                target_link_libraries(${example_name} PRIVATE log)
            elseif(APPLE)
                find_library(_cf CoreFoundation)
                target_link_libraries(${example_name} PRIVATE pthread ${_cf})
            endif()
        endif()
        set_target_properties(${example_name} PROPERTIES FOLDER "example")
    endforeach()
endif()

# ========================= 构建摘要 =========================
message(STATUS "")
message(STATUS "=== WebRTC APM + NetEq Build Configuration ===")
message(STATUS "  Version            : ${PROJECT_VERSION}")
message(STATUS "  Architecture       : ${CMAKE_SYSTEM_PROCESSOR}")
message(STATUS "  Platform           : ${CMAKE_SYSTEM_NAME}")
message(STATUS "  Build type         : ${CMAKE_BUILD_TYPE}")
message(STATUS "  Library type       : ${LIB_TYPE}")
message(STATUS "  NEON optimizations : ${WEBRTC_USE_NEON}")
message(STATUS "  SSE2 optimizations : ${WEBRTC_USE_SSE2}")
message(STATUS "  Build examples     : ${APM_BUILD_EXAMPLES}")
if(MSVC)
    message(STATUS "  MSVC CRT           : ${CMAKE_MSVC_RUNTIME_LIBRARY}")
endif()
message(STATUS "=============================================")
message(STATUS "")

# ========================= VS 解决方案文件夹注入 =========================
if(WIN32 AND CMAKE_GENERATOR MATCHES "Visual Studio")
    find_program(POWERSHELL_EXE powershell)
    if(POWERSHELL_EXE AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/cmake/fix_sln.ps1")
        add_custom_target(fix_sln
            COMMAND ${POWERSHELL_EXE} -ExecutionPolicy Bypass
                -File "${CMAKE_CURRENT_SOURCE_DIR}/cmake/fix_sln.ps1"
                -SlnPath "${CMAKE_BINARY_DIR}/${CMAKE_PROJECT_NAME}.sln"
            COMMENT "Injecting solution folders into .sln ..."
            VERBATIM
        )
        set_target_properties(fix_sln PROPERTIES FOLDER "_utility")
        execute_process(
            COMMAND ${POWERSHELL_EXE} -ExecutionPolicy Bypass
                -File "${CMAKE_CURRENT_SOURCE_DIR}/cmake/fix_sln.ps1"
                -SlnPath "${CMAKE_BINARY_DIR}/${CMAKE_PROJECT_NAME}.sln"
            RESULT_VARIABLE FIX_SLN_RESULT
            OUTPUT_VARIABLE FIX_SLN_OUTPUT
            ERROR_VARIABLE FIX_SLN_ERROR
        )
        if(FIX_SLN_RESULT EQUAL 0)
            message(STATUS "fix_sln.ps1: solution folders injected")
        else()
            message(STATUS "fix_sln.ps1: will run after generate (first configure)")
        endif()
    endif()
    if(POWERSHELL_EXE AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/cmake/fix_filters.ps1")
        add_custom_target(fix_filters
            COMMAND ${POWERSHELL_EXE} -ExecutionPolicy Bypass
                -File "${CMAKE_CURRENT_SOURCE_DIR}/cmake/fix_filters.ps1"
                -BuildDir "${CMAKE_BINARY_DIR}"
                -SourceDir "${CMAKE_CURRENT_SOURCE_DIR}"
            COMMENT "Generating hierarchical .vcxproj.filters ..."
            VERBATIM
        )
        set_target_properties(fix_filters PROPERTIES FOLDER "_utility")
        execute_process(
            COMMAND ${POWERSHELL_EXE} -ExecutionPolicy Bypass
                -File "${CMAKE_CURRENT_SOURCE_DIR}/cmake/fix_filters.ps1"
                -BuildDir "${CMAKE_BINARY_DIR}"
                -SourceDir "${CMAKE_CURRENT_SOURCE_DIR}"
            RESULT_VARIABLE FIX_FILTERS_RESULT
            OUTPUT_VARIABLE FIX_FILTERS_OUTPUT
            ERROR_VARIABLE FIX_FILTERS_ERROR
        )
        if(FIX_FILTERS_RESULT EQUAL 0)
            message(STATUS "fix_filters.ps1: hierarchical filters generated")
        else()
            message(STATUS "fix_filters.ps1: will run after generate (first configure)")
        endif()
    endif()
endif()