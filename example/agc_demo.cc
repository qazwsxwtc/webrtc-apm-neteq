#include <cmath>
#include <cstdio>
#include <cstring>
#include <vector>
#include "modules/audio_processing/include/audio_processing.h"

static void FillTone(float* data, size_t samples, float freq, float amplitude) {
    for (size_t i = 0; i < samples; ++i) {
        double t = i / 16000.0;
        data[i] = static_cast<float>(amplitude * std::sin(2.0 * M_PI * freq * t));
    }
}

static float ComputeRms(const float* data, size_t samples) {
    double sum = 0.0;
    for (size_t i = 0; i < samples; ++i)
        sum += static_cast<double>(data[i]) * static_cast<double>(data[i]);
    return static_cast<float>(std::sqrt(sum / static_cast<double>(samples)));
}

int main() {
    const int kSampleRate = 16000;
    const size_t kChannels = 1;
    const size_t kFrameSamples = kSampleRate / 100;

    printf("=== AGC2 Demo: Automatic Gain Control ===\n");
    printf("Sample rate    : %d Hz\n", kSampleRate);
    printf("Frame size     : %zu samples (10 ms)\n", kFrameSamples);
    printf("Target level   : -20 dBFS (AGC2 adaptive digital)\n");
    printf("Input pattern  : varying amplitude tone\n\n");

    std::unique_ptr<webrtc::AudioProcessing> apm(
        webrtc::AudioProcessingBuilder().Create());

    webrtc::AudioProcessing::Config config;
    config.gain_controller2.enabled = true;
    config.gain_controller2.fixed_digital.gain_db = 0.0f;
    config.gain_controller2.adaptive_digital.enabled = true;
    config.gain_controller2.adaptive_digital.level_estimator =
        webrtc::AudioProcessing::Config::GainController2::kRms;
    config.gain_controller2.adaptive_digital.use_saturation_protector = true;
    config.gain_controller2.adaptive_digital.extra_saturation_margin_db = 2.0f;
    config.level_estimation.enabled = true;
    apm->ApplyConfig(config);

    webrtc::ProcessingConfig pconfig;
    pconfig.input_stream() = webrtc::StreamConfig(kSampleRate, kChannels);
    pconfig.output_stream() = webrtc::StreamConfig(kSampleRate, kChannels);
    pconfig.reverse_input_stream() = webrtc::StreamConfig(kSampleRate, kChannels);
    pconfig.reverse_output_stream() = webrtc::StreamConfig(kSampleRate, kChannels);
    apm->Initialize(pconfig);

    std::vector<float> input(kFrameSamples);
    std::vector<float> output(kFrameSamples);
    float* input_ptrs[] = { input.data() };
    float* output_ptrs[] = { output.data() };

    float amplitudes[] = { 0.005f, 0.01f, 0.02f, 0.05f, 0.1f, 0.2f, 0.008f, 0.03f };
    const int kNumAmplitudes = sizeof(amplitudes) / sizeof(amplitudes[0]);

    printf("%-6s %-12s %-12s %-12s %-10s\n",
           "Frame", "InputRMS", "OutputRMS", "OutRMS_dB", "InputAmp");
    printf("------------------------------------------------------\n");

    for (int seq = 0; seq < 5; ++seq) {
        for (int a = 0; a < kNumAmplitudes; ++a) {
            int frame = seq * kNumAmplitudes + a;
            FillTone(input.data(), kFrameSamples, 440.0f, amplitudes[a]);

            float in_rms = ComputeRms(input.data(), kFrameSamples);

            apm->ProcessStream(
                const_cast<const float* const*>(input_ptrs),
                pconfig.input_stream(),
                pconfig.output_stream(),
                output_ptrs);

            float out_rms = ComputeRms(output.data(), kFrameSamples);
            auto stats = apm->GetStatistics();
            int level_db = stats.output_rms_dbfs.value_or(-127);

            printf("%-6d %-12.6f %-12.6f %-12d %-10.3f\n",
                   frame, in_rms, out_rms, level_db, amplitudes[a]);
        }
    }

    printf("\n=== Done ===\n");
    return 0;
}