/*
 *  Copyright (c) 2019 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef MODULES_AUDIO_PROCESSING_NS_HISTOGRAMS_H_
#define MODULES_AUDIO_PROCESSING_NS_HISTOGRAMS_H_

#include <array>

#include "api/array_view.h"
#include "modules/audio_processing/ns/ns_common.h"
#include "modules/audio_processing/ns/signal_model.h"

namespace webrtc {

constexpr int kHistogramSize = 1000;

// Class for handling the updating of histograms.
class Histograms {
 public:
  Histograms();
  Histograms(const Histograms&) = delete;
  Histograms& operator=(const Histograms&) = delete;

  // Clears the histograms.
  // 重置所有直方图计数器，通常在初始化或重新校准时使用。.
  void Clear();

  // Extracts thresholds for feature parameters and updates the corresponding
  // histogram.
  //核心更新逻辑。
  //接收当前的 SignalModel（包含提取出的音频特征）。
//根据特征值计算对应的直方图索引（bin）。
// 增加相应 bin 的计数。
//注：虽然头文件未显示具体实现，但通常此类直方图用于动态调整判决阈值或进行概率估计。
  void Update(const SignalModel& features_);

  // Methods for accessing the histograms.
  rtc::ArrayView<const int, kHistogramSize> get_lrt() const { return lrt_; }
  rtc::ArrayView<const int, kHistogramSize> get_spectral_flatness() const {
    return spectral_flatness_;
  }
  rtc::ArrayView<const int, kHistogramSize> get_spectral_diff() const {
    return spectral_diff_;
  }

 private:
  std::array<int, kHistogramSize> lrt_;//似然比测试值。用于衡量当前频谱与噪声模型的匹配程度。
  std::array<int, kHistogramSize> spectral_flatness_;//频谱平坦度。语音通常具有较低的频谱平坦度（有峰值），而白噪声具有较高的平坦度。
  std::array<int, kHistogramSize> spectral_diff_;//频谱差分。反映频谱随时间的变化率，语音变化较快，噪声变化较慢。 
};

}  // namespace webrtc

#endif  // MODULES_AUDIO_PROCESSING_NS_HISTOGRAMS_H_
