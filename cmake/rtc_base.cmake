# cmake/rtc_base.cmake — rtc_base + system_wrappers + common_audio + third_party 小库
# ============================================================
# rtc_base / system_wrappers 已经是显式硬编码文件列表（安全、可控）。
# common_audio 用 GLOB 但路径明确（不 RECURSE），也比较安全。
# third_party 的 4 个小库（pffft / rnn_vad / jsoncpp / fft）源文件极少，直接列。
# ============================================================

# -------- rtc_base --------
# 显式硬编码 + 平台分支 —— 最安全的模式，与 WebRTC 上游 BUILD.gn 一致
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
# platform-specific 源文件
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
if(ANDROID)
    list(APPEND RTC_BASE_SOURCES
        rtc_base/ifaddrs_android.cc
        rtc_base/system/thread_registry.cc
        rtc_base/system/warn_current_thread_is_deadlocked.cc
    )
endif()
webrtc_source_summary("rtc_base sources" RTC_BASE_SOURCES)

# -------- system_wrappers --------
set(SYSTEM_WRAPPERS_SOURCES
    system_wrappers/source/clock.cc
    system_wrappers/source/cpu_info.cc
    system_wrappers/source/field_trial.cc
    system_wrappers/source/metrics.cc
    system_wrappers/source/rtp_to_ntp_estimator.cc
    system_wrappers/source/sleep.cc
)
# cpu_features.cc 提供通用的 WebRtc_GetCPUInfo；
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
webrtc_source_summary("system_wrappers sources" SYSTEM_WRAPPERS_SOURCES)

file(GLOB SYSTEM_WRAPPERS_HEADERS CONFIGURE_DEPENDS
    "system_wrappers/include/**/*.h"
    "system_wrappers/source/*.h"
)

# -------- common_audio --------
# GLOB 明确列出各子目录 —— 非 RECURSE，安全性介于硬编码和全盘扫之间
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
webrtc_filter_standard(COMMON_AUDIO_SOURCES)
webrtc_filter_platform_simd(COMMON_AUDIO_SOURCES)
webrtc_source_summary("common_audio sources" COMMON_AUDIO_SOURCES)

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
webrtc_filter_standard(COMMON_AUDIO_HEADERS)
webrtc_filter_platform_simd(COMMON_AUDIO_HEADERS)

# -------- third_party: pffft / rnn_vad / jsoncpp / fft --------
# 这 4 个库源文件极少，直接 file(GLOB *.c *.cc) 即可

file(GLOB PFFFT_SOURCES CONFIGURE_DEPENDS "third_party/pffft/src/pffft.c")
file(GLOB PFFFT_HEADERS CONFIGURE_DEPENDS "third_party/pffft/src/*.h")

file(GLOB RNN_VAD_SOURCES CONFIGURE_DEPENDS "third_party/rnnoise/src/*.cc")
file(GLOB RNN_VAD_HEADERS CONFIGURE_DEPENDS "third_party/rnnoise/src/*.h")

file(GLOB JSONCPP_SOURCES CONFIGURE_DEPENDS
    "third_party/jsoncpp/source/src/lib_json/json_reader.cpp"
    "third_party/jsoncpp/source/src/lib_json/json_value.cpp"
    "third_party/jsoncpp/source/src/lib_json/json_writer.cpp"
)
file(GLOB JSONCPP_HEADERS CONFIGURE_DEPENDS
    "third_party/jsoncpp/source/include/**/*.h"
    "third_party/jsoncpp/generated/**/*.h"
)

file(GLOB FFT_SOURCES CONFIGURE_DEPENDS "modules/third_party/fft/fft.c")
file(GLOB FFT_HEADERS CONFIGURE_DEPENDS "modules/third_party/fft/*.h")

set(LEGACY_ENC_SOURCES
    modules/audio_coding/codecs/legacy_encoded_audio_frame.cc
)
file(GLOB LEGACY_ENC_HEADERS CONFIGURE_DEPENDS
    "modules/audio_coding/codecs/legacy_encoded_audio_frame*.h"
)

file(GLOB CNG_SOURCES CONFIGURE_DEPENDS "modules/audio_coding/codecs/cng/*.cc")
webrtc_filter_standard(CNG_SOURCES)
file(GLOB CNG_HEADERS CONFIGURE_DEPENDS "modules/audio_coding/codecs/cng/*.h")
webrtc_filter_standard(CNG_HEADERS)