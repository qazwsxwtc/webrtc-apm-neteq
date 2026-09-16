# cmake/apm.cmake — APM 核心源文件 + api/ 接口
# ============================================================
# 定义：
#   APM_SOURCES / APM_HEADERS
#   APM_API_SOURCES（api/audio + api/units + api/task_queue）
#   NETEQ_API_SOURCES（api/neteq）
#   API_AUDIO_CODECS_SOURCES / API_RTP_SOURCES
#   API_AUDIO_HEADERS / API_TASK_QUEUE_HEADERS / API_UNITS_HEADERS / API_NETEQ_HEADERS
#
# APM_SOURCES 设计讨论：
#   上游 WebRTC 的 modules/audio_processing/ 下有 ~20 个子目录
#   （aec3/ agc2/ nsx/ echo_canceller3/ gain_control/ noise_suppression/ ...）
#   且每隔几个 commit 就会新增子目录（如 rnnoise_v3/、echo_detector/）。
#   用 GLOB_RECURSE 全盘扫 + 黑名单过滤的代价是：
#   - 上游加了不该编的东西（如 video_utils/ 依赖 video 模块）会静默漏编
#   - 排除规则全是正则字符串，看不出"为什么排除"
#
#   折中方案：
#   1. 用 GLOB_RECURSE 但限制在 modules/audio_processing/*.cc/c 两个目录
#   2. 标准排除交给 webrtc_filter_standard()（unittest / _test / mips / mock）
#   3. 特殊排除 **必须写注释** 说明"为什么"
#   4. 特殊文件 **必须 list(APPEND)** 显式加回去（如 null_aec_dump_factory.cc）
#   5. configure 时 webrtc_source_summary() 打印数量 —— 同步上游后先看数字对不对
# ============================================================

# -------- APM 实现源文件 --------
file(GLOB_RECURSE APM_SOURCES CONFIGURE_DEPENDS
    "modules/audio_processing/*.cc"
    "modules/audio_processing/*.c"
)
webrtc_filter_standard(APM_SOURCES)
webrtc_filter_platform_simd(APM_SOURCES)

# -- 特殊排除：非测试文件但不参与静态库 --------
# fixed_gain_controller.cc 是 APM 的内部 stub，生产构建不使用
list(FILTER APM_SOURCES EXCLUDE REGEX "fixed_gain_controller\\.cc$")
# agc2_testing_common.cc 是 AGC2 的单元测试辅助代码
list(FILTER APM_SOURCES EXCLUDE REGEX "agc2_testing_common\\.cc$")
# rnn_vad_tool.cc 是 rnnoise VAD 的离线命令行工具
list(FILTER APM_SOURCES EXCLUDE REGEX "rnn_vad_tool\\.cc$")
# click_annotate.cc 是录音标注工具（用于训练数据生成）
list(FILTER APM_SOURCES EXCLUDE REGEX "click_annotate\\.cc$")
# aec_dump/ 整个目录是 AEC3 dump 录制/回放工具，依赖 Chromium base
list(FILTER APM_SOURCES EXCLUDE REGEX "/aec_dump/")

# -- 特殊加回：null_aec_dump_factory.cc --------
# 被 /aec_dump/ 排除规则误伤的"空实现"，AEC3 在非 dump 模式下需要它
list(APPEND APM_SOURCES
    modules/audio_processing/aec_dump/null_aec_dump_factory.cc
)

webrtc_source_summary("APM sources" APM_SOURCES)

# -------- NetEQ 实现源文件 --------
# NETEQ 用单层 GLOB（非 RECURSE）扫 modules/audio_coding/neteq/*.cc
# 路径单一、目录扁平，比 APM 安全得多
file(GLOB NETEQ_SOURCES CONFIGURE_DEPENDS
    "modules/audio_coding/neteq/*.cc"
    "modules/audio_coding/neteq/*.c"
)
webrtc_filter_standard(NETEQ_SOURCES)
webrtc_source_summary("NETEQ sources" NETEQ_SOURCES)

file(GLOB NETEQ_HEADERS CONFIGURE_DEPENDS
    "modules/audio_coding/neteq/*.h"
    "modules/audio_coding/neteq/*.hpp"
)
webrtc_filter_standard(NETEQ_HEADERS)

# -------- APM 头文件 --------
file(GLOB_RECURSE APM_HEADERS CONFIGURE_DEPENDS
    "modules/audio_processing/*.h"
    "modules/audio_processing/*.hpp"
)
webrtc_filter_standard(APM_HEADERS)
webrtc_filter_platform_simd(APM_HEADERS)
list(FILTER APM_HEADERS EXCLUDE REGEX "fixed_gain_controller\\.h$")
list(FILTER APM_HEADERS EXCLUDE REGEX "agc2_testing_common\\.h$")
list(FILTER APM_HEADERS EXCLUDE REGEX "rnn_vad_tool\\.h$")
list(FILTER APM_HEADERS EXCLUDE REGEX "click_annotate\\.h$")
list(FILTER APM_HEADERS EXCLUDE REGEX "/aec_dump/")
webrtc_source_summary("APM headers" APM_HEADERS)

# -------- api/audio + api/units --------
file(GLOB APM_API_SOURCES CONFIGURE_DEPENDS
    "api/audio/*.cc"
    "api/units/*.cc"
)
webrtc_filter_standard(APM_API_SOURCES)
webrtc_source_summary("API audio + units" APM_API_SOURCES)

# -------- api/task_queue（按平台选 factory）--------
file(GLOB TASK_QUEUE_SOURCES CONFIGURE_DEPENDS
    "api/task_queue/*.cc"
)
webrtc_filter_standard(TASK_QUEUE_SOURCES)

# task_queue_factory 的平台选择：WebRTC 提供 4 种实现（win / stdlib / libevent / gcd）
# 每个平台只编自己那一个，其余全部排除
if(WIN32)
    # Windows 用 default_task_queue_factory_win.cc
    list(FILTER TASK_QUEUE_SOURCES EXCLUDE REGEX "default_task_queue_factory_(gcd|libevent|stdlib)")
elseif(APPLE)
    # macOS / iOS 用 default_task_queue_factory_gcd.cc
    list(FILTER TASK_QUEUE_SOURCES EXCLUDE REGEX "default_task_queue_factory_(win|stdlib|libevent)")
elseif(ANDROID)
    # Android / Linux 默认用 default_task_queue_factory_stdlib.cc
    list(FILTER TASK_QUEUE_SOURCES EXCLUDE REGEX "default_task_queue_factory_(win|libevent|gcd)")
else()
    list(FILTER TASK_QUEUE_SOURCES EXCLUDE REGEX "default_task_queue_factory_(win|libevent|gcd)")
endif()
list(APPEND APM_API_SOURCES ${TASK_QUEUE_SOURCES})
webrtc_source_summary("API task_queue (after platform filter)" TASK_QUEUE_SOURCES)

# -------- api/neteq --------
file(GLOB NETEQ_API_SOURCES CONFIGURE_DEPENDS
    "api/neteq/*.cc"
)
webrtc_filter_standard(NETEQ_API_SOURCES)
webrtc_source_summary("API neteq" NETEQ_API_SOURCES)

# -------- api/audio_codecs + api/rtp --------
file(GLOB API_AUDIO_CODECS_SOURCES CONFIGURE_DEPENDS
    "api/audio_codecs/*.cc"
    "api/audio_codecs/isac/*.cc"
)
webrtc_filter_standard(API_AUDIO_CODECS_SOURCES)
webrtc_source_summary("API audio_codecs" API_AUDIO_CODECS_SOURCES)

file(GLOB API_RTP_SOURCES CONFIGURE_DEPENDS
    "api/rtp*.cc"
)
webrtc_filter_standard(API_RTP_SOURCES)
webrtc_source_summary("API rtp" API_RTP_SOURCES)

# -------- api/* 头文件（全部 RECURSE，仅排除 unittest / _test）--------
file(GLOB_RECURSE API_AUDIO_HEADERS CONFIGURE_DEPENDS "api/audio/*.h" "api/audio/*.hpp")
webrtc_filter_standard(API_AUDIO_HEADERS)

file(GLOB_RECURSE API_TASK_QUEUE_HEADERS CONFIGURE_DEPENDS "api/task_queue/*.h" "api/task_queue/*.hpp")
webrtc_filter_standard(API_TASK_QUEUE_HEADERS)

file(GLOB_RECURSE API_UNITS_HEADERS CONFIGURE_DEPENDS "api/units/*.h" "api/units/*.hpp")
webrtc_filter_standard(API_UNITS_HEADERS)

file(GLOB_RECURSE API_NETEQ_HEADERS CONFIGURE_DEPENDS "api/neteq/*.h" "api/neteq/*.hpp")
webrtc_filter_standard(API_NETEQ_HEADERS)

file(GLOB_RECURSE RTC_BASE_HEADERS CONFIGURE_DEPENDS
    "rtc_base/*.h"
    "rtc_base/*.hpp"
)
webrtc_filter_standard(RTC_BASE_HEADERS)