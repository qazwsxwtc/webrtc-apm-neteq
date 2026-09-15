#include <cmath>
#include <cstdio>
#include <cstring>
#include <vector>
#include "modules/audio_processing/include/audio_processing.h"

static void FillSpeechSegment(int16_t* data, size_t samples, int seed) {
    for (size_t i = 0; i < samples; ++i) {
        double t = i / 16000.0;
        double f1 = 200.0 + (seed % 3) * 100.0;
        double f2 = 800.0 + (seed % 4) * 200.0;
        data[i] = static_cast<int16_t>(
            6000.0 * std::sin(2.0 * M_PI * f1 * t) +
            3000.0 * std::sin(2.0 * M_PI * f2 * t) +
            ((std::rand() % 600) - 300));
    }
}

static void FillSilenceSegment(int16_t* data, size_t samples) {
    for (size_t i = 0; i < samples; ++i) {
        data[i] = static_cast<int16_t>((std::rand() % 80) - 40);
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

    printf("=== VAD Demo: Voice Activity Detection ===\n");
    printf("Sample rate  : %d Hz\n", kSampleRate);
    printf("Frame size   : %zu samples (10 ms)\n", kFrameSamples);
    printf("Input pattern: SPEECH blocks with SILENCE gaps\n\n");

    std::unique_ptr<webrtc::AudioProcessing> apm(
        webrtc::AudioProcessingBuilder().Create());

    webrtc::AudioProcessing::Config config;
    config.voice_detection.enabled = true;
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

    int sequence[] = {
        0, 0, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0,
        1, 1, 1, 1, 1, 0, 0, 1, 1, 0, 0, 0, 1, 1, 1, 0, 0, 1, 0, 0,
    };
    int total_frames = sizeof(sequence) / sizeof(sequence[0]);

    int speech_detected = 0;
    int silence_detected = 0;
    int speech_frames = 0;
    int silence_frames = 0;

    printf("%-6s %-8s %-10s %-12s %-10s\n",
           "Frame", "Expect", "VAD", "InputRMS", "Level_dB");
    printf("---------------------------------------------------\n");

    for (int frame = 0; frame < total_frames; ++frame) {
        bool expect_speech = (sequence[frame] == 1);
        if (expect_speech) {
            FillSpeechSegment(input.data(), kFrameSamples, frame);
            speech_frames++;
        } else {
            FillSilenceSegment(input.data(), kFrameSamples);
            silence_frames++;
        }

        int16_t in_rms = ComputeRms(input.data(), kFrameSamples);

        apm->ProcessStream(input.data(),
                           pconfig.input_stream(),
                           pconfig.output_stream(),
                           output.data());

        auto stats = apm->GetStatistics();
        bool detected = stats.voice_detected.value_or(false);
        int level_db = stats.output_rms_dbfs.value_or(-127);

        if (detected) speech_detected++;
        else silence_detected++;

        const char* expect_str = expect_speech ? "VOICE" : "SILENCE";
        const char* vad_str = detected ? "VOICE" : "SILENCE";

        printf("%-6d %-8s %-10s %-12d %-10d%s\n",
               frame, expect_str, vad_str, in_rms, level_db,
               (expect_speech != detected) ? "  <-- MISMATCH" : "");
    }

    printf("\n=== Summary ===\n");
    printf("Total frames   : %d\n", total_frames);
    printf("Expected voice : %d\n", speech_frames);
    printf("Expected silence: %d\n", silence_frames);
    printf("Detected voice : %d\n", speech_detected);
    printf("Detected silence: %d\n", silence_detected);
    printf("\n=== Done ===\n");
    return 0;
}