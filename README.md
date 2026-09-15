> English | [中文](README.zh-CN.md)

# WebRTC APM + NetEQ — Stand‑Alone Audio Processing Library

An independent **Audio Processing Module (APM)** and **Network Equalizer (NetEQ)** extracted from the WebRTC source tree. Only audio‑related features are retained; video, network, ICE, media and other irrelevant modules are stripped out.

**Single CMake project · Zero external dependencies · Cross‑platform** compilation, split into **17 independent static libraries** whose names mirror the original WebRTC `rtc_library(...)` targets in `BUILD.gn`.

> **Source reference**: This project is a trimmed copy of the WebRTC native library. The original reference tree at `webrc ` is **read‑only** — nothing is ever deleted from it.

---

## Feature List

### APM (Audio Processing Module)

| Module | Description |
|--------|-------------|
| **HPF** | High‑pass filter for low‑frequency noise suppression (default 100 Hz) |
| **AEC3** | Third‑generation acoustic echo canceller (frequency‑domain adaptive filtering) |
| **AECM** | Light‑weight mobile‑oriented echo cancellation (Android / iOS native path) |
| **NS** | Noise suppression (combined time‑domain + frequency‑domain algorithms) |
| **NSX** | Deep‑neural‑network‑driven noise suppression powered by RNNoise |
| **AGC1** | Classic automatic gain control |
| **AGC2** | Loudness‑based second‑generation AGC |
| **VAD** | Voice activity detection (dual‑engine: GMM + deep neural network) |
| **HWA** | Proxy for hardware‑backed AGC / Echo / Suppression (optional) |

APM pipeline:

```
Capture → HPF → AEC3/AECM → NS/NSX → AGC1/AGC2 → VAD → Output
```

### NetEQ (Network Equalizer)

| Module | Description |
|--------|-------------|
| **Packet Buffer** | Reordering & jitter‑buffering of incoming RTP packets |
| **Delay Manager** | Adaptive target delay based on network conditions |
| **Expand / Preemptive Expand** | Packet loss concealment (PLC) — generates synthetic audio for lost packets |
| **Accelerate / Preempt** | Speed up audio to recover from buffer overshoot |
| **Time Stretch** | Waveform‑preserving time‑scale modification |
| **Comfort Noise** | CNG (Comfort Noise Generation) during silence / packet loss |
| **DTMF** | DTMF tone buffer & tone generator |
| **Statistics** | Buffer level, packet loss ratio, expand rate, etc. |

NetEQ decision flow:

```
RTP Packet → PacketBuffer → DecisionLogic → Normal / Expand / Accelerate / Merge → Audio Output
```

---

## Build Requirements

| Item | Minimum Version |
|------|-----------------|
| CMake | 3.16+ |
| C++ Compiler | C++17 capable (MSVC 2019+ / GCC 9+ / Clang 10+) |

**Zero external dependencies** — all source code (ISAC codec, PFFFT, RNNoise, jsoncpp, Abseil‑CPP subset) is built‑in.

---

## Quick Start

### Windows (VS2022)

```powershell
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release -- /m
```

> CMake 4.0 no longer writes `SolutionFolder` / `NestedProjects` into `.sln`. Two bundled post‑process scripts are invoked **automatically** after configure:
>
> - `cmake/fix_sln.ps1` — injects virtual solution folders into the `.sln`
> - `cmake/fix_filters.ps1` — generates hierarchical `.vcxproj.filters` so the Solution Explorer groups files by their actual source directory tree instead of dumping everything under flat `Source Files` / `Header Files`
>
> No manual step required.

### Linux / macOS

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(nproc)
```

### Android (NDK)

```bash
cmake -S . -B build-android \
  -DCMAKE_TOOLCHAIN_FILE=$NDK/build/cmake/android.toolchain.cmake \
  -DANDROID_ABI=arm64-v8a \
  -DANDROID_PLATFORM=android-24
cmake --build build-android -j
```

### iOS (Xcode)

```bash
cmake -S . -B build-ios \
  -G Xcode \
  -DCMAKE_OSX_SYSROOT=iphoneos \
  -DCMAKE_OSX_ARCHITECTURES=arm64
cmake --build build-ios --config Release
```

### Disable example programs

```bash
cmake -S . -B build -DAPM_BUILD_EXAMPLES=OFF
```

### Auto‑configured build definitions

| Definition | Value | Meaning |
|------------|-------|---------|
| `WEBRTC_APM_DEBUG_DUMP` | `0` / `1` | Enable APM debug dump files (off by default) |
| `WEBRTC_ARCH_X86_64` | auto | Set on x86_64 / AMD64 |
| `WEBRTC_USE_SSE2` | auto | Set on x86 / x86_64 |
| `WEBRTC_USE_NEON` | auto | Set on ARM / ARM64 |
| `RTC_DCHECK_IS_ON` | config‑based | `1` for Debug, `0` for Release |

---

## Build Artifacts

All static libraries. Names follow the original WebRTC `BUILD.gn` `rtc_library(...)` convention. Header files are also attached to each target so that Visual Studio Solution Explorer displays the full source tree.

### Core libraries (2 targets)

| Library | Windows Size (Release) | Description |
|---------|----------------------|-------------|
| **`webrtc_apm`** | 4.29 MB | Audio Processing Module + `api/audio` + `api/task_queue` + `api/units` |
| **`webrtc_neteq`** | 1.33 MB | Network Equalizer + `api/neteq` |

### Base libraries (3 targets)

| Library | Windows Size (Release) | Description |
|---------|----------------------|-------------|
| **`rtc_base`** | 2.43 MB | Thread / checks / logging / task_queue / numerics / strings / memory / experiments / sigslot / base64 |
| **`common_audio`** | 0.77 MB | Signal‑processing: resampler / SPL / VAD / ooura fft / spl_sqrt_floor |
| **`system_wrappers`** | 0.28 MB | clock / cpu_info / cpu_features / sleep / rtp_to_ntp_estimator / field_trial / metrics (WebRTC default impl, no Chromium dependency) |

### Third‑party libraries (4 targets)

| Library | Windows Size (Release) | Description |
|---------|----------------------|-------------|
| **`pffft`** | 0.06 MB | PFFFT single‑precision FFT (`third_party/pffft/src/`) |
| **`fft`** | 0.01 MB | `modules/third_party/fft/fft.c` — generic FFT used by ISAC |
| **`rnn_vad`** | 0.01 MB | RNNoise neural VAD weights (`third_party/rnnoise/src/rnn_vad_weights.cc`) |
| **`jsoncpp`** | 1.02 MB | jsoncpp reader / value / writer — used by `echo_canceller3_config_json.cc` |

### Codec libraries (8 targets)

| Library | Windows Size (Release) | Description |
|---------|----------------------|-------------|
| **`webrtc_cng`** | 0.03 MB | Comfort Noise Generator — NetEQ dependency |
| **`legacy_encoded_audio_frame`** | 0.06 MB | Legacy encoded audio frame container |
| **`isac_vad`** | 0.07 MB | ISAC VAD / pitch / filter functions (`deps: fft`) |
| **`isac_c`** | 0.31 MB | Wide‑band ISAC core codec (`deps: fft, isac_vad`) |
| **`isac_fix_common`** | 0.01 MB | ISAC fix‑rate common FFT / transform tables |
| **`isac_fix_c`** | 0.27 MB | Fix‑rate ISAC core codec (`deps: fft, isac_fix_common`) |
| **`isac`** | 0.07 MB | Wide‑band ISAC `AudioDecoderIsac` / `AudioEncoderIsac` (`deps: isac_c`) |
| **`isac_fix`** | 0.06 MB | Fix‑rate ISAC `AudioDecoderIsacFix` / `AudioEncoderIsacFix` (`deps: isac_fix_c`) |

### Dependency graph

```
               ┌──────────────┐
               │   rtc_base   │◀───────────────┐
               └──────────────┘                │
               ┌──────────────┐                │
               │ common_audio │◀───────────────┤
               └──────────────┘                │
               ┌──────────────┐                │
               │system_wrappers│◀──────────────┤ field_trial + metrics
               └──────────────┘                │ live here
                                               │
    ┌───────────────────────────────────────────┼─────────────────────────────────────────────┐
    │                                           │                                           │
    │   ┌─────── codec deps ───────┐            │   ┌─────── codec deps ───────┐             │
    │   │ isac ← isac_c ← isac_vad │            │   │ isac ← isac_c ← isac_vad │             │
    │   │ isac_fix ← isac_fix_c    │            │   │ isac_fix ← isac_fix_c    │             │
    │   │        ← isac_fix_common │            │   │        ← isac_fix_common │             │
    │   │ webrtc_cng               │            │   │ webrtc_cng               │             │
    │   │ legacy_encoded_audio_frame│            │   │ legacy_encoded_audio_frame│             │
    │   └──────────┬───────────────┘            │   └──────────┬───────────────┘             │
    │              │                             │              │                             │
    │   ┌─────── third_party ──────┐             │   ┌─────── third_party ──────┐             │
    │   │ pffft  fft  rnn_vad      │             │   │ pffft  fft              │             │
    │   │ jsoncpp                  │             │   └──────────┬──────────────┘             │
    │   └──────────┬───────────────┘             │              │                             │
    │              │                             │              │                             │
    │              ▼                             │              ▼                             │
    │        ┌──────────────┐                    │        ┌──────────────┐                    │
    │        │  webrtc_apm  │                    │        │ webrtc_neteq │                    │
    │        │ (AEC/NS/AGC  │                    │        │ (PLC/CNG/    │                    │
    │        │  /VAD/HPF)   │                    │        │  jitter buf) │                    │
    │        └──────────────┘                    │        └──────────────┘                    │
    │                                           │                                           │
    └───────────────────────────────────────────┴───────────────────────────────────────────┘
```

### Visual Studio solution layout

The `.sln` is organized into nested virtual folders so that the 17 libraries and 7 examples are grouped by role. Folders are injected by `cmake/fix_sln.ps1` (run automatically after CMake configure):

```
webrtc_apm_neteq.sln
├── webrtc/                              ← 2 top‑level libraries
│   │   webrtc_apm
│   │   webrtc_neteq
│   │
│   ├── base/                            ← 3 foundation libraries
│   │       rtc_base
│   │       common_audio
│   │       system_wrappers
│   │
│   ├── codecs/                          ← 8 codec libraries (ISAC family + CNG)
│   │       isac / isac_c / isac_vad / isac_fix / isac_fix_c / isac_fix_common
│   │       webrtc_cng
│   │       legacy_encoded_audio_frame
│   │
│   └── third_party/                     ← 4 third‑party libraries
│           pffft
│           fft
│           rnn_vad
│           jsoncpp
│
└── example/                             ← 7 example programs
    │   basic_apm.exe
    │   aec_demo.exe
    │   agc_demo.exe
    │   ns_demo.exe
    │   vad_demo.exe
    │   apm_pipeline.exe
    │   hpf_aec_ns_agc_vad.exe
```

The `ALL_BUILD` and `ZERO_CHECK` helper projects generated by CMake remain at the solution root (not moved into any folder).

### Visual Studio Solution Explorer hierarchy

Even with folders above, CMake still generates **flat** `.vcxproj.filters` — every `.cc` under `Source Files`, every `.h` under `Header Files`. `cmake/fix_filters.ps1` (also run automatically after configure) rewrites each `.vcxproj.filters` to mirror the actual source directory tree, so the Solution Explorer groups files like:

```
Source Files
  ├── api\audio
  ├── modules\audio_processing\aec3
  ├── modules\audio_processing\agc2
  ├── rtc_base\numerics
  ├── common_audio\resampler
  └── third_party\jsoncpp\source\src\lib_json

Header Files
  ├── api\audio
  ├── modules\audio_processing\aec3
  ├── rtc_base\synchronization
  └── ...
```

---

## Directory Layout

```
apm-neteq/
├── CMakeLists.txt                   # Sole build entry point
│
├── cmake/
│   ├── fix_sln.ps1                  # VS post‑process: injects SolutionFolder + NestedProjects
│   └── fix_filters.ps1              # VS post‑process: generates hierarchical .vcxproj.filters
│
├── modules/
│   ├── audio_processing/            # ★ APM core (compiled into webrtc_apm)
│   │   ├── include/                 #   Public API (audio_processing.h, config.h, ...)
│   │   ├── aec3/                    #   AEC3 core
│   │   ├── aecm/                    #   AECM core
│   │   ├── agc/                     #   AGC1
│   │   ├── agc2/                    #   AGC2
│   │   ├── ns/                      #   NS + NSX
│   │   ├── vad/                     #   VAD (GMM + RNN VAD)
│   │   ├── echo_detector/           #   AEC3 helper
│   │   ├── utility/                 #   Delay estimator, pffft wrapper, ...
│   │   ├── null_aec_dump_factory.cc #   AEC dump stub (aec_dump/ excluded)
│   │   └── logging/                 #   APM data dumper
│   │
│   └── audio_coding/
│       ├── neteq/                   # ★ NetEQ core (compiled into webrtc_neteq)
│       │   ├── neteq_impl.cc               ← main implementation
│       │   ├── decision_logic.cc            ← state machine
│       │   ├── packet_buffer.cc             ← jitter buffer
│       │   ├── delay_manager.cc             ← adaptive delay
│       │   ├── expand.cc / preemptive_expand.cc
│       │   ├── accelerate.cc / normal.cc / merge.cc
│       │   ├── comfort_noise.cc            ← CNG (webrtc_cng)
│       │   ├── dtmf_buffer.cc / dtmf_tone_generator.cc
│       │   ├── time_stretch.cc              ← time‑scale modification
│       │   ├── nack_tracker.cc              ← NACK feedback
│       │   ├── decoder_database.cc          ← decoder registry
│       │   └── statistics_calculator.cc
│       │
│       └── codecs/
│           ├── isac/                #   ISAC family (8 libraries)
│           │   ├── main/{source,include}/  → isac_vad, isac_c, isac
│           │   └── fix/{source,include}/  → isac_fix_common, isac_fix_c, isac_fix
│           ├── cng/                 #   webrtc_cng (comfort noise generation)
│           └── legacy_encoded_audio_frame.cc  → legacy_encoded_audio_frame
│
├── modules/third_party/fft/        # fft.c → fft library (ISAC dependency)
│
├── api/
│   ├── audio/                       # AudioFrame / EchoCanceller3Config / ...
│   ├── audio_codecs/                # AudioEncoder / AudioDecoder interfaces
│   ├── neteq/                       # NetEQ public API (neteq.h, neteq_factory.h, ...)
│   ├── task_queue/                  # TaskQueue interface (stdlib default on non‑Win / Apple)
│   ├── units/                       # Timestamp / TimeDelta / DataRate
│   ├── array_view.h                 # span‑like view
│   ├── audio_options.h
│   ├── function_view.h
│   ├── ref_counted_base.h
│   ├── scoped_refptr.h
│   └── rtp_headers.h / rtp_packet_info.h / rtp_packet_infos.h
│
├── rtc_base/                        # ★ rtc_base library
│   ├── system/                      # arch / file_wrapper / thread_registry (Android only)
│   ├── synchronization/             # mutex / rw_lock (win+posix) / critical_section / sequence_checker
│   ├── memory/                      # aligned_malloc / fifo_buffer
│   ├── numerics/                    # exp_filter / histogram / moving_average / sample_stats / ...
│   ├── strings/                     # string_builder / string_format / audio_format_to_string
│   ├── task_queue*.cc               # win / stdlib / gcd / libevent back‑ends
│   ├── time_utils.cc
│   ├── experiments/                 # Field trial parsing
│   ├── task_utils/                  # pending_task_safety_flag / repeating_task
│   └── third_party/                 # base64 / sigslot
│
├── common_audio/                    # ★ common_audio library
│   ├── signal_processing/           # SPL library (resample / fft / filter / ...)
│   ├── resampler/                   # PushResampler / SincResampler
│   ├── vad/                         # Standalone VAD
│   ├── third_party/                 # spl_sqrt_floor / ooura_fft (sse2 + generic)
│   └── ...
│
├── system_wrappers/                 # ★ system_wrappers library
│   ├── include/field_trial.h        #   Public header + WebRTC default implementation
│   ├── include/metrics.h            #   Public header + WebRTC default implementation
│   └── source/
│       ├── field_trial.cc           #   Key/value string parser (InitFieldTrialsFromString)
│       ├── metrics.cc               #   In‑process RtcHistogramMap (Enable / GetAndReset)
│       ├── clock.cc / cpu_info.cc / cpu_features.cc
│       └── rtp_to_ntp_estimator.cc / sleep.cc
│
├── sdk/android/native_api/stacktrace/   # StackTraceElement header (Android only)
│
├── third_party/
│   ├── abseil-cpp/                  # Abseil‑CPP subset (string_view, span, bind_front, ...)
│   ├── pffft/                       # pffft.c → pffft library
│   ├── rnnoise/                     # rnn_vad_weights.cc → rnn_vad library
│   └── jsoncpp/                     # json_reader/value/writer.cpp → jsoncpp library
│
└── example/                         # 7 demonstration programs
    ├── basic_apm.cc
    ├── apm_pipeline.cc
    ├── hpf_aec_ns_agc_vad.cc
    ├── aec_demo.cc
    ├── ns_demo.cc
    ├── agc_demo.cc
    └── vad_demo.cc
```

---

## Quick API Reference

### APM usage

```cpp
#include "modules/audio_processing/include/audio_processing.h"
#include "api/audio/audio_frame.h"

apm::AudioProcessingBuilder builder;
builder.SetCapturePostProcessing(std::make_unique<apm::EchoCanceller3Factory>());
auto apm = builder.Create();

apm::AudioProcessing::Config cfg;
cfg.echo_canceller.enabled = true;
cfg.noise_suppression.enabled = true;
cfg.gain_controller1.enabled = true;
cfg.high_pass_filter.enabled = true;
apm->ApplyConfig(cfg);

apm->Initialize(16000, 16000, 1, 1, apm::AudioProcessing::kFullDuplex);

// Per 10‑ms frame:
//   capture_frame — microphone input
//   render_frame  — far‑end reference (required for AEC)
apm->ProcessStream(capture_frame);
apm->ProcessReverseStream(render_frame);

auto stats = apm->GetStatistics();
// stats.echo_cancel.erl_db
// stats.noise_suppression.noise_level
// stats.voice_detected
```

### NetEQ usage

```cpp
#include "api/neteq/neteq.h"
#include "api/neteq/neteq_factory.h"

NetEq::Config cfg;
cfg.sample_rate_hz = 16000;
cfg.max_delay_ms = 200;
auto neteq = NetEqFactory::Create(cfg);

// Feed RTP packets
neteq->InsertPacket(rtp_timestamp, payload_type, pkt.data(), pkt.size(),
                    /*is_dtx=*/false, /*speaker_angle=*/0.0f);

// Get 10 ms of decoded audio
int16_t out[160];
NetEq::AudioFrameInfo info;
auto status = neteq->GetAudio(out, /*samples_per_channel=*/160,
                              /*num_channels=*/1, &info);

NetEq::NetworkStatistics stats;
neteq->GetNetworkStatistics(&stats);
```

### Field trial / metrics (optional)

```cpp
#include "system_wrappers/include/field_trial.h"
#include "system_wrappers/include/metrics.h"

// Enable in‑process metrics collection (call once before any histogram usage)
webrtc::metrics::Enable();

// Inject trial flags: "WebRTC-experimentFoo/Enabled/WebRTC-experimentBar/Enabled100kbps/"
webrtc::field_trial::InitFieldTrialsFromString(kMyTrials);

if (webrtc::field_trial::IsEnabled("WebRTC-experimentFoo")) {
  // ...
}

// Read accumulated histograms
std::map<std::string, std::unique_ptr<webrtc::metrics::SampleInfo>> hists;
webrtc::metrics::GetAndReset(&hists);
```

Full API documentation:
- APM: `modules/audio_processing/include/audio_processing.h`
- NetEQ: `api/neteq/neteq.h`
- Metrics / FieldTrial: `system_wrappers/include/metrics.h`, `system_wrappers/include/field_trial.h`

---

## Example Programs

Compiled binaries are placed under `build/Release/`. They take no command‑line arguments; each generates synthetic test audio and applies processing.

| Example | What it does |
|---------|-------------|
| `basic_apm` | HPF + NS, per‑frame InputRMS / OutputRMS / attenuation dB |
| `apm_pipeline` | Full HPF→AEC→NS→AGC→VAD pipeline |
| `hpf_aec_ns_agc_vad` | Pipeline + ERLE / noise‑reduction dB / VAD probability |
| `aec_demo` | ERLE (dB) — simulated echo‑cancellation measurement |
| `ns_demo` | Noise‑reduction amount (dB) |
| `agc_demo` | Input level vs output level comparison |
| `vad_demo` | Per‑frame speech probability (0.0 – 1.0) |

---

## Cross‑Platform Support

| Platform | Defines | Linked Libraries |
|----------|---------|------------------|
| **Windows** | `WEBRTC_WIN` + `NOMINMAX` | `winmm ws2_32` |
| **Linux** | `WEBRTC_LINUX` + `WEBRTC_POSIX` | `pthread` |
| **macOS** | `WEBRTC_APPLE` + `WEBRTC_OSX` + `WEBRTC_POSIX` | `pthread` + `CoreAudio` + `AudioToolbox` |
| **iOS** | `WEBRTC_APPLE` + `WEBRTC_IOS` + `WEBRTC_POSIX` | `pthread` + `CoreAudio` + `AudioToolbox` |
| **Android** | `WEBRTC_ANDROID` + `WEBRTC_POSIX` | `pthread log` |
| **Emscripten** | `WEBRTC_LINUX` + `WEBRTC_POSIX` | (none) |

Architecture auto‑detection:

- x86 / x86_64 → `WEBRTC_USE_SSE2=ON`, NEON sources filtered out
- ARM32 / ARM64 → `WEBRTC_USE_NEON=ON`, SSE sources filtered out
- MIPS → filtered out by default

Non‑target assembly sources (`*_mips*`, `*_neon*`, `*_sse*`) are excluded at CMake configure time.

---

## Source Tree & Filtering

### Source origin

The entire project is extracted from:

```
D:\newwebrtc\webrtc-checkout\src
```

**The reference tree is read‑only.** Nothing is ever deleted from it — this project is a standalone working copy with the same APM + NetEQ sources and a hand‑tuned CMakeLists.txt.

### Filtering rules (CMakeLists.txt)

| Rule | What gets excluded |
|------|--------------------|
| `unittest` regex | All unit test sources |
| `_test\.cc$` regex | All test binaries |
| `mips` regex | MIPS SIMD sources |
| `neon` regex | ARM NEON sources (non‑ARM builds) |
| `sse` regex | SSE sources (non‑x86 builds) |
| `/mock/` regex | Mock implementations |
| `/aec_dump/` regex | AEC dump subsystem (stub kept via `null_aec_dump_factory.cc`) |
| `field_trial.cc` / `metrics.cc` | Compiled from `system_wrappers/source/` (WebRTC default impl), **not** from `third_party/webrtc_overrides/` (Chromium bridge layer) |
| ISAC MIPS / Neutrino files | Excluded from `isac_fix_c` by explicit file list |
| Android guards | `thread_registry.cc` / `warn_current_thread_is_deadlocked.cc` only on Android |

### Why `field_trial` / `metrics` live in `system_wrappers`

WebRTC ships with two possible implementations:

| Location | Type | Dependencies |
|----------|------|-------------|
| `system_wrappers/source/field_trial.cc` | **WebRTC default** | none (plain key/value string parser) |
| `system_wrappers/source/metrics.cc` | **WebRTC default** | none (in‑process `RtcHistogramMap` with `std::map`) |
| `third_party/webrtc_overrides/field_trial.cc` | Chromium bridge | `base/metrics/field_trial.h` |
| `third_party/webrtc_overrides/metrics.cc` | Chromium bridge | `base/metrics/histogram.h` |

This project uses the **WebRTC default implementations** — no Chromium dependency required. The Chromium bridge files in `third_party/webrtc_overrides/` are present in the source tree but intentionally **not compiled**.

### Header files

Every `add_library` target also pulls in its matching `*.h` headers (via `file(GLOB ... *_HEADERS CONFIGURE_DEPENDS ...)`). This makes Visual Studio Solution Explorer show the complete physical directory tree — clickable, fully indexed by IntelliSense.

### Build target statistics

| Target | Source files | Output |
|--------|-------------|--------|
| `rtc_base` | ~63 .cc | `rtc_base.lib` |
| `common_audio` | ~30 .cc + .c | `common_audio.lib` |
| `system_wrappers` | ~7 .cc / .c (incl. `field_trial.cc` + `metrics.cc`) | `system_wrappers.lib` |
| `pffft` | 1 .c | `pffft.lib` |
| `fft` | 1 .c | `fft.lib` |
| `rnn_vad` | 1 .cc | `rnn_vad.lib` |
| `jsoncpp` | 3 .cpp | `jsoncpp.lib` |
| `webrtc_cng` | 1 .cc | `webrtc_cng.lib` |
| `legacy_encoded_audio_frame` | 1 .cc | `legacy_encoded_audio_frame.lib` |
| `isac_vad` | 4 .c | `isac_vad.lib` |
| `isac_c` | 22 .c | `isac_c.lib` |
| `isac_fix_common` | 2 .c | `isac_fix_common.lib` |
| `isac_fix_c` | 26 .c | `isac_fix_c.lib` |
| `isac` | 2 .cc | `isac.lib` |
| `isac_fix` | 2 .cc | `isac_fix.lib` |
| `webrtc_apm` | ~161 .cc | `webrtc_apm.lib` |
| `webrtc_neteq` | ~32 .cc | `webrtc_neteq.lib` |

---

## License

BSD 3‑Clause License, consistent with the original WebRTC code‑base.