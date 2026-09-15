#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <vector>

#include "modules/audio_processing/include/audio_processing.h"

static void FillSine(float* data, size_t samples, float freq, int frame_index) {
    for (size_t i = 0; i < samples; ++i) {
        double t = (frame_index * samples + i) / 16000.0;
        data[i] = static_cast<float>(0.3 * std::sin(2.0 * M_PI * freq * t));
    }
}

static void FillNoise(float* data, size_t samples, float amplitude) {
    for (size_t i = 0; i < samples; ++i) {
        data[i] = static_cast<float>(std::rand()) / RAND_MAX * 2.0f * amplitude - amplitude;
    }
}

static float ComputeRms(const float* data, size_t samples) {
    double sum = 0.0;
    for (size_t i = 0; i < samples; ++i) {
        sum += static_cast<double>(data[i]) * static_cast<double>(data[i]);
    }
    return static_cast<float>(std::sqrt(sum / static_cast<double>(samples)));
}

int main() {
    const int kSampleRate = 16000;
    const size_t kCaptureChannels = 1;
    const size_t kRenderChannels = 1;
    const size_t kFrameSamples = kSampleRate / 100;

    printf("=== apm_pipeline: Full AEC3 + NS + AGC2 + HPF + VAD ===\n");
    printf("Capture: %d Hz x %zu ch, Render: %d Hz x %zu ch\n",
           kSampleRate, kCaptureChannels, kSampleRate, kRenderChannels);
    printf("Frame: %zu samples (10 ms)\n\n", kFrameSamples);

    std::unique_ptr<webrtc::AudioProcessing> apm(
        webrtc::AudioProcessingBuilder().Create());

    webrtc::AudioProcessing::Config config;
    config.pipeline.maximum_internal_processing_rate = 48000;

    config.high_pass_filter.enabled = true;

    config.echo_canceller.enabled = true;
    config.echo_canceller.mobile_mode = true;
    config.residual_echo_detector.enabled = true;

    config.noise_suppression.enabled = true;
    config.noise_suppression.level =
        webrtc::AudioProcessing::Config::NoiseSuppression::kHigh;

    config.gain_controller2.enabled = true;
    config.gain_controller2.fixed_digital.gain_db = 0.0f;
    config.gain_controller2.adaptive_digital.enabled = true;

    config.voice_detection.enabled = true;
    config.level_estimation.enabled = true;

    apm->ApplyConfig(config);

    webrtc::ProcessingConfig pconfig;
    pconfig.input_stream() = webrtc::StreamConfig(kSampleRate, kCaptureChannels);
    pconfig.output_stream() = webrtc::StreamConfig(kSampleRate, kCaptureChannels);
    pconfig.reverse_input_stream() = webrtc::StreamConfig(kSampleRate, kRenderChannels);
    pconfig.reverse_output_stream() = webrtc::StreamConfig(kSampleRate, kRenderChannels);
    apm->Initialize(pconfig);

    std::vector<float> render_ch(kFrameSamples);
    std::vector<float> capture_ch(kFrameSamples);
    std::vector<float> output_ch(kFrameSamples);
    float* render_ptrs[] = { render_ch.data() };
    float* capture_ptrs[] = { capture_ch.data() };
    float* output_ptrs[] = { output_ch.data() };

    printf("%-6s %-10s %-10s %-10s %-8s %-8s %-8s\n",
           "Frame", "RenderRMS", "InCapture", "OutCapture", "VAD", "Delay", "Clipped");
    printf("----------------------------------------------------------------------\n");

    for (int frame = 0; frame < 50; ++frame) {
        FillSine(render_ch.data(), kFrameSamples, 440.0f, frame);
        float render_rms = ComputeRms(render_ch.data(), kFrameSamples);

        FillSine(capture_ch.data(), kFrameSamples, 440.0f, frame);
        FillNoise(capture_ch.data(), kFrameSamples, 0.05f);
        float in_rms = ComputeRms(capture_ch.data(), kFrameSamples);

        int err = apm->ProcessReverseStream(
            const_cast<const float* const*>(render_ptrs),
            pconfig.reverse_input_stream(),
            pconfig.reverse_output_stream(),
            render_ptrs);
        if (err != webrtc::AudioProcessing::kNoError) {
            printf("ProcessReverseStream error: %d\n", err);
            return 1;
        }

        if (frame == 0 || frame == 10 || frame == 30) {
            apm->set_stream_delay_ms(80);
        } else {
            apm->set_stream_delay_ms(0);
        }

        err = apm->ProcessStream(
            const_cast<const float* const*>(capture_ptrs),
            pconfig.input_stream(),
            pconfig.output_stream(),
            output_ptrs);
        if (err != webrtc::AudioProcessing::kNoError) {
            printf("ProcessStream error: %d\n", err);
            return 1;
        }

        float out_rms = ComputeRms(output_ch.data(), kFrameSamples);

        auto stats = apm->GetStatistics();
        const char* vad_str = stats.voice_detected.value_or(false) ? "VOICE" : "SILENCE";
        int delay = stats.delay_ms.value_or(-1);
        int level = stats.output_rms_dbfs.value_or(0);

        printf("%-6d %-10.4f %-10.4f %-10.4f %-8s %-8d %-8d\n",
               frame, render_rms, in_rms, out_rms, vad_str, delay, level);
    }

    printf("\n=== Done ===\n");
    return 0;
}