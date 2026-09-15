#include <cmath>
#include <cstdio>
#include <cstring>
#include <vector>
#include "modules/audio_processing/include/audio_processing.h"

static void FillSine(float* data, size_t samples, float freq, float amp, int frame_index) {
    for (size_t i = 0; i < samples; ++i) {
        double t = (frame_index * samples + i) / 16000.0;
        data[i] = static_cast<float>(amp * std::sin(2.0 * M_PI * freq * t));
    }
}

static void FillSpeechSim(float* data, size_t samples, int frame_index) {
    for (size_t i = 0; i < samples; ++i) {
        double t = (frame_index * samples + i) / 16000.0;
        bool voiced = (frame_index % 6 >= 2 && frame_index % 6 <= 4);
        double amp = voiced ? 0.15 : 0.005;
        double f0 = voiced ? 180.0 : 0.0;
        double f1 = voiced ? 600.0 : 0.0;
        data[i] = static_cast<float>(
            amp * std::sin(2.0 * M_PI * f0 * t) +
            amp * 0.5 * std::sin(2.0 * M_PI * f1 * t) +
            amp * 0.1 * ((std::rand() % 2000) / 1000.0 - 1.0));
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
    const int kEchoDelayMs = 40;
    const size_t kDelaySamples = static_cast<size_t>(kEchoDelayMs * kSampleRate / 1000);

    printf("=== Full APM Pipeline ===\n");
    printf("Chain   : HPF -> AEC3 -> NS -> AGC2 -> VAD\n");
    printf("Rate    : %d Hz, 10 ms frames\n", kSampleRate);
    printf("Echo    : %d ms simulated delay (far-end 440 Hz)\n", kEchoDelayMs);
    printf("Speech  : voiced segments (180+600 Hz) + noise\n\n");

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
    config.gain_controller2.adaptive_digital.level_estimator =
        webrtc::AudioProcessing::Config::GainController2::kRms;
    config.gain_controller2.adaptive_digital.use_saturation_protector = true;
    config.gain_controller2.adaptive_digital.extra_saturation_margin_db = 2.0f;
    config.voice_detection.enabled = true;
    config.level_estimation.enabled = true;
    apm->ApplyConfig(config);

    webrtc::ProcessingConfig pconfig;
    pconfig.input_stream() = webrtc::StreamConfig(kSampleRate, kChannels);
    pconfig.output_stream() = webrtc::StreamConfig(kSampleRate, kChannels);
    pconfig.reverse_input_stream() = webrtc::StreamConfig(kSampleRate, kChannels);
    pconfig.reverse_output_stream() = webrtc::StreamConfig(kSampleRate, kChannels);
    apm->Initialize(pconfig);

    std::vector<float> delay_buf(kDelaySamples, 0.0f);
    size_t write_pos = 0;

    std::vector<float> render(kFrameSamples);
    std::vector<float> capture(kFrameSamples);
    std::vector<float> output(kFrameSamples);
    float* render_ptrs[] = { render.data() };
    float* capture_ptrs[] = { capture.data() };
    float* output_ptrs[] = { output.data() };

    printf("%-6s %-10s %-10s %-10s %-8s %-10s %-10s %-10s %-10s\n",
           "Frame", "Render", "Capture", "Out", "VAD", "ERLE_dB",
           "Level_dB", "Delay", "VoiceRMS");
    printf("-------------------------------------------------------------------------------\n");

    for (int frame = 0; frame < 80; ++frame) {
        FillSine(render.data(), kFrameSamples, 440.0f, 0.2f, frame);
        float render_rms = ComputeRms(render.data(), kFrameSamples);

        FillSpeechSim(capture.data(), kFrameSamples, frame);
        for (size_t i = 0; i < kFrameSamples; ++i) {
            size_t idx = (write_pos + i) % kDelaySamples;
            float delayed_render = delay_buf[idx];
            capture[i] = capture[i] + delayed_render * 0.6f;
        }
        float capture_rms = ComputeRms(capture.data(), kFrameSamples);

        for (size_t i = 0; i < kFrameSamples; ++i) {
            delay_buf[write_pos] = render[i];
            write_pos = (write_pos + 1) % kDelaySamples;
        }

        apm->ProcessReverseStream(
            const_cast<const float* const*>(render_ptrs),
            pconfig.reverse_input_stream(),
            pconfig.reverse_output_stream(),
            render_ptrs);

        apm->set_stream_delay_ms(kEchoDelayMs);

        apm->ProcessStream(
            const_cast<const float* const*>(capture_ptrs),
            pconfig.input_stream(),
            pconfig.output_stream(),
            output_ptrs);

        float out_rms = ComputeRms(output.data(), kFrameSamples);
        auto stats = apm->GetStatistics();
        bool vad = stats.voice_detected.value_or(false);
        double erle = stats.echo_return_loss_enhancement.value_or(0.0);
        int level_db = stats.output_rms_dbfs.value_or(-127);
        int delay = stats.delay_ms.value_or(-1);

        printf("%-6d %-10.4f %-10.4f %-10.4f %-8s %-10.1f %-10d %-10d %-10.4f\n",
               frame, render_rms, capture_rms, out_rms,
               vad ? "VOICE" : "SILENCE", erle, level_db, delay,
               vad ? out_rms : 0.0f);
    }

    printf("\n=== Done ===\n");
    return 0;
}