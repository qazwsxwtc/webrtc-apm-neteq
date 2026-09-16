cmake_minimum_required(VERSION 3.16)
project(webrtc_apm_neteq LANGUAGES C CXX VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(APM_BUILD_EXAMPLES "Build example programs" ON)
option(APM_BUILD_SHARED "Build shared libraries instead of static" OFF)
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

# ========================= 平台检测 =========================

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

# ========================= 架构检测 =========================
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
        "WebRTC APM + NetEq only supports: x86, x86_64, armv7, aarch64. "
        "Check CMAKE_SYSTEM_PROCESSOR or CMAKE_OSX_ARCHITECTURES.")
endif()

set(WEBRTC_USE_NEON FALSE)
set(WEBRTC_USE_SSE2 FALSE)

if(WEBRTC_ARCH_ARM OR WEBRTC_ARCH_ARM64)
    set(WEBRTC_USE_NEON TRUE)
    add_compile_definitions(WEBRTC_HAS_NEON)
elseif(WEBRTC_ARCH_X86 OR WEBRTC_ARCH_X86_64)
    set(WEBRTC_USE_SSE2 TRUE)
    add_compile_definitions(WEBRTC_HAS_SSE2)
endif()

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
            add_compile_options(-mfpu=neon -mfloat-abi=hard)
        elseif(WEBRTC_ARCH_ARM64)
            add_compile_options(-march=armv8-a+simd)
        endif()
    endif()
endif()

# ========================= 头文件搜索路径 =========================

include_directories(
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
    ${CMAKE_CURRENT_SOURCE_DIR}/sdk/android/native_api/stacktrace
    ${CMAKE_CURRENT_SOURCE_DIR}/modules/third_party/fft
)

# ========================= 源码收集 =========================

file(GLOB_RECURSE APM_SOURCES CONFIGURE_DEPENDS
    "modules/audio_processing/*.cc"
)
list(FILTER APM_SOURCES EXCLUDE REGEX "unittest")
list(FILTER APM_SOURCES EXCLUDE REGEX "_test\\.cc$")
list(FILTER APM_SOURCES EXCLUDE REGEX "mips")
list(FILTER APM_SOURCES EXCLUDE REGEX "/mock/")
list(FILTER APM_SOURCES EXCLUDE REGEX "/aec_dump/")
list(APPEND APM_SOURCES
    modules/audio_processing/aec_dump/null_aec_dump_factory.cc
)
list(FILTER APM_SOURCES EXCLUDE REGEX "fixed_gain_controller\\.cc$")
list(FILTER APM_SOURCES EXCLUDE REGEX "agc2_testing_common\\.cc$")
list(FILTER APM_SOURCES EXCLUDE REGEX "rnn_vad_tool\\.cc$")
list(FILTER APM_SOURCES EXCLUDE REGEX "click_annotate\\.cc$")
if(NOT WEBRTC_USE_NEON)
    list(FILTER APM_SOURCES EXCLUDE REGEX "neon")
endif()

file(GLOB_RECURSE APM_HEADERS CONFIGURE_DEPENDS
    "modules/audio_processing/*.h"
    "modules/audio_processing/*.hpp"
    "modules/audio_processing/*.hh"
)
list(FILTER APM_HEADERS EXCLUDE REGEX "unittest")
list(FILTER APM_HEADERS EXCLUDE REGEX "_test\\.h")
list(FILTER APM_HEADERS EXCLUDE REGEX "mips")
list(FILTER APM_HEADERS EXCLUDE REGEX "/mock/")
list(FILTER APM_HEADERS EXCLUDE REGEX "/aec_dump/")
list(FILTER APM_HEADERS EXCLUDE REGEX "fixed_gain_controller\\.h$")
list(FILTER APM_HEADERS EXCLUDE REGEX "agc2_testing_common\\.h$")
list(FILTER APM_HEADERS EXCLUDE REGEX "rnn_vad_tool\\.h$")
list(FILTER APM_HEADERS EXCLUDE REGEX "click_annotate\\.h$")
if(NOT WEBRTC_USE_NEON)
    list(FILTER APM_HEADERS EXCLUDE REGEX "neon")
endif()

file(GLOB NETEQ_SOURCES CONFIGURE_DEPENDS
    "modules/audio_coding/neteq/*.cc"
)
list(FILTER NETEQ_SOURCES EXCLUDE REGEX "unittest")
list(FILTER NETEQ_SOURCES EXCLUDE REGEX "_test")
list(FILTER NETEQ_SOURCES EXCLUDE REGEX "mips")

file(GLOB_RECURSE NETEQ_HEADERS CONFIGURE_DEPENDS
    "modules/audio_coding/neteq/*.h"
    "modules/audio_coding/neteq/*.hpp"
)
list(FILTER NETEQ_HEADERS EXCLUDE REGEX "unittest")
list(FILTER NETEQ_HEADERS EXCLUDE REGEX "_test")
list(FILTER NETEQ_HEADERS EXCLUDE REGEX "mips")

file(GLOB COMMON_AUDIO_SOURCES CONFIGURE_DEPENDS
    "common_audio/*.cc"
    "common_audio/*.c"
    "common_audio/resampler/*.cc"
    "common_audio/resampler/*.c"
    "common_audio/signal_processing/*.c"
    "common_audio/signal_processing/*.cc"
    "common_audio/vad/*.c"
    "common_audio/vad/*.cc"
    "common_audio/third_party/ooura/fft_size_128/ooura_fft.cc"
    "common_audio/third_party/ooura/fft_size_128/ooura_fft_sse2.cc"
    "common_audio/third_party/ooura/fft_size_128/ooura_fft_neon.cc"
    "common_audio/third_party/ooura/fft_size_256/fft4g.cc"
    "common_audio/third_party/spl_sqrt_floor/spl_sqrt_floor.c"
)
list(FILTER COMMON_AUDIO_SOURCES EXCLUDE REGEX "unittest")
list(FILTER COMMON_AUDIO_SOURCES EXCLUDE REGEX "_test")
list(FILTER COMMON_AUDIO_SOURCES EXCLUDE REGEX "mips")
if(NOT WEBRTC_USE_NEON)
    list(FILTER COMMON_AUDIO_SOURCES EXCLUDE REGEX "neon")
endif()
if(NOT WEBRTC_USE_SSE2)
    list(FILTER COMMON_AUDIO_SOURCES EXCLUDE REGEX "sse")
endif()

file(GLOB COMMON_AUDIO_HEADERS CONFIGURE_DEPENDS
    "common_audio/*.h"
    "common_audio/*.hpp"
    "common_audio/include/**/*.h"
    "common_audio/resampler/*.h"
    "common_audio/resampler/include/**/*.h"
    "common_audio/signal_processing/*.h"
    "common_audio/signal_processing/include/**/*.h"
    "common_audio/vad/*.h"
    "common_audio/vad/include/**/*.h"
    "common_audio/third_party/ooura/fft_size_128/*.h"
    "common_audio/third_party/ooura/fft_size_256/*.h"
    "common_audio/third_party/spl_sqrt_floor/*.h"
)
list(FILTER COMMON_AUDIO_HEADERS EXCLUDE REGEX "unittest")
list(FILTER COMMON_AUDIO_HEADERS EXCLUDE REGEX "_test")
list(FILTER COMMON_AUDIO_HEADERS EXCLUDE REGEX "mips")
if(NOT WEBRTC_USE_NEON)
    list(FILTER COMMON_AUDIO_HEADERS EXCLUDE REGEX "neon")
endif()
if(NOT WEBRTC_USE_SSE2)
    list(FILTER COMMON_AUDIO_HEADERS EXCLUDE REGEX "sse")
endif()

file(GLOB_RECURSE RTC_BASE_HEADERS CONFIGURE_DEPENDS
    "rtc_base/*.h"
    "rtc_base/*.hpp"
)
list(FILTER RTC_BASE_HEADERS EXCLUDE REGEX "unittest")
list(FILTER RTC_BASE_HEADERS EXCLUDE REGEX "_test")

file(GLOB_RECURSE API_AUDIO_HEADERS CONFIGURE_DEPENDS
    "api/audio/*.h"
    "api/audio/*.hpp"
)
list(FILTER API_AUDIO_HEADERS EXCLUDE REGEX "unittest")
list(FILTER API_AUDIO_HEADERS EXCLUDE REGEX "_test")

file(GLOB_RECURSE API_TASK_QUEUE_HEADERS CONFIGURE_DEPENDS
    "api/task_queue/*.h"
    "api/task_queue/*.hpp"
)
list(FILTER API_TASK_QUEUE_HEADERS EXCLUDE REGEX "unittest|_test")

file(GLOB_RECURSE API_UNITS_HEADERS CONFIGURE_DEPENDS
    "api/units/*.h"
    "api/units/*.hpp"
)
list(FILTER API_UNITS_HEADERS EXCLUDE REGEX "unittest")
list(FILTER API_UNITS_HEADERS EXCLUDE REGEX "_test")

file(GLOB_RECURSE API_NETEQ_HEADERS CONFIGURE_DEPENDS
    "api/neteq/*.h"
    "api/neteq/*.hpp"
)
list(FILTER API_NETEQ_HEADERS EXCLUDE REGEX "unittest")
list(FILTER API_NETEQ_HEADERS EXCLUDE REGEX "_test")

# ========================= rtc_base 源文件（按平台拆分） =========================

set(RTC_BASE_SOURCES
    rtc_base/async_resolver_interface.cc
    rtc_base/async_socket.cc
    rtc_base/bit_buffer.cc
    rtc_base/buffer_queue.cc
    rtc_base/byte_buffer.cc
    rtc_base/checks.cc
    rtc_base/copy_on_write_buffer.cc
    rtc_base/crc32.cc
    rtc_base/critical_section.cc
    rtc_base/event.cc
    rtc_base/event_tracer.cc
    rtc_base/fake_clock.cc
    rtc_base/ip_address.cc
    rtc_base/location.cc
    rtc_base/logging.cc
    rtc_base/log_sinks.cc
    rtc_base/message_handler.cc
    rtc_base/net_helpers.cc
    rtc_base/null_socket_server.cc
    rtc_base/platform_thread.cc
    rtc_base/platform_thread_types.cc
    rtc_base/race_checker.cc
    rtc_base/rate_statistics.cc
    rtc_base/rate_tracker.cc
    rtc_base/signal_thread.cc
    rtc_base/socket.cc
    rtc_base/socket_address.cc
    rtc_base/stream.cc
    rtc_base/string_encode.cc
    rtc_base/string_to_number.cc
    rtc_base/string_utils.cc
    rtc_base/task_queue.cc
    rtc_base/task_queue_stdlib.cc
    rtc_base/thread.cc
    rtc_base/time_utils.cc
    rtc_base/unique_id_generator.cc
    rtc_base/zero_memory.cc
    rtc_base/experiments/field_trial_list.cc
    rtc_base/experiments/field_trial_parser.cc
    rtc_base/experiments/field_trial_units.cc
    rtc_base/experiments/struct_parameters_parser.cc
    rtc_base/memory/aligned_malloc.cc
    rtc_base/memory/fifo_buffer.cc
    rtc_base/numerics/event_based_exponential_moving_average.cc
    rtc_base/numerics/event_rate_counter.cc
    rtc_base/numerics/exp_filter.cc
    rtc_base/numerics/histogram_percentile_counter.cc
    rtc_base/numerics/moving_average.cc
    rtc_base/numerics/samples_stats_counter.cc
    rtc_base/numerics/sample_counter.cc
    rtc_base/numerics/sample_stats.cc
    rtc_base/strings/audio_format_to_string.cc
    rtc_base/strings/json.cc
    rtc_base/strings/string_builder.cc
    rtc_base/strings/string_format.cc
    rtc_base/synchronization/sequence_checker.cc
    rtc_base/synchronization/rw_lock_wrapper.cc
    rtc_base/synchronization/yield_policy.cc
    rtc_base/system/file_wrapper.cc
    rtc_base/task_utils/pending_task_safety_flag.cc
    rtc_base/task_utils/repeating_task.cc
    rtc_base/third_party/base64/base64.cc
    rtc_base/third_party/sigslot/sigslot.cc
    third_party/abseil-cpp/absl/strings/ascii.cc
    third_party/abseil-cpp/absl/strings/match.cc
    third_party/abseil-cpp/absl/strings/internal/memutil.cc
)

# Windows 专属源文件
if(WIN32)
    list(APPEND RTC_BASE_SOURCES
        rtc_base/task_queue_win.cc
        rtc_base/win32.cc
        rtc_base/synchronization/rw_lock_win.cc
    )
else()
    list(APPEND RTC_BASE_SOURCES
        rtc_base/synchronization/rw_lock_posix.cc
    )
endif()

# Android 专属源文件
if(ANDROID)
    list(APPEND RTC_BASE_SOURCES
        rtc_base/ifaddrs_android.cc
        rtc_base/system/thread_registry.cc
        rtc_base/system/warn_current_thread_is_deadlocked.cc
    )
endif()

set(SYSTEM_WRAPPERS_SOURCES
    system_wrappers/source/clock.cc
    system_wrappers/source/cpu_info.cc
    system_wrappers/source/field_trial.cc
    system_wrappers/source/metrics.cc
    system_wrappers/source/rtp_to_ntp_estimator.cc
    system_wrappers/source/sleep.cc
)

# cpu_features: cpu_features.cc 提供通用的 WebRtc_GetCPUInfo，
# cpu_features_linux.c 提供 Linux/Android ARM 平台特有的 WebRtc_GetCPUFeaturesARM
if(WIN32)
    list(APPEND SYSTEM_WRAPPERS_SOURCES system_wrappers/source/cpu_features.cc)
elseif(APPLE)
    list(APPEND SYSTEM_WRAPPERS_SOURCES system_wrappers/source/cpu_features.cc)
elseif(UNIX)
    list(APPEND SYSTEM_WRAPPERS_SOURCES system_wrappers/source/cpu_features.cc)
    if(WEBRTC_ARCH_ARM_FAMILY)
        list(APPEND SYSTEM_WRAPPERS_SOURCES system_wrappers/source/cpu_features_linux.c)
    endif()
else()
    list(APPEND SYSTEM_WRAPPERS_SOURCES system_wrappers/source/cpu_features.cc)
endif()

file(GLOB SYSTEM_WRAPPERS_HEADERS CONFIGURE_DEPENDS
    "system_wrappers/include/**/*.h"
    "system_wrappers/source/*.h"
)

file(GLOB APM_API_SOURCES CONFIGURE_DEPENDS
    "api/audio/*.cc"
    "api/units/*.cc"
)
list(FILTER APM_API_SOURCES EXCLUDE REGEX "unittest")
list(FILTER APM_API_SOURCES EXCLUDE REGEX "_test")

file(GLOB TASK_QUEUE_SOURCES CONFIGURE_DEPENDS "api/task_queue/*.cc")
list(FILTER TASK_QUEUE_SOURCES EXCLUDE REGEX "unittest|_test")

# task_queue_factory 按平台过滤
if(WIN32)
    list(FILTER TASK_QUEUE_SOURCES EXCLUDE REGEX "default_task_queue_factory_(gcd|libevent|stdlib)")
elseif(APPLE)
    list(FILTER TASK_QUEUE_SOURCES EXCLUDE REGEX "default_task_queue_factory_(win|stdlib|libevent)")
elseif(ANDROID)
    list(FILTER TASK_QUEUE_SOURCES EXCLUDE REGEX "default_task_queue_factory_(win|libevent|gcd)")
else()
    list(FILTER TASK_QUEUE_SOURCES EXCLUDE REGEX "default_task_queue_factory_(win|libevent|gcd)")
endif()
list(APPEND APM_API_SOURCES ${TASK_QUEUE_SOURCES})

file(GLOB NETEQ_API_SOURCES CONFIGURE_DEPENDS
    "api/neteq/*.cc"
)
list(FILTER NETEQ_API_SOURCES EXCLUDE REGEX "unittest")
list(FILTER NETEQ_API_SOURCES EXCLUDE REGEX "_test")

file(GLOB API_AUDIO_CODECS_SOURCES CONFIGURE_DEPENDS
    "api/audio_codecs/*.cc"
    "api/audio_codecs/isac/*.cc"
)
list(FILTER API_AUDIO_CODECS_SOURCES EXCLUDE REGEX "unittest|_test")

file(GLOB API_RTP_SOURCES CONFIGURE_DEPENDS
    "api/rtp*.cc"
)
list(FILTER API_RTP_SOURCES EXCLUDE REGEX "unittest|_test")

# ========================= 工程定义 =========================

add_library(rtc_base ${LIB_TYPE}
    ${RTC_BASE_SOURCES}
    ${RTC_BASE_HEADERS}
)
set_target_properties(rtc_base PROPERTIES FOLDER "webrtc/base")

add_library(common_audio ${LIB_TYPE}
    ${COMMON_AUDIO_SOURCES}
    ${COMMON_AUDIO_HEADERS}
)
set_target_properties(common_audio PROPERTIES FOLDER "webrtc/base")

add_library(system_wrappers ${LIB_TYPE}
    ${SYSTEM_WRAPPERS_SOURCES}
    ${SYSTEM_WRAPPERS_HEADERS}
)
set_target_properties(system_wrappers PROPERTIES FOLDER "webrtc/base")

# ===== third_party 独立库 =====

file(GLOB PFFFT_HEADERS CONFIGURE_DEPENDS
    "third_party/pffft/src/*.h"
)
add_library(pffft ${LIB_TYPE}
    third_party/pffft/src/pffft.c
    ${PFFFT_HEADERS}
)
target_compile_definitions(pffft PRIVATE _USE_MATH_DEFINES)
set_target_properties(pffft PROPERTIES FOLDER "webrtc/third_party")

file(GLOB RNN_VAD_HEADERS CONFIGURE_DEPENDS
    "third_party/rnnoise/src/*.h"
)
add_library(rnn_vad ${LIB_TYPE}
    third_party/rnnoise/src/rnn_vad_weights.cc
    ${RNN_VAD_HEADERS}
)
set_target_properties(rnn_vad PROPERTIES FOLDER "webrtc/third_party")

file(GLOB JSONCPP_HEADERS CONFIGURE_DEPENDS
    "third_party/jsoncpp/source/include/json/*.h"
    "third_party/jsoncpp/source/src/lib_json/*.h"
)
add_library(jsoncpp ${LIB_TYPE}
    third_party/jsoncpp/source/src/lib_json/json_reader.cpp
    third_party/jsoncpp/source/src/lib_json/json_value.cpp
    third_party/jsoncpp/source/src/lib_json/json_writer.cpp
    ${JSONCPP_HEADERS}
)
target_compile_definitions(jsoncpp PRIVATE JSON_USE_EXCEPTION=0 JSON_USE_NULLREF=0)
set_target_properties(jsoncpp PROPERTIES FOLDER "webrtc/third_party")

file(GLOB FFT_HEADERS CONFIGURE_DEPENDS
    "modules/third_party/fft/*.h"
)
add_library(fft ${LIB_TYPE}
    modules/third_party/fft/fft.c
    ${FFT_HEADERS}
)
set_target_properties(fft PROPERTIES FOLDER "webrtc/third_party")

# ===== modules/audio_coding/codecs 独立库 =====

file(GLOB LEGACY_ENC_HEADERS CONFIGURE_DEPENDS
    "modules/audio_coding/codecs/legacy_encoded_audio_frame*.h"
)
add_library(legacy_encoded_audio_frame ${LIB_TYPE}
    modules/audio_coding/codecs/legacy_encoded_audio_frame.cc
    ${LEGACY_ENC_HEADERS}
)
set_target_properties(legacy_encoded_audio_frame PROPERTIES FOLDER "webrtc/codecs")

file(GLOB CNG_HEADERS CONFIGURE_DEPENDS
    "modules/audio_coding/codecs/cng/*.h"
)
add_library(webrtc_cng ${LIB_TYPE}
    modules/audio_coding/codecs/cng/webrtc_cng.cc
    ${CNG_HEADERS}
)
set_target_properties(webrtc_cng PROPERTIES FOLDER "webrtc/codecs")

file(GLOB_RECURSE ISAC_MAIN_SOURCE_HEADERS CONFIGURE_DEPENDS
    "modules/audio_coding/codecs/isac/main/source/*.h"
    "modules/audio_coding/codecs/isac/main/include/*.h"
)
file(GLOB_RECURSE ISAC_FIX_SOURCE_HEADERS CONFIGURE_DEPENDS
    "modules/audio_coding/codecs/isac/fix/source/*.h"
    "modules/audio_coding/codecs/isac/fix/include/*.h"
)

add_library(isac_vad ${LIB_TYPE}
    modules/audio_coding/codecs/isac/main/source/filter_functions.c
    modules/audio_coding/codecs/isac/main/source/isac_vad.c
    modules/audio_coding/codecs/isac/main/source/pitch_estimator.c
    modules/audio_coding/codecs/isac/main/source/pitch_filter.c
    ${ISAC_MAIN_SOURCE_HEADERS}
)
set_target_properties(isac_vad PROPERTIES FOLDER "webrtc/codecs")
target_link_libraries(isac_vad PUBLIC fft)

add_library(isac_c ${LIB_TYPE}
    modules/audio_coding/codecs/isac/main/source/arith_routines.c
    modules/audio_coding/codecs/isac/main/source/arith_routines_hist.c
    modules/audio_coding/codecs/isac/main/source/arith_routines_logist.c
    modules/audio_coding/codecs/isac/main/source/bandwidth_estimator.c
    modules/audio_coding/codecs/isac/main/source/crc.c
    modules/audio_coding/codecs/isac/main/source/decode.c
    modules/audio_coding/codecs/isac/main/source/decode_bwe.c
    modules/audio_coding/codecs/isac/main/source/encode.c
    modules/audio_coding/codecs/isac/main/source/encode_lpc_swb.c
    modules/audio_coding/codecs/isac/main/source/entropy_coding.c
    modules/audio_coding/codecs/isac/main/source/filterbanks.c
    modules/audio_coding/codecs/isac/main/source/intialize.c
    modules/audio_coding/codecs/isac/main/source/isac.c
    modules/audio_coding/codecs/isac/main/source/lattice.c
    modules/audio_coding/codecs/isac/main/source/lpc_analysis.c
    modules/audio_coding/codecs/isac/main/source/lpc_gain_swb_tables.c
    modules/audio_coding/codecs/isac/main/source/lpc_shape_swb12_tables.c
    modules/audio_coding/codecs/isac/main/source/lpc_shape_swb16_tables.c
    modules/audio_coding/codecs/isac/main/source/lpc_tables.c
    modules/audio_coding/codecs/isac/main/source/pitch_gain_tables.c
    modules/audio_coding/codecs/isac/main/source/pitch_lag_tables.c
    modules/audio_coding/codecs/isac/main/source/spectrum_ar_model_tables.c
    modules/audio_coding/codecs/isac/main/source/transform.c
    ${ISAC_MAIN_SOURCE_HEADERS}
)
set_target_properties(isac_c PROPERTIES FOLDER "webrtc/codecs")
target_link_libraries(isac_c PUBLIC fft isac_vad)

add_library(isac_fix_common ${LIB_TYPE}
    modules/audio_coding/codecs/isac/fix/source/fft.c
    modules/audio_coding/codecs/isac/fix/source/transform_tables.c
    ${ISAC_FIX_SOURCE_HEADERS}
)
if(WEBRTC_USE_NEON)
    target_sources(isac_fix_common PRIVATE
        modules/audio_coding/codecs/isac/fix/source/transform_neon.c
    )
endif()
set_target_properties(isac_fix_common PROPERTIES FOLDER "webrtc/codecs")

add_library(isac_fix_c ${LIB_TYPE}
    modules/audio_coding/codecs/isac/fix/source/arith_routines.c
    modules/audio_coding/codecs/isac/fix/source/arith_routines_hist.c
    modules/audio_coding/codecs/isac/fix/source/arith_routines_logist.c
    modules/audio_coding/codecs/isac/fix/source/bandwidth_estimator.c
    modules/audio_coding/codecs/isac/fix/source/decode.c
    modules/audio_coding/codecs/isac/fix/source/decode_bwe.c
    modules/audio_coding/codecs/isac/fix/source/decode_plc.c
    modules/audio_coding/codecs/isac/fix/source/encode.c
    modules/audio_coding/codecs/isac/fix/source/entropy_coding.c
    modules/audio_coding/codecs/isac/fix/source/filterbank_tables.c
    modules/audio_coding/codecs/isac/fix/source/filterbanks.c
    modules/audio_coding/codecs/isac/fix/source/filters.c
    modules/audio_coding/codecs/isac/fix/source/initialize.c
    modules/audio_coding/codecs/isac/fix/source/isacfix.c
    modules/audio_coding/codecs/isac/fix/source/lattice.c
    modules/audio_coding/codecs/isac/fix/source/lattice_c.c
    modules/audio_coding/codecs/isac/fix/source/lpc_masking_model.c
    modules/audio_coding/codecs/isac/fix/source/lpc_tables.c
    modules/audio_coding/codecs/isac/fix/source/pitch_estimator.c
    modules/audio_coding/codecs/isac/fix/source/pitch_estimator_c.c
    modules/audio_coding/codecs/isac/fix/source/pitch_filter.c
    modules/audio_coding/codecs/isac/fix/source/pitch_filter_c.c
    modules/audio_coding/codecs/isac/fix/source/pitch_gain_tables.c
    modules/audio_coding/codecs/isac/fix/source/pitch_lag_tables.c
    modules/audio_coding/codecs/isac/fix/source/spectrum_ar_model_tables.c
    modules/audio_coding/codecs/isac/fix/source/transform.c
    ${ISAC_FIX_SOURCE_HEADERS}
)
if(WEBRTC_USE_NEON)
    target_sources(isac_fix_c PRIVATE
        modules/audio_coding/codecs/isac/fix/source/entropy_coding_neon.c
        modules/audio_coding/codecs/isac/fix/source/filterbanks_neon.c
        modules/audio_coding/codecs/isac/fix/source/filters_neon.c
        modules/audio_coding/codecs/isac/fix/source/lattice_neon.c
    )
endif()
set_target_properties(isac_fix_c PROPERTIES FOLDER "webrtc/codecs")
target_link_libraries(isac_fix_c PUBLIC fft isac_fix_common)

add_library(isac ${LIB_TYPE}
    modules/audio_coding/codecs/isac/main/source/audio_decoder_isac.cc
    modules/audio_coding/codecs/isac/main/source/audio_encoder_isac.cc
    ${ISAC_MAIN_SOURCE_HEADERS}
)
set_target_properties(isac PROPERTIES FOLDER "webrtc/codecs")
target_link_libraries(isac PUBLIC isac_c)

add_library(isac_fix ${LIB_TYPE}
    modules/audio_coding/codecs/isac/fix/source/audio_decoder_isacfix.cc
    modules/audio_coding/codecs/isac/fix/source/audio_encoder_isacfix.cc
    ${ISAC_FIX_SOURCE_HEADERS}
)
set_target_properties(isac_fix PROPERTIES FOLDER "webrtc/codecs")
target_link_libraries(isac_fix PUBLIC isac_fix_c)

# ========================= 通用编译定义和平台链接 =========================

set(WEBRTC_TARGETS
    rtc_base common_audio system_wrappers
    pffft rnn_vad jsoncpp fft
    legacy_encoded_audio_frame webrtc_cng
    isac_vad isac_c isac_fix_common isac_fix_c isac isac_fix
)

if(APPLE)
    find_library(COREFOUNDATION_FRAMEWORK CoreFoundation)
endif()

foreach(_target ${WEBRTC_TARGETS})
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
        target_link_libraries(${_target} PRIVATE pthread ${COREFOUNDATION_FRAMEWORK})
    elseif(UNIX)
        target_link_libraries(${_target} PRIVATE pthread dl resolv rt m)
    endif()
endforeach()

# ========================= 顶层 APM / NetEQ 静态库 =========================

add_library(webrtc_apm ${LIB_TYPE}
    ${APM_SOURCES}
    ${APM_HEADERS}
    ${APM_API_SOURCES}
    ${API_AUDIO_CODECS_SOURCES}
    ${API_RTP_SOURCES}
    ${API_AUDIO_HEADERS}
    ${API_TASK_QUEUE_HEADERS}
    ${API_UNITS_HEADERS}
)
target_compile_definitions(webrtc_apm PRIVATE
    WEBRTC_APM_DEBUG_DUMP=0
    $<$<CONFIG:Debug>:RTC_DCHECK_IS_ON=1>
    $<$<CONFIG:Release>:RTC_DCHECK_IS_ON=0>
)
target_link_libraries(webrtc_apm PUBLIC
    rtc_base common_audio system_wrappers
    pffft rnn_vad jsoncpp
    isac_vad isac_c isac_fix_c isac_fix_common fft
    legacy_encoded_audio_frame
)

if(WIN32)
    target_link_libraries(webrtc_apm PRIVATE winmm ws2_32)
elseif(APPLE)
    find_library(COREAUDIO_FRAMEWORK CoreAudio)
    find_library(AUDIOTOOLBOX_FRAMEWORK AudioToolbox)
    target_link_libraries(webrtc_apm PUBLIC pthread ${COREFOUNDATION_FRAMEWORK})
    target_link_libraries(webrtc_apm PRIVATE ${COREAUDIO_FRAMEWORK} ${AUDIOTOOLBOX_FRAMEWORK})
elseif(ANDROID)
    target_link_libraries(webrtc_apm PRIVATE log)
else()
    target_link_libraries(webrtc_apm PRIVATE pthread dl resolv rt m)
endif()
set_target_properties(webrtc_apm PROPERTIES FOLDER "webrtc")

add_library(webrtc_neteq ${LIB_TYPE}
    ${NETEQ_SOURCES}
    ${NETEQ_HEADERS}
    ${NETEQ_API_SOURCES}
    ${API_AUDIO_CODECS_SOURCES}
    ${API_RTP_SOURCES}
    ${API_NETEQ_HEADERS}
)
target_compile_definitions(webrtc_neteq PRIVATE
    WEBRTC_APM_DEBUG_DUMP=0
    $<$<CONFIG:Debug>:RTC_DCHECK_IS_ON=1>
    $<$<CONFIG:Release>:RTC_DCHECK_IS_ON=0>
)
target_link_libraries(webrtc_neteq PUBLIC
    rtc_base common_audio system_wrappers
    pffft fft
    webrtc_cng
    isac isac_vad isac_c isac_fix isac_fix_c isac_fix_common
    legacy_encoded_audio_frame
)

if(WIN32)
    target_link_libraries(webrtc_neteq PRIVATE winmm ws2_32)
elseif(APPLE)
    find_library(COREAUDIO_FRAMEWORK CoreAudio)
    find_library(AUDIOTOOLBOX_FRAMEWORK AudioToolbox)
    target_link_libraries(webrtc_neteq PUBLIC pthread ${COREFOUNDATION_FRAMEWORK})
    target_link_libraries(webrtc_neteq PRIVATE ${COREAUDIO_FRAMEWORK} ${AUDIOTOOLBOX_FRAMEWORK})
elseif(ANDROID)
    target_link_libraries(webrtc_neteq PRIVATE log)
else()
    target_link_libraries(webrtc_neteq PRIVATE pthread dl resolv rt m)
endif()
set_target_properties(webrtc_neteq PROPERTIES FOLDER "webrtc")

# ========================= Example =========================

if(APM_BUILD_EXAMPLES AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/example")
    file(GLOB EXAMPLE_SOURCES CONFIGURE_DEPENDS "example/*.cc")
    file(GLOB EXAMPLE_HEADERS CONFIGURE_DEPENDS "example/*.h")
    foreach(example_src ${EXAMPLE_SOURCES})
        get_filename_component(example_name ${example_src} NAME_WE)
        add_executable(${example_name} ${example_src} ${EXAMPLE_HEADERS})
        if(UNIX AND NOT APPLE)
            target_link_libraries(${example_name} PRIVATE
                "-Wl,--start-group"
                webrtc_apm webrtc_neteq
                "-Wl,--end-group"
                pthread dl resolv rt m
            )
        else()
            target_link_libraries(${example_name} PRIVATE webrtc_apm webrtc_neteq)
            if(WIN32)
                target_link_libraries(${example_name} PRIVATE winmm ws2_32)
            elseif(ANDROID)
                target_link_libraries(${example_name} PRIVATE log)
            elseif(APPLE)
                target_link_libraries(${example_name} PRIVATE pthread ${COREFOUNDATION_FRAMEWORK})
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