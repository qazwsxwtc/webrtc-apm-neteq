#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <string>
#include <vector>

#include "api/array_view.h"
#include "api/audio/audio_frame.h"
#include "api/audio_codecs/audio_codec_pair_id.h"
#include "api/audio_codecs/audio_decoder.h"
#include "api/audio_codecs/audio_decoder_factory.h"
#include "api/audio_codecs/audio_format.h"
#include "api/audio_codecs/isac/audio_decoder_isac_fix.h"
#include "api/neteq/neteq.h"
#include "api/rtp_headers.h"
#include "api/scoped_refptr.h"
#include "rtc_base/buffer.h"
#include "rtc_base/ref_count.h"
#include "rtc_base/ref_counted_object.h"
#include "system_wrappers/include/clock.h"

namespace {

constexpr int kPayloadType = 112;
constexpr int kSampleRateHz = 16000;
constexpr int kFrameSizeMs = 30;

struct IsacDecoderFactory final
    : public rtc::RefCountedObject<webrtc::AudioDecoderFactory> {
  IsacDecoderFactory() = default;
  std::vector<webrtc::AudioCodecSpec> GetSupportedDecoders() override {
    std::vector<webrtc::AudioCodecSpec> specs;
    webrtc::AudioDecoderIsacFix::AppendSupportedDecoders(&specs);
    return specs;
  }
  bool IsSupportedDecoder(const webrtc::SdpAudioFormat& format) override {
    return webrtc::AudioDecoderIsacFix::SdpToConfig(format).has_value();
  }
  std::unique_ptr<webrtc::AudioDecoder> MakeAudioDecoder(
      const webrtc::SdpAudioFormat& format,
      absl::optional<webrtc::AudioCodecPairId> codec_pair_id) override {
    auto cfg = webrtc::AudioDecoderIsacFix::SdpToConfig(format);
    if (!cfg.has_value())
      return nullptr;
    return webrtc::AudioDecoderIsacFix::MakeAudioDecoder(*cfg, codec_pair_id);
  }
};

struct EncodedPacket {
  webrtc::RTPHeader header;
  std::vector<uint8_t> payload;
};

const char* SpeechTypeName(webrtc::AudioFrame::SpeechType t) {
  switch (t) {
    case webrtc::AudioFrame::kNormalSpeech: return "Normal";
    case webrtc::AudioFrame::kPLC:          return "PLC";
    case webrtc::AudioFrame::kCNG:          return "CNG";
    case webrtc::AudioFrame::kPLCCNG:       return "PLCCNG";
    case webrtc::AudioFrame::kCodecPLC:     return "CodecPLC";
    case webrtc::AudioFrame::kUndefined:    return "Undefined";
  }
  return "?";
}

double ComputeEnergyDb(const int16_t* data, size_t samples) {
  if (samples == 0)
    return -99.0;
  double sum = 0.0;
  for (size_t i = 0; i < samples; ++i) {
    sum += static_cast<double>(data[i]) * static_cast<double>(data[i]);
  }
  double rms = std::sqrt(sum / static_cast<double>(samples));
  if (rms < 1.0)
    return -99.0;
  return 20.0 * std::log10(rms / 32767.0);
}

std::vector<EncodedPacket> GenerateFakePackets(int num_packets,
                                               int payload_size_bytes) {
  std::vector<EncodedPacket> packets;
  uint32_t timestamp = 0;
  for (int i = 0; i < num_packets; ++i) {
    EncodedPacket pkt;
    pkt.header.timestamp = timestamp;
    pkt.header.sequenceNumber = static_cast<uint16_t>(i);
    pkt.header.ssrc = 0x12345678;
    pkt.header.payloadType = kPayloadType;
    pkt.payload.resize(payload_size_bytes, 0);
    timestamp += kSampleRateHz / 1000 * kFrameSizeMs;
    packets.push_back(std::move(pkt));
  }
  return packets;
}

int RunNetEqDemo(bool simulate_loss) {
  printf("=== WebRTC NetEQ Basic Demo ===\n\n");
  fflush(stdout);

  rtc::scoped_refptr<IsacDecoderFactory> decoder_factory(
      new IsacDecoderFactory());

  webrtc::SdpAudioFormat isac_format("ISAC", 16000, 1);

  webrtc::NetEq::Config neteq_config;
  neteq_config.sample_rate_hz = kSampleRateHz;
  neteq_config.max_packets_in_buffer = 200;

  webrtc::SimulatedClock clock(0);

  std::unique_ptr<webrtc::NetEq> neteq(webrtc::NetEq::Create(
      neteq_config, &clock, decoder_factory));
  if (!neteq) {
    printf("ERROR: Failed to create NetEq\n");
    return 1;
  }

  if (!neteq->RegisterPayloadType(kPayloadType, isac_format)) {
    printf("ERROR: Failed to register payload type\n");
    return 1;
  }

  constexpr int kNumPackets = 60;
  constexpr int kPayloadSize = 100;

  printf("[1] Generating %d fake packets (%d bytes each)...\n",
         kNumPackets, kPayloadSize);
  fflush(stdout);

  auto all_packets = GenerateFakePackets(kNumPackets, kPayloadSize);
  printf("    Done. Seq 0..%u, timestamp step = %d\n",
         all_packets.back().header.sequenceNumber,
         kSampleRateHz / 1000 * kFrameSizeMs);
  fflush(stdout);

  printf("\n[2] Inserting into NetEq (simulate_loss=%s)...\n",
         simulate_loss ? "ON (25%)" : "OFF");
  fflush(stdout);

  int inserted = 0;
  int dropped = 0;
  for (auto& pkt : all_packets) {
    bool skip = simulate_loss && (pkt.header.sequenceNumber % 4 == 3);
    if (skip) {
      dropped++;
      continue;
    }
    int ret = neteq->InsertPacket(
        pkt.header,
        rtc::ArrayView<const uint8_t>(pkt.payload.data(), pkt.payload.size()));
    if (ret == 0)
      inserted++;
    else
      printf("    WARN: InsertPacket seq=%u returned %d\n",
             pkt.header.sequenceNumber, ret);
  }
  printf("    Inserted=%d, Dropped(simulated)=%d\n", inserted, dropped);
  fflush(stdout);

  printf("\n[3] Decoding from NetEq (10ms per GetAudio)...\n");
  fflush(stdout);

  int num_get = inserted * (kFrameSizeMs / 10) + 20;
  int normal_count = 0, plc_count = 0, cng_count = 0;
  int plccng_count = 0, codec_plc_count = 0;
  int muted_count = 0;
  int get_ok = 0;

  for (int i = 0; i < num_get; ++i) {
    clock.AdvanceTimeMilliseconds(10);

    webrtc::AudioFrame audio_frame;
    bool muted = false;
    int ret = neteq->GetAudio(&audio_frame, &muted);

    if (ret != webrtc::NetEq::kOK) {
      continue;
    }
    get_ok++;

    double energy_db = muted ? -99.0
                             : ComputeEnergyDb(audio_frame.data(),
                                               audio_frame.samples_per_channel_);
    const char* type_name = SpeechTypeName(audio_frame.speech_type_);

    switch (audio_frame.speech_type_) {
      case webrtc::AudioFrame::kNormalSpeech: normal_count++; break;
      case webrtc::AudioFrame::kPLC:          plc_count++; break;
      case webrtc::AudioFrame::kCNG:          cng_count++; break;
      case webrtc::AudioFrame::kPLCCNG:       plccng_count++; break;
      case webrtc::AudioFrame::kCodecPLC:     codec_plc_count++; break;
      default: break;
    }
    if (muted) muted_count++;

    auto ops = neteq->GetOperationsAndState();
    static uint64_t last_buf = 0xFFFFFFFFFFFFFFFFULL;
    bool show = (i == 0) || (i == num_get - 1) ||
                (audio_frame.speech_type_ != webrtc::AudioFrame::kNormalSpeech &&
                 audio_frame.speech_type_ != webrtc::AudioFrame::kPLCCNG) ||
                muted ||
                (ops.current_buffer_size_ms != last_buf);
    if (show) {
      printf("    f=%3d  type=%-10s  e=%6.1fdB  buf=%4llums  muted=%d\n",
             i, type_name, energy_db,
             (unsigned long long)ops.current_buffer_size_ms, muted ? 1 : 0);
      last_buf = ops.current_buffer_size_ms;
    }
    fflush(stdout);
  }

  printf("\n[4] Speech Type Counts (GetAudio OK=%d):\n", get_ok);
  printf("    NormalSpeech : %d\n", normal_count);
  printf("    PLC          : %d\n", plc_count);
  printf("    CNG          : %d\n", cng_count);
  printf("    PLCCNG       : %d\n", plccng_count);
  printf("    CodecPLC     : %d\n", codec_plc_count);
  printf("    Muted frames : %d\n", muted_count);
  fflush(stdout);

  auto lifetime = neteq->GetLifetimeStatistics();
  auto ops = neteq->GetOperationsAndState();

  printf("\n[5] NetEQ Lifetime Statistics:\n");
  printf("    total_samples_received         : %llu\n",
         (unsigned long long)lifetime.total_samples_received);
  printf("    concealed_samples              : %llu\n",
         (unsigned long long)lifetime.concealed_samples);
  printf("    concealment_events             : %llu\n",
         (unsigned long long)lifetime.concealment_events);
  printf("    jitter_buffer_delay_ms         : %llu\n",
         (unsigned long long)lifetime.jitter_buffer_delay_ms);
  printf("    packet_buffer_flushes          : %llu\n",
         (unsigned long long)ops.packet_buffer_flushes);
  printf("    preemptive_samples             : %llu\n",
         (unsigned long long)ops.preemptive_samples);
  printf("    accelerate_samples             : %llu\n",
         (unsigned long long)ops.accelerate_samples);
  printf("    current_buffer_size_ms         : %llu\n",
         (unsigned long long)ops.current_buffer_size_ms);
  printf("    interruption_count             : %d\n",
         lifetime.interruption_count);
  printf("    total_interruption_duration_ms : %d\n",
         lifetime.total_interruption_duration_ms);

  if (lifetime.total_samples_received > 0) {
    double conceal_rate =
        100.0 * static_cast<double>(lifetime.concealed_samples) /
        static_cast<double>(lifetime.total_samples_received);
    printf("\n[6] Concealment rate: %.2f%%\n", conceal_rate);
  }

  printf("\n=== Demo Complete ===\n");
  fflush(stdout);
  return 0;
}

}  // namespace

int main(int argc, char* argv[]) {
  bool simulate_loss = true;
  for (int i = 1; i < argc; ++i) {
    if (std::strcmp(argv[i], "--no-loss") == 0 ||
        std::strcmp(argv[i], "-n") == 0) {
      simulate_loss = false;
    } else if (std::strcmp(argv[i], "--help") == 0 ||
               std::strcmp(argv[i], "-h") == 0) {
      printf("Usage: %s [--no-loss|-n]\n\n", argv[0]);
      printf("Demonstrates WebRTC NetEQ JitterBuffer / PLC pipeline.\n");
      printf("  (uses manually constructed RTP packets with a dummy payload)\n");
      return 0;
    }
  }
  return RunNetEqDemo(simulate_loss);
}