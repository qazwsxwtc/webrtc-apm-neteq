#include <cmath>
#include <cstdio>
#include <cstring>
#include <vector>
#include "modules/audio_processing/include/audio_processing.h"

static void FillWhiteNoise(int16_t* data, size_t samples, int amplitude) {
    for (size_t i = 0; i < samples; ++i) {
        data[i] = static_cast<int16_t>(
            (std::rand() % (amplitude * 2 + 1)) - amplitude);
    }
}

static void FillLowFreqNoise(int16_t* data, size_t samples, int amplitude) {
    for (size_t i = 0; i < samples; ++i) {
        double t = i / 16000.0;
        data[i] = static_cast<int16_t>(
            amplitude * std::sin(2.0 * M_PI * 80.0 * t));
    }
}

static void FillSpeechTone(int16_t* data, size_t samples, int frame_index) {
    for (size_t i = 0; i < samples; ++i) {
        double t = (frame_index * samples + i) / 16000.0;
        int tone = (frame_index % 3 == 0) ? 300 : 600;
        data[i] = static_cast<int16_t>(8000.0 * std::sin(2.0 * M_PI * tone * t));
    }
}

static int16_t ComputeRms(const int16_t* data, size_t samples) {
    double sum = 0.0;
    for (size_t i = 0; i < samples; ++i)
        sum += static_cast<double>(data[i]) * static_cast<double>(data[i]);
    return static_cast<int16_t>(std::sqrt(sum / static_cast<double>(samples)));
}

int main() {
    const int kSampleRate = 16000;
    const size_t kChannels = 1;
    const size_t kFrameSamples = kSampleRate / 100;

    printf("=== NS Demo: Noise Suppression ===\n");
    printf("Sample rate  : %d Hz\n", kSampleRate);
    printf("Frame size   : %zu samples (10 ms)\n", kFrameSamples);
    printf("Input signal : speech tone (interleaved with white noise)\n\n");

    std::unique_ptr<webrtc::AudioProcessing> apm(
        webrtc::AudioProcessingBuilder().Create());

    webrtc::AudioProcessing::Config config;
    config.noise_suppression.enabled = true;
    config.noise_suppression.level =
        webrtc::AudioProcessing::Config::NoiseSuppression::kHigh;
    apm->ApplyConfig(config);

    webrtc::ProcessingConfig pconfig;
    pconfig.input_stream() = webrtc::StreamConfig(kSampleRate, kChannels);
    pconfig.output_stream() = webrtc::StreamConfig(kSampleRate, kChannels);
    pconfig.reverse_input_stream() = webrtc::StreamConfig(kSampleRate, kChannels);
    pconfig.reverse_output_stream() = webrtc::StreamConfig(kSampleRate, kChannels);
    apm->Initialize(pconfig);

    std::vector<int16_t> input(kFrameSamples * kChannels);
    std::vector<int16_t> output(kFrameSamples * kChannels);

    printf("%-6s %-12s %-12s %-12s %-10s\n",
           "Frame", "InputRMS", "OutputRMS", "Atten_dB", "Signal");
    printf("------------------------------------------------------\n");

    for (int frame = 0; frame < 50; ++frame) {
        bool is_speech = (frame % 3 == 0);
        if (is_speech) {
            FillSpeechTone(input.data(), kFrameSamples, frame);
            int16_t* p = input.data();
            for (size_t i = 0; i < kFrameSamples; ++i)
                p[i] = static_cast<int16_t>(p[i] + ((std::rand() % 400) - 200));
        } else {
            FillWhiteNoise(input.data(), kFrameSamples, 3000);
        }

        int16_t in_rms = ComputeRms(input.data(), kFrameSamples);

        apm->ProcessStream(input.data(),
                           pconfig.input_stream(),
                           pconfig.output_stream(),
                           output.data());

        int16_t out_rms = ComputeRms(output.data(), kFrameSamples);
        double atten_db = (in_rms > 0 && out_rms > 0)
            ? 20.0 * std::log10(static_cast<double>(out_rms) /
                                static_cast<double>(in_rms))
            : 0.0;

        printf("%-6d %-12d %-12d %-12.1f %-10s\n",
               frame, in_rms, out_rms, atten_db,
               is_speech ? "SPEECH" : "NOISE");
    }

    printf("\n=== Done ===\n");
    return 0;
}