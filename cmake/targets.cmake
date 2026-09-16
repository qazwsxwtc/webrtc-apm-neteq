# cmake/targets.cmake — 17 个独立库 + 最终 webrtc_apm / webrtc_neteq 汇总库
# ============================================================
# 按依赖顺序声明 —— 下层先 add，上层后 add
# ============================================================

# -------- 下层第三方库 --------
add_library(pffft ${LIB_TYPE} ${PFFFT_SOURCES} ${PFFFT_HEADERS})
set_target_properties(pffft PROPERTIES FOLDER "webrtc/third_party")

add_library(rnn_vad ${LIB_TYPE} ${RNN_VAD_SOURCES} ${RNN_VAD_HEADERS})
set_target_properties(rnn_vad PROPERTIES FOLDER "webrtc/third_party")

add_library(jsoncpp ${LIB_TYPE} ${JSONCPP_SOURCES} ${JSONCPP_HEADERS})
set_target_properties(jsoncpp PROPERTIES FOLDER "webrtc/third_party")

add_library(fft ${LIB_TYPE} ${FFT_SOURCES} ${FFT_HEADERS})
set_target_properties(fft PROPERTIES FOLDER "webrtc/third_party")

# -------- audio_coding: legacy + CNG --------
add_library(legacy_encoded_audio_frame ${LIB_TYPE}
    ${LEGACY_ENC_SOURCES}
    ${LEGACY_ENC_HEADERS}
)
set_target_properties(legacy_encoded_audio_frame PROPERTIES FOLDER "webrtc/codecs")

add_library(webrtc_cng ${LIB_TYPE} ${CNG_SOURCES} ${CNG_HEADERS})
set_target_properties(webrtc_cng PROPERTIES FOLDER "webrtc/codecs")
target_link_libraries(webrtc_cng PUBLIC rtc_base system_wrappers)

# -------- ISAC 全家桶（main + fix）--------
file(GLOB_RECURSE ISAC_MAIN_SOURCE_HEADERS CONFIGURE_DEPENDS
    "modules/audio_coding/codecs/isac/main/source/*.h"
    "modules/audio_coding/codecs/isac/main/include/*.h"
)
file(GLOB_RECURSE ISAC_FIX_SOURCE_HEADERS CONFIGURE_DEPENDS
    "modules/audio_coding/codecs/isac/fix/source/*.h"
    "modules/audio_coding/codecs/isac/fix/include/*.h"
)

# isac_vad: 纯 C，VAD + pitch estimator + filter —— ISAC main 的依赖
add_library(isac_vad ${LIB_TYPE}
    modules/audio_coding/codecs/isac/main/source/filter_functions.c
    modules/audio_coding/codecs/isac/main/source/isac_vad.c
    modules/audio_coding/codecs/isac/main/source/pitch_estimator.c
    modules/audio_coding/codecs/isac/main/source/pitch_filter.c
    ${ISAC_MAIN_SOURCE_HEADERS}
)
set_target_properties(isac_vad PROPERTIES FOLDER "webrtc/codecs")
target_link_libraries(isac_vad PUBLIC fft)

# isac_c: ISAC main 编解码器核心 —— 22 个 .c
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

# isac_fix_common: ISAC fix 公共部分
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

# isac_fix_c: ISAC fix 编解码器核心 —— 26 个 .c
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

# isac / isac_fix: 薄封装 C++ 层，只是 audio_decoder_isac.cc + audio_encoder_isac.cc
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

# -------- 上层核心库 --------
add_library(rtc_base ${LIB_TYPE}
    ${RTC_BASE_SOURCES}
    ${RTC_BASE_HEADERS}
)
set_target_properties(rtc_base PROPERTIES FOLDER "webrtc")

add_library(common_audio ${LIB_TYPE}
    ${COMMON_AUDIO_SOURCES}
    ${COMMON_AUDIO_HEADERS}
)
set_target_properties(common_audio PROPERTIES FOLDER "webrtc")
target_link_libraries(common_audio PUBLIC rtc_base)

add_library(system_wrappers ${LIB_TYPE}
    ${SYSTEM_WRAPPERS_SOURCES}
    ${SYSTEM_WRAPPERS_HEADERS}
)
set_target_properties(system_wrappers PROPERTIES FOLDER "webrtc")
target_link_libraries(system_wrappers PUBLIC rtc_base)

# -------- webrtc_apm / webrtc_neteq 汇总 --------
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
target_link_libraries(webrtc_apm PUBLIC
    rtc_base common_audio system_wrappers
    pffft rnn_vad jsoncpp
    isac_vad isac_c isac_fix_c isac_fix_common fft
    legacy_encoded_audio_frame
)
set_target_properties(webrtc_apm PROPERTIES FOLDER "webrtc")

add_library(webrtc_neteq ${LIB_TYPE}
    ${NETEQ_SOURCES}
    ${NETEQ_HEADERS}
    ${NETEQ_API_SOURCES}
    ${API_AUDIO_CODECS_SOURCES}
    ${API_RTP_SOURCES}
    ${API_NETEQ_HEADERS}
)
target_link_libraries(webrtc_neteq PUBLIC
    rtc_base common_audio system_wrappers
    pffft fft
    webrtc_cng
    isac isac_vad isac_c isac_fix isac_fix_c isac_fix_common
    legacy_encoded_audio_frame
)
set_target_properties(webrtc_neteq PROPERTIES FOLDER "webrtc")

# Apple 上 webrtc_apm/neteq 需要把 pthread + CoreFoundation PUBLIC 传出去
# （下层库 webrtc_finalize_target 里只 PRIVATE 链接了，consumer 看不到）
# 同时 PRIVATE 链接 CoreAudio + AudioToolbox（AudioUnit）—— 只 APM 内部用
if(APPLE)
    find_library(_cf CoreFoundation)
    find_library(_ca CoreAudio)
    find_library(_at AudioToolbox)
    target_link_libraries(webrtc_apm PUBLIC pthread ${_cf} PRIVATE ${_ca} ${_at})
    target_link_libraries(webrtc_neteq PUBLIC pthread ${_cf} PRIVATE ${_ca} ${_at})
endif()

# -------- 批量注入 compile defs + 平台 link --------
# 前面每个 target 只做了 FOLDER / link_library，这里统一注入 compile defs + 平台库
set(WEBRTC_ALL_TARGETS
    rtc_base common_audio system_wrappers
    pffft rnn_vad jsoncpp fft
    legacy_encoded_audio_frame webrtc_cng
    isac_vad isac_c isac_fix_common isac_fix_c isac isac_fix
    webrtc_apm webrtc_neteq
)

foreach(_t ${WEBRTC_ALL_TARGETS})
    if(TARGET ${_t})
        webrtc_finalize_target(${_t})
    endif()
endforeach()