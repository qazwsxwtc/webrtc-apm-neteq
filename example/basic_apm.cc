#include <cmath>
#include <cstdio>
#include <cstring>
#include <vector>

#include "modules/audio_processing/include/audio_processing.h"

static int16_t g_sample_count = 0;

static void FillNoise(int16_t* data, size_t samples) {
    for (size_t i = 0; i < samples; ++i) {
        data[i] = static_cast<int16_t>((std::rand() % 20000) - 10000);
    }
}

static int16_t ComputeRms(const int16_t* data, size_t samples) {
    double sum = 0.0;
    for (size_t i = 0; i < samples; ++i) {
        sum += static_cast<double>(data[i]) * static_cast<double>(data[i]);
    }
    return static_cast<int16_t>(std::sqrt(sum / static_cast<double>(samples)));
}

int main() {
    const int kSampleRate = 16000;
    const size_t kChannels = 1;
    const size_t kFrameSamples = kSampleRate / 100;

    printf("=== basic_apm: HighPassFilter + NoiseReduction ===\n");
    printf("Sample rate : %d Hz\n", kSampleRate);
    printf("Channels    : %zu\n", kChannels);
    printf("Frame size  : %zu samples (10 ms)\n\n", kFrameSamples);

    std::unique_ptr<webrtc::AudioProcessing> apm(
        webrtc::AudioProcessingBuilder().Create());

    webrtc::AudioProcessing::Config config;
    config.high_pass_filter.enabled = true;
    config.noise_suppression.enabled = true;
    config.noise_suppression.level =
        webrtc::AudioProcessing::Config::NoiseSuppression::kHigh;
    config.level_estimation.enabled = true;
    apm->ApplyConfig(config);

    webrtc::ProcessingConfig pconfig;
    pconfig.input_stream() = webrtc::StreamConfig(kSampleRate, kChannels);
    pconfig.output_stream() = webrtc::StreamConfig(kSampleRate, kChannels);
    pconfig.reverse_input_stream() = webrtc::StreamConfig(kSampleRate, kChannels);
    pconfig.reverse_output_stream() = webrtc::StreamConfig(kSampleRate, kChannels);
    apm->Initialize(pconfig);

    std::vector<int16_t> input(kFrameSamples * kChannels);
    std::vector<int16_t> output(kFrameSamples * kChannels);

    printf("%-6s %-10s %-10s %-10s\n", "Frame", "InputRMS", "OutputRMS", "Attenuation");
    printf("----------------------------------------\n");

    for (int frame = 0; frame < 20; ++frame) {
        FillNoise(input.data(), input.size());

        int16_t in_rms = ComputeRms(input.data(), input.size());

        int err = apm->ProcessStream(
            input.data(),
            pconfig.input_stream(),
            pconfig.output_stream(),
            output.data());

        if (err != webrtc::AudioProcessing::kNoError) {
            printf("ProcessStream error: %d\n", err);
            return 1;
        }

        int16_t out_rms = ComputeRms(output.data(), output.size());
        double atten_db = (in_rms > 0 && out_rms > 0)
            ? 20.0 * std::log10(static_cast<double>(out_rms) / static_cast<double>(in_rms))
            : 0.0;

        printf("%-6d %-10d %-10d %-10.1f dB\n", frame, in_rms, out_rms, atten_db);
    }

    printf("\n=== Done ===\n");
    return 0;
}