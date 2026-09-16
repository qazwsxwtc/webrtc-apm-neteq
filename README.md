> English | [中文](README.zh-CN.md)

# WebRTC APM + NetEQ — Stand‑Alone Audio Processing Library

An independent **Audio Processing Module (APM)** and **Network Equalizer (NetEQ)** extracted from the WebRTC source tree. Only audio‑related features are retained; video, network, ICE, media and other irrelevant modules are stripped out.

**Single CMake project · Zero external dependencies · Cross‑platform** compilation, split into **17 independent static libraries** whose names mirror the original WebRTC `rtc_library(...)` targets in `BUILD.gn`.

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
| **Ninja** | **Recommended on Linux / Android** (gmake parallel output hides real errors) |

**Zero external dependencies** — all source code (ISAC codec, PFFFT, RNNoise, jsoncpp, Abseil‑CPP subset) is built‑in.

---

## Quick Start

### Windows (MSVC via Ninja — recommended)

```powershell
# Requires Visual Studio 2022 Build Tools (Desktop development with C++)
# In CI, ilammy/msvc-dev-cmd@v1 injects the MSVC environment automatically

cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
```

### Windows (Visual Studio generator — for IDE)

```powershell
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release -- /m
```

> CMake 4.0 no longer writes `SolutionFolder` / `NestedProjects` into `.sln`. Two bundled post‑process scripts are invoked **automatically** after configure:
>
> - `cmake/fix_sln.ps1` — injects virtual solution folders into the `.sln`
> - `cmake/fix_filters.ps1` — generates hierarchical `.vcxproj.filters`
>
> No manual step required.

### Linux (Ninja — strongly recommended)

```bash
sudo apt-get install -y build-essential ninja-build
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

> **Why Ninja on Linux?** GNU Make's recursive parallel output buries real errors under `gmake[2]: Leaving directory...` noise. Ninja surfaces the actual failing command and error message at the bottom.

### Linux (GNU Make — not recommended for diagnosing failures)

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(nproc)
```

### macOS

```bash
# Native arm64 or x86_64 (auto-detects host arch)
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j

# Cross-compile for specific arch on Apple Silicon runner
cmake -S . -B build-arm64 \
  -DCMAKE_OSX_ARCHITECTURES=arm64 \
  -DCMAKE_BUILD_TYPE=Release
cmake --build build-arm64 -j
```

### Android (NDK)

```bash
export ANDROID_NDK_HOME=/path/to/android-ndk-r27b
cmake -S . -B build-android \
  -G Ninja \
  -DCMAKE_TOOLCHAIN_FILE="$ANDROID_NDK_HOME/build/cmake/android.toolchain.cmake" \
  -DANDROID_ABI=arm64-v8a \
  -DANDROID_PLATFORM=android-24
cmake --build build-android
```

### iOS (Xcode)

```bash
cmake -S . -B build-ios \
  -G Xcode \
  -DCMAKE_SYSTEM_NAME=iOS \
  -DCMAKE_OSX_SYSROOT=iphoneos \
  -DCMAKE_OSX_ARCHITECTURES=arm64 \
  -DCMAKE_BUILD_TYPE=Release
cmake --build build-ios --config Release -- -sdk iphoneos
```

### Disable example programs

```bash
cmake -S . -B build -DAPM_BUILD_EXAMPLES=OFF
```

### Build options

| Option | Default | Description |
|--------|---------|-------------|
| `APM_BUILD_EXAMPLES` | `ON` | Build 8 demo programs under `example/` |
| `APM_BUILD_SHARED` | `OFF` | Build `.so` / `.dylib` / `.dll` instead of static `.a` / `.lib` |
| `APM_FORCE_STATIC_CRT` | `OFF` | Link `/MT` instead of `/MD` on MSVC |

### Auto‑configured definitions

| Definition | Value | Meaning |
|------------|-------|---------|
| `WEBRTC_APM_DEBUG_DUMP` | `0` / `1` | Enable APM debug dump files (off by default) |
| `WEBRTC_ARCH_X86_64` | auto | Set on x86_64 / AMD64 |
| `WEBRTC_ARCH_X86` | auto | Set on x86 / i386 |
| `WEBRTC_ARCH_ARM64` | auto | Set on aarch64 / arm64 |
| `WEBRTC_ARCH_ARM` | auto | Set on ARM32 |
| `WEBRTC_USE_SSE2` | auto | `ON` on x86 / x86_64 |
| `WEBRTC_USE_NEON` | auto | `ON` on ARM / ARM64 |
| `WEBRTC_HAS_SSE2` | auto | Compile definition when SSE2 sources are enabled |
| `WEBRTC_HAS_NEON` | auto | Compile definition when NEON sources are enabled |
| `WEBRTC_ARCH_X86_FAMILY` | auto | Set together with `WEBRTC_ARCH_X86` / `WEBRTC_ARCH_X86_64` |
| `WEBRTC_ARCH_ARM_FAMILY` | auto | Set together with `WEBRTC_ARCH_ARM` / `WEBRTC_ARCH_ARM64` |
| `RTC_DCHECK_IS_ON` | config‑based | `1` for Debug, `0` for Release |

---

## Build Artifacts

All static libraries. Names follow the original WebRTC `BUILD.gn` `rtc_library(...)` convention. Header files are also attached to each target so that Visual Studio Solution Explorer displays the full source tree.

### Core libraries (2 targets)

| Library | Description |
|---------|-------------|
| **`webrtc_apm`** | Audio Processing Module + `api/audio` + `api/task_queue` + `api/units` |
| **`webrtc_neteq`** | Network Equalizer + `api/neteq` |

### Base libraries (3 targets)

| Library | Description |
|---------|-------------|
| **`rtc_base`** | Thread / checks / logging / task_queue / numerics / strings / memory / experiments / sigslot / base64 + **abseil strings** (`match.cc`, `memutil.cc`, `ascii.cc`) |
| **`common_audio`** | Signal‑processing: resampler / SPL / VAD / ooura fft / spl_sqrt_floor |
| **`system_wrappers`** | clock / cpu_info / cpu_features / sleep / rtp_to_ntp_estimator / field_trial / metrics |

### Third‑party libraries (4 targets)

| Library | Description |
|---------|-------------|
| **`pffft`** | PFFFT single‑precision FFT (`third_party/pffft/src/`) |
| **`fft`** | `modules/third_party/fft/fft.c` — generic FFT used by ISAC |
| **`rnn_vad`** | RNNoise neural VAD weights (`third_party/rnnoise/src/rnn_vad_weights.cc`) |
| **`jsoncpp`** | jsoncpp reader / value / writer — used by `echo_canceller3_config_json.cc` |

### Codec libraries (8 targets)

| Library | Description |
|---------|-------------|
| **`webrtc_cng`** | Comfort Noise Generator — NetEQ dependency |
| **`legacy_encoded_audio_frame`** | Legacy encoded audio frame container |
| **`isac_vad`** | ISAC VAD / pitch / filter functions (`deps: fft`) |
| **`isac_c`** | Wide‑band ISAC core codec (`deps: fft, isac_vad`) |
| **`isac_fix_common`** | ISAC fix‑rate common FFT / transform tables (+ NEON `transform_neon.c` on ARM) |
| **`isac_fix_c`** | Fix‑rate ISAC core codec (`deps: fft, isac_fix_common`, + NEON sources on ARM) |
| **`isac`** | Wide‑band ISAC `AudioDecoderIsac` / `AudioEncoderIsac` (`deps: isac_c`) |
| **`isac_fix`** | Fix‑rate ISAC `AudioDecoderIsacFix` / `AudioEncoderIsacFix` (`deps: isac_fix_c`) |

---

## Cross‑Platform Support

| Platform | Defines | Required Link Libraries |
|----------|---------|--------------------------|
| **Windows** | `WEBRTC_WIN` + `NOMINMAX` | `winmm ws2_32` |
| **Linux** | `WEBRTC_LINUX` + `WEBRTC_POSIX` | `pthread dl resolv rt m` |
| **macOS** | `WEBRTC_APPLE` + `WEBRTC_OSX` + `WEBRTC_MAC` + `WEBRTC_POSIX` | `pthread CoreFoundation CoreAudio AudioToolbox` |
| **iOS** | `WEBRTC_APPLE` + `WEBRTC_IOS` + `WEBRTC_MAC` + `WEBRTC_POSIX` | `pthread CoreFoundation CoreAudio AudioToolbox` |
| **Android** | `WEBRTC_ANDROID` + `WEBRTC_POSIX` | `log` |

> **Note on static library linking**: Static library `PRIVATE` dependencies do **not** propagate to final executables. Example programs (`basic_apm`, `neteq_basic`, ...) explicitly link all platform system libraries listed above. When integrating these libraries into your own project, you must also link the same system libraries.

### Architecture auto‑detection

| Architecture | `WEBRTC_USE_NEON` | `WEBRTC_USE_SSE2` | Notes |
|--------------|-------------------|-------------------|-------|
| x86 / x86_64 | OFF | ON | NEON sources filtered out, SSE sources compiled |
| ARM32 / ARM64 | ON | OFF | SSE sources filtered out, NEON sources compiled |
| MIPS | OFF | OFF | All SIMD sources filtered out |

### Apple platform note

On Apple Silicon runners (`macos-latest`), `CMAKE_SYSTEM_PROCESSOR` always reports `arm64` — even when cross‑compiling for x86_64 via `-DCMAKE_OSX_ARCHITECTURES=x86_64`. The build system uses `CMAKE_OSX_ARCHITECTURES` first (when set and on Apple), falling back to `CMAKE_SYSTEM_PROCESSOR` on other platforms. This prevents x86_64 targets from accidentally including NEON sources.

### Compiler flag note (Apple Clang / NDK Clang)

GCC‑style instruction‑set flags (`-mfpu=neon`, `-march=armv8-a`, `-msse2`) are **not supported** by Apple Clang or Android NDK Clang. These compilers enable the appropriate ISA based on the target triple and the source file's own `#include <arm_neon.h>` / `<emmintrin.h>`. The build system adds these flags **only** for Linux GCC/Clang on ARM targets.

---

## CI / CD

GitHub Actions workflow (`.github/workflows/build.yml`) runs on every push / PR to `main`:

| Job | Runner | Config | Status |
|-----|--------|--------|--------|
| Windows | `windows-latest` | Ninja + MSVC (Debug, Release) | ✅ Working |
| Linux | `ubuntu-latest` | Ninja + GCC / Clang (Release) | ✅ Working |
| macOS | `macos-latest` | Apple Clang (x86_64, arm64) | ✅ Working |
| Android | `ubuntu-latest` | NDK r27b + Ninja (armeabi-v7a, arm64-v8a, x86_64) | ✅ Working |
| iOS | `macos-latest` | Xcode (iphoneos, iphonesimulator) | ✅ Working |

Artifacts are uploaded for each platform's primary target (Windows Release, Linux GCC, macOS arm64, Android arm64-v8a, iOS iphoneos).

---

## Platform‑Specific Build Notes

### Windows

- Use **Ninja + MSVC** as the primary build generator. The `windows-latest` runner in GitHub Actions does **not** have Visual Studio 2022 installed by default — it only comes with Build Tools. Use `ilammy/msvc-dev-cmd@v1` to inject the MSVC compiler environment.
- The "Visual Studio 17 2022" generator is only recommended when you have the full Visual Studio IDE installed locally.
- Required system libraries: `winmm` (waveOut / waveIn), `ws2_32` (Winsock).

### Linux

Linux requires **five** system libraries for a complete link:

| Library | Symbols provided |
|---------|-----------------|
| `-lpthread` | Threads, mutexes, condition variables (`pthread_*`) |
| `-ldl` | Dynamic loading (`dlopen`, `dlsym`) |
| `-lresolv` | DNS resolution (`getaddrinfo`, `getnameinfo`) |
| `-lrt` | Realtime extensions (`clock_gettime`, `shm_open`) |
| `-lm` | **Math functions** (`sin`, `cos`, `sqrt`, `pow`, `exp`, `log`) — standalone on Linux, auto‑included on all other platforms |

> Common mistake: forgetting `-lm`. On Linux, math functions live in a separate library, unlike Windows/macOS where they're in the C runtime. This causes `undefined reference to cos` / `sin` / `sqrt` linker errors.

Linux is the **only** platform where GCC‑style ISA flags (`-mfpu=neon`, `-march=armv8-a`) are added — both the system GCC and Clang toolchains accept them.

### Android

- Uses `cpu_features_linux.c` instead of the original `cpu_features_android.c`, because NDK r24+ removed `<cpu-features.h>`. The Linux implementation works on Android via `getauxval` and `/proc/self/auxv`.
- `WEBRTC_ANDROID` also implies `WEBRTC_LINUX` in the `platform_thread_types.cc` include guard, because Android's Bionic libc provides the Linux `prctl()` and `gettid()` syscalls. The source has been patched to extend the guard from `WEBRTC_LINUX` to `WEBRTC_LINUX || WEBRTC_ANDROID`, plus `#define _GNU_SOURCE` to unlock `PR_SET_NAME`.
- ISAC ISA flags (`-mfpu=neon`) are **not** added — NDK Clang enables NEON based on the target ABI (`arm64-v8a` always has NEON, `armeabi-v7a` has it by default since NDK r17).
- Required system library: `-llog` (android logging).

### macOS / iOS

- **`CoreFoundation`** framework is required for `rtc_base/logging.cc` (CFBundle, CFPreferences).
- NEON optimization (`WEBRTC_HAS_NEON`) is enabled on arm64; SSE2 (`WEBRTC_USE_SSE2`) on x86_64.
- Apple Clang rejects `-mfpu=neon`, `-march=armv8-a`, `-msse2` — ISA selection is automatic via the target triple. The build system skips these flags for all Apple targets.
- The source has been patched to add `WEBRTC_MAC` compile definition — some headers (e.g., `byte_order.h`) check this macro to decide between `endian.h` (Linux/Android) and `libkern/OSByteOrder.h` (Apple).

### ISAC NEON sources

The ISAC fix codec's NEON implementations (`*_neon.c`) are conditionally compiled into `isac_fix_common` and `isac_fix_c` only when `WEBRTC_USE_NEON=ON`. These provide the function‑pointer targets that `isacfix.c` selects at runtime when `WEBRTC_HAS_NEON` is defined.

| Source file | Provides | Linked into |
|-------------|----------|-------------|
| `transform_neon.c` | `WebRtcIsacfix_Spec2TimeNeon`, `WebRtcIsacfix_Time2SpecNeon` | `isac_fix_common` |
| `entropy_coding_neon.c` | `WebRtcIsacfix_MatrixProduct1Neon`, `WebRtcIsacfix_MatrixProduct2Neon` | `isac_fix_c` |
| `filterbanks_neon.c` | `WebRtcIsacfix_AllpassFilter2FixDec16Neon` | `isac_fix_c` |
| `filters_neon.c` | `WebRtcIsacfix_AutocorrNeon` | `isac_fix_c` |
| `lattice_neon.c` | `WebRtcIsacfix_FilterMaLoopNeon` | `isac_fix_c` |

Similarly, `common_audio/third_party/ooura/fft_size_128/ooura_fft_neon.cc` is compiled on ARM targets, providing `webrtc::cft1st_128_neon(float*)` etc. used by `real_fourier_ooura.cc`.

### Common pitfalls across platforms

| Error | Root cause | Fix |
|-------|-----------|-----|
| `Visual Studio 17 2022 could not find any instance of Visual Studio` | `windows-latest` runner has Build Tools only | Use Ninja generator + `ilammy/msvc-dev-cmd` |
| `unsupported option '-mfpu=' for target 'arm64-apple-darwin'` | Apple Clang doesn't support GCC‑style ISA flags | Skip these flags for Apple targets |
| `unsupported option '-mfpu=' for target 'aarch64-none-linux-android'` | NDK Clang also rejects `-mfpu=` | Skip these flags for Android targets |
| `<cpu-features.h> file not found` | NDK r24+ removed this header | Use `cpu_features_linux.c` instead |
| `Undefined symbols: _WebRtcIsacfix_AllpassFilter2FixDec16Neon` | ISAC NEON source files missing from build | Add `transform_neon.c` etc. conditionally |
| `<arm_neon.h> is intended only for ARM targets` | Apple Silicon runner's `SYSTEM_PROCESSOR` reports `arm64` even for x86_64 cross‑compile | Detect arch via `CMAKE_OSX_ARCHITECTURES` on Apple |
| `fatal error: 'endian.h' file not found` | `byte_order.h` needs `WEBRTC_MAC` to pick Apple's header | Add `WEBRTC_APPLE WEBRTC_MAC` to Apple compile defs |
| `PR_SET_NAME undeclared` | `platform_thread_types.cc` guard only covered Linux | Extend to `WEBRTC_LINUX || WEBRTC_ANDROID` + `_GNU_SOURCE` |
| `undefined reference to sin / cos / sqrt` | Linux doesn't auto‑link `-lm` | Add `-lm` to the link line |
| `xcodebuild: error: option '-configuration' may only be provided once` | CMake Xcode generator passes `-configuration` internally, don't pass it again via `--` args | Use `-- -sdk iphoneos` only |

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

Compiled binaries are placed under `build/Release/` (or `build/<config>/Debug/` for VS). They take no command‑line arguments; each generates synthetic test audio and applies processing.

| Example | What it does |
|---------|-------------|
| `basic_apm` | HPF + NS, per‑frame InputRMS / OutputRMS / attenuation dB |
| `apm_pipeline` | Full HPF→AEC→NS→AGC→VAD pipeline |
| `hpf_aec_ns_agc_vad` | Pipeline + ERLE / noise‑reduction dB / VAD probability |
| `aec_demo` | ERLE (dB) — simulated echo‑cancellation measurement |
| `ns_demo` | Noise‑reduction amount (dB) |
| `agc_demo` | Input level vs output level comparison |
| `vad_demo` | Per‑frame speech probability (0.0 – 1.0) |
| `neteq_basic` | NetEQ basic usage — packet insertion + audio extraction |

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
│   ├── strings/                     # string_builder / string_format / audio_format_to_string / json
│   ├── task_queue*.cc               # win / stdlib / gcd / libevent back‑ends
│   ├── time_utils.cc
│   ├── ip_address.cc / socket_address.cc / net_helpers.cc / async_socket.cc
│   ├── experiments/                 # Field trial parsing
│   ├── task_utils/                  # pending_task_safety_flag / repeating_task
│   └── third_party/                 # base64 / sigslot
│
├── common_audio/                    # ★ common_audio library
│   ├── signal_processing/           # SPL library (resample / fft / filter / NEON variants)
│   ├── resampler/                   # PushResampler / SincResampler (+ NEON / SSE2)
│   ├── vad/                         # Standalone VAD
│   ├── third_party/                 # spl_sqrt_floor / ooura_fft (NEON + SSE2 + generic)
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
├── example/                         # 8 demonstration programs
│   ├── basic_apm.cc
│   ├── apm_pipeline.cc
│   ├── hpf_aec_ns_agc_vad.cc
│   ├── aec_demo.cc
│   ├── ns_demo.cc
│   ├── agc_demo.cc
│   ├── vad_demo.cc
│   └── neteq_basic.cc
│
└── .github/workflows/build.yml      # CI configuration (5 platforms)
```

---

## Source Tree & Filtering

### Source origin

The entire project is extracted from the WebRTC native library source tree. This project is a standalone working copy with the same APM + NetEQ sources and a hand‑tuned `CMakeLists.txt`.

### Filtering rules (CMakeLists.txt)

| Rule | What gets excluded |
|------|--------------------|
| `unittest` regex | All unit test sources |
| `_test\.cc$` regex | All test binaries |
| `mips` regex | MIPS SIMD sources |
| `neon` regex | ARM NEON sources (filtered out on non‑ARM builds via `if(NOT WEBRTC_USE_NEON)`) |
| `sse` regex | SSE sources (filtered out on non‑x86 builds via `if(NOT WEBRTC_USE_SSE2)`) |
| `/mock/` regex | Mock implementations |
| `/aec_dump/` regex | AEC dump subsystem (stub kept via `null_aec_dump_factory.cc`) |
| ISAC NEON sources | **Conditionally added** — `*_neon.c` files are compiled into `isac_fix_common` / `isac_fix_c` only when `WEBRTC_USE_NEON=ON` |
| Ooura FFT NEON / SSE2 | `ooura_fft_neon.cc` compiled on ARM, `ooura_fft_sse2.cc` compiled on x86 |
| **SIMD enabled by default** | SSE2 sources (`fir_filter_sse.cc`, `sinc_resampler_sse.cc`, `ooura_fft_sse2.cc`) auto‑compiled on x86; NEON sources (`cross_correlation_neon.c`, `aecm_core_neon.cc`, ISAC `*_neon.c`) auto‑compiled on ARM |
| **abseil strings** | `third_party/abseil-cpp/absl/strings/match.cc` + `internal/memutil.cc` + `ascii.cc` compiled into `rtc_base` — provides `absl::EqualsIgnoreCase`, `StartsWithIgnoreCase`, etc. (no stub, fully original) |

### Why `field_trial` / `metrics` live in `system_wrappers`

WebRTC ships with two possible implementations:

| Location | Type | Dependencies |
|----------|------|-------------|
| `system_wrappers/source/field_trial.cc` | **WebRTC default** | none (plain key/value string parser) |
| `system_wrappers/source/metrics.cc` | **WebRTC default** | none (in‑process `RtcHistogramMap` with `std::map`) |
| `third_party/webrtc_overrides/field_trial.cc` | Chromium bridge | `base/metrics/field_trial.h` |
| `third_party/webrtc_overrides/metrics.cc` | Chromium bridge | `base/metrics/histogram.h` |

This project uses the **WebRTC default implementations** — no Chromium dependency required. The Chromium bridge files in `third_party/webrtc_overrides/` are present in the source tree but intentionally **not compiled**.

---

## License

BSD 3‑Clause License, consistent with the original WebRTC code‑base.