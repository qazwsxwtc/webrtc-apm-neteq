> [English](README.md) | 中文

# WebRTC APM + NetEQ — 独立音频处理库

从 WebRTC 源码树中提取的独立 **Audio Processing Module (APM)** 和 **Network Equalizer (NetEQ)**。仅保留音频相关功能，剔除视频、网络、ICE、媒体等无关模块。

**单 CMake 工程 · 零外部依赖 · 跨平台**编译，参考原始 WebRTC `BUILD.gn` 中的 `rtc_library(...)` 命名拆分为 **17 个独立静态库**。

> **源码来源说明**：本工程是 WebRTC native library 的精简拷贝；参考源码 webrtc官网源码 **只读、永不修改**——仅做源码挖掘和复制。

---

## 功能特性

### APM（音频处理模块）

| 模块 | 说明 |
|------|------|
| **HPF** | 高通滤波器，抑制低频噪声（默认 100 Hz） |
| **AEC3** | 第三代回声消除器（基于频域自适应滤波） |
| **AECM** | 移动端回声消除（轻量级，Android / iOS 原生路径） |
| **NS** | 噪声抑制（时域 + 频域融合算法） |
| **NSX** | RNNoise 驱动的深度神经网络噪声抑制 |
| **AGC1** | 传统自动增益控制 |
| **AGC2** | 基于响度的第二代 AGC |
| **VAD** | 语音活动检测（GMM + 深度网络双引擎） |
| **HWA** | 硬件 AGC/Echo/Suppression 代理（可选） |

APM 处理流水线：

```
Capture → HPF → AEC3/AECM → NS/NSX → AGC1/AGC2 → VAD → Output
```

### NetEQ（网络均衡器）

| 模块 | 说明 |
|------|------|
| **Packet Buffer** | RTP 包重排序 + 抖动缓冲 |
| **Delay Manager** | 根据网络状况自适应调整目标延迟 |
| **Expand / Preemptive Expand** | 丢包隐藏（PLC）——为丢失的包生成合成音频 |
| **Accelerate / Preempt** | 加速音频，从缓冲溢出中恢复 |
| **Time Stretch** | 保持波形的时间拉伸/压缩 |
| **Comfort Noise** | 静音/丢包期间的 CNG（舒适噪声生成） |
| **DTMF** | DTMF 音调缓冲与生成 |
| **Statistics** | 缓冲等级、丢包率、expand 率等统计 |

NetEQ 决策流程：

```
RTP Packet → PacketBuffer → DecisionLogic → Normal / Expand / Accelerate / Merge → Audio Output
```

---

## 构建要求

| 项目 | 最低版本 |
|------|---------|
| CMake | 3.16+ |
| C++ 编译器 | 支持 C++17（MSVC 2019+ / GCC 9+ / Clang 10+） |

**零外部依赖**——所有源码（含 ISAC 编解码器、PFFFT、RNNoise、jsoncpp、Abseil‑CPP 子集）均已内置。

---

## 快速开始

### Windows (VS2022)

```powershell
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release -- /m
```

> CMake 4.0 不再向 `.sln` 写入 `SolutionFolder` / `NestedProjects`。两个内置后处理脚本会在 CMake 配置完成后被**自动调用**：
>
> - `cmake/fix_sln.ps1` — 向 `.sln` 注入虚拟解决方案文件夹
> - `cmake/fix_filters.ps1` — 生成带层级的 `.vcxproj.filters`，让 Solution Explorer 按真实源码目录结构分组文件，而不是全部堆在扁平的 `Source Files` / `Header Files` 下
>
> 无需手动执行。

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

### 关闭示例程序

```bash
cmake -S . -B build -DAPM_BUILD_EXAMPLES=OFF
```

### CMake 自动定义的宏

| 宏 | 值 | 含义 |
|------|----|------|
| `WEBRTC_APM_DEBUG_DUMP` | `0` / `1` | 开启 APM debug dump 文件（默认关闭） |
| `WEBRTC_ARCH_X86_64` | 自动 | x86_64 / AMD64 平台自动设置 |
| `WEBRTC_USE_SSE2` | 自动 | x86 / x86_64 平台自动开启 |
| `WEBRTC_USE_NEON` | 自动 | ARM / ARM64 平台自动开启 |
| `RTC_DCHECK_IS_ON` | 按配置 | Debug=`1`，Release=`0` |

---

## 构建产物

全部静态库，命名参考原始 WebRTC `BUILD.gn` 中的 `rtc_library(...)` target。
每个 target 同时收集对应 `.h` 头文件，Visual Studio 中可看到完整的物理目录树。

### 顶层库（2 个）

| 库 | Windows 大小 (Release) | 说明 |
|----|----------------------|------|
| **`webrtc_apm`** | 4.29 MB | 音频处理模块 + `api/audio` + `api/task_queue` + `api/units` |
| **`webrtc_neteq`** | 1.33 MB | 网络均衡器 + `api/neteq` |

### 基础库（3 个）

| 库 | Windows 大小 (Release) | 说明 |
|----|----------------------|------|
| **`rtc_base`** | 2.43 MB | Thread / checks / logging / task_queue / numerics / strings / memory / experiments / sigslot / base64 |
| **`common_audio`** | 0.77 MB | 信号处理：resampler / SPL / VAD / ooura fft / spl_sqrt_floor |
| **`system_wrappers`** | 0.28 MB | clock / cpu_info / cpu_features / sleep / rtp_to_ntp_estimator / **field_trial / metrics**（WebRTC 官方默认实现，零 Chromium 依赖） |

### 第三方库（4 个）

| 库 | Windows 大小 (Release) | 说明 |
|----|----------------------|------|
| **`pffft`** | 0.06 MB | PFFFT 单精度 FFT（`third_party/pffft/src/`） |
| **`fft`** | 0.01 MB | `modules/third_party/fft/fft.c`——ISAC 共用的通用 FFT |
| **`rnn_vad`** | 0.01 MB | RNNoise 神经网络 VAD 权重（`third_party/rnnoise/src/rnn_vad_weights.cc`） |
| **`jsoncpp`** | 1.02 MB | jsoncpp reader / value / writer——`echo_canceller3_config_json.cc` 必需 |

### 编解码器库（8 个）

| 库 | Windows 大小 (Release) | 说明 |
|----|----------------------|------|
| **`webrtc_cng`** | 0.03 MB | 舒适噪声生成器——NetEQ 依赖 |
| **`legacy_encoded_audio_frame`** | 0.06 MB | 遗留编码音频帧容器 |
| **`isac_vad`** | 0.07 MB | ISAC VAD / pitch / filter（`deps: fft`） |
| **`isac_c`** | 0.31 MB | 宽带 ISAC 核心算法（`deps: fft, isac_vad`） |
| **`isac_fix_common`** | 0.01 MB | 固定码率 ISAC 公共 FFT / 变换表 |
| **`isac_fix_c`** | 0.27 MB | 固定码率 ISAC 核心算法（`deps: fft, isac_fix_common`） |
| **`isac`** | 0.07 MB | 宽带 ISAC `AudioDecoderIsac` / `AudioEncoderIsac`（`deps: isac_c`） |
| **`isac_fix`** | 0.06 MB | 固定码率 ISAC `AudioDecoderIsacFix` / `AudioEncoderIsacFix`（`deps: isac_fix_c`） |

### 依赖关系

```
               ┌──────────────┐
               │   rtc_base   │◀───────────────┐
               └──────────────┘                │
               ┌──────────────┐                │
               │ common_audio │◀───────────────┤
               └──────────────┘                │
               ┌──────────────┐                │
               │system_wrappers│◀──────────────┤  field_trial + metrics
               └──────────────┘                │  就在这里
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

### Visual Studio 解决方案分组

`.sln` 通过虚拟文件夹嵌套展示，17 个静态库和 7 个示例按功能角色分组。文件夹由 `cmake/fix_sln.ps1` 注入（CMake 配置完成后自动运行）：

```
webrtc_apm_neteq.sln
├── webrtc/                              ← 2 个顶层库
│   │   webrtc_apm
│   │   webrtc_neteq
│   │
│   ├── base/                            ← 3 个基础库
│   │       rtc_base
│   │       common_audio
│   │       system_wrappers
│   │
│   ├── codecs/                          ← 8 个编解码库（ISAC 家族 + CNG）
│   │       isac / isac_c / isac_vad / isac_fix / isac_fix_c / isac_fix_common
│   │       webrtc_cng
│   │       legacy_encoded_audio_frame
│   │
│   └── third_party/                     ← 4 个第三方库
│           pffft
│           fft
│           rnn_vad
│           jsoncpp
│
└── example/                             ← 7 个示例程序
    │   basic_apm.exe
    │   aec_demo.exe
    │   agc_demo.exe
    │   ns_demo.exe
    │   vad_demo.exe
    │   apm_pipeline.exe
    │   hpf_aec_ns_agc_vad.exe
```

CMake 生成的辅助工程 `ALL_BUILD` 和 `ZERO_CHECK` 保持在解决方案根目录，不移动到任何文件夹。

### Visual Studio Solution Explorer 层级

上面的 `.sln` 文件夹只解决了**工程分组**问题，每个工程内部 CMake 生成的 `.vcxproj.filters` 仍然是**扁平的**——所有 `.cc` 堆在 `Source Files` 下，所有 `.h` 堆在 `Header Files` 下。`cmake/fix_filters.ps1`（也是 CMake 配置后自动运行）会重写每个 `.vcxproj.filters`，让 Solution Explorer 按真实源码目录结构分组：

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

## 目录结构

```
apm-neteq/
├── CMakeLists.txt                   # 唯一构建入口
│
├── cmake/
│   ├── fix_sln.ps1                  # VS 后处理：注入 SolutionFolder + NestedProjects
│   └── fix_filters.ps1              # VS 后处理：生成带层级的 .vcxproj.filters
│
├── modules/
│   ├── audio_processing/            # ★ APM 核心（编译进 webrtc_apm）
│   │   ├── include/                 #   对外主接口（audio_processing.h, config.h, ...）
│   │   ├── aec3/                    #   AEC3 核心实现
│   │   ├── aecm/                    #   AECM 核心实现
│   │   ├── agc/                     #   AGC1
│   │   ├── agc2/                    #   AGC2
│   │   ├── ns/                      #   NS + NSX
│   │   ├── vad/                     #   VAD（GMM + RNN VAD）
│   │   ├── echo_detector/           #   回声检测（AEC3 辅助）
│   │   ├── utility/                 #   工具函数（delay_estimator, pffft_wrapper ...）
│   │   ├── null_aec_dump_factory.cc #   排除整个 aec_dump/ 后单独保留的 stub
│   │   └── logging/                 #   APM 数据 dump 器
│   │
│   └── audio_coding/
│       ├── neteq/                   # ★ NetEQ 核心（编译进 webrtc_neteq）
│       │   ├── neteq_impl.cc               ← NetEQ 主实现
│       │   ├── decision_logic.cc            ← 状态机
│       │   ├── packet_buffer.cc             ← 抖动缓冲
│       │   ├── delay_manager.cc             ← 自适应延迟
│       │   ├── expand.cc / preemptive_expand.cc
│       │   ├── accelerate.cc / normal.cc / merge.cc
│       │   ├── comfort_noise.cc            ← CNG
│       │   ├── dtmf_buffer.cc / dtmf_tone_generator.cc
│       │   ├── time_stretch.cc              ← 时间拉伸
│       │   ├── nack_tracker.cc              ← NACK 反馈
│       │   ├── decoder_database.cc          ← 解码器注册
│       │   └── statistics_calculator.cc
│       │
│       └── codecs/
│           ├── isac/                #   ISAC 家族（8 个库）
│           │   ├── main/{source,include}/  → isac_vad, isac_c, isac
│           │   └── fix/{source,include}/  → isac_fix_common, isac_fix_c, isac_fix
│           ├── cng/                 #   webrtc_cng（舒适噪声生成器）
│           └── legacy_encoded_audio_frame.cc  → legacy_encoded_audio_frame
│
├── modules/third_party/fft/        # fft.c → fft 库（ISAC 依赖）
│
├── api/
│   ├── audio/                       # AudioFrame / EchoCanceller3Config / ...
│   ├── audio_codecs/                # AudioEncoder / AudioDecoder 接口
│   ├── neteq/                       # NetEQ 对外 API（neteq.h, neteq_factory.h ...）
│   ├── task_queue/                  # TaskQueue 接口（非 Win/Apple 默认 stdlib 实现）
│   ├── units/                       # Timestamp / TimeDelta / DataRate
│   ├── array_view.h                 # span 类
│   ├── audio_options.h
│   ├── function_view.h
│   ├── ref_counted_base.h
│   ├── scoped_refptr.h
│   └── rtp_headers.h / rtp_packet_info.h / rtp_packet_infos.h
│
├── rtc_base/                        # ★ rtc_base 库
│   ├── system/                      # arch / file_wrapper / thread_registry（仅 Android）
│   ├── synchronization/             # mutex / rw_lock (win+posix) / critical_section / sequence_checker
│   ├── memory/                      # aligned_malloc / fifo_buffer
│   ├── numerics/                    # exp_filter / histogram / moving_average / sample_stats / ...
│   ├── strings/                     # string_builder / string_format / audio_format_to_string
│   ├── task_queue*.cc               # win / stdlib / gcd / libevent 四套后端
│   ├── time_utils.cc
│   ├── experiments/                 # 字段试验解析
│   ├── task_utils/                  # pending_task_safety_flag / repeating_task
│   └── third_party/                 # base64 / sigslot
│
├── common_audio/                    # ★ common_audio 库
│   ├── signal_processing/           # SPL 库（resample / fft / filter / ...）
│   ├── resampler/                   # PushResampler / SincResampler
│   ├── vad/                         # 独立 VAD
│   ├── third_party/                 # spl_sqrt_floor / ooura_fft (sse2 + generic)
│   └── ...
│
├── system_wrappers/                 # ★ system_wrappers 库
│   ├── include/field_trial.h        #   对外头文件
│   ├── include/metrics.h            #   对外头文件
│   └── source/
│       ├── field_trial.cc           #   键/值字符串解析器（InitFieldTrialsFromString）
│       ├── metrics.cc               #   进程内 RtcHistogramMap（Enable / GetAndReset）
│       ├── clock.cc / cpu_info.cc / cpu_features.cc
│       └── rtp_to_ntp_estimator.cc / sleep.cc
│
├── sdk/android/native_api/stacktrace/   # StackTraceElement 头文件（仅 Android）
│
├── third_party/
│   ├── abseil-cpp/                  # Abseil‑CPP 子集（string_view, span, bind_front, ...）
│   ├── pffft/                       # pffft.c → pffft 库
│   ├── rnnoise/                     # rnn_vad_weights.cc → rnn_vad 库
│   └── jsoncpp/                     # json_reader/value/writer.cpp → jsoncpp 库
│
└── example/                         # 7 个示例程序
    ├── basic_apm.cc
    ├── apm_pipeline.cc
    ├── hpf_aec_ns_agc_vad.cc
    ├── aec_demo.cc
    ├── ns_demo.cc
    ├── agc_demo.cc
    └── vad_demo.cc
```

---

## 核心接口速览

### APM 用法

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

// 处理每帧 10ms 音频
//   capture_frame —— 麦克风数据
//   render_frame  —— 远端参考（AEC 必需）
apm->ProcessStream(capture_frame);
apm->ProcessReverseStream(render_frame);

auto stats = apm->GetStatistics();
// stats.echo_cancel.erl_db
// stats.noise_suppression.noise_level
// stats.voice_detected
```

### NetEQ 用法

```cpp
#include "api/neteq/neteq.h"
#include "api/neteq/neteq_factory.h"

NetEq::Config cfg;
cfg.sample_rate_hz = 16000;
cfg.max_delay_ms = 200;
auto neteq = NetEqFactory::Create(cfg);

// 喂入 RTP 包（pkt = RTP 头 + payload）
neteq->InsertPacket(rtp_timestamp, payload_type, pkt.data(), pkt.size(),
                    /*is_dtx=*/false, /*speaker_angle=*/0.0f);

// 获取 10 ms 解码音频
int16_t out[160];
NetEq::AudioFrameInfo info;
auto status = neteq->GetAudio(out, /*samples_per_channel=*/160,
                              /*num_channels=*/1, &info);

NetEq::NetworkStatistics stats;
neteq->GetNetworkStatistics(&stats);
```

### Field trial / metrics（可选）

```cpp
#include "system_wrappers/include/field_trial.h"
#include "system_wrappers/include/metrics.h"

// 启动进程内 metrics 收集（在任何 histogram 使用前调用一次）
webrtc::metrics::Enable();

// 注入实验开关："WebRTC-experimentFoo/Enabled/WebRTC-experimentBar/Enabled100kbps/"
webrtc::field_trial::InitFieldTrialsFromString(kMyTrials);

if (webrtc::field_trial::IsEnabled("WebRTC-experimentFoo")) {
  // ...
}

// 读取累积的 histogram 数据
std::map<std::string, std::unique_ptr<webrtc::metrics::SampleInfo>> hists;
webrtc::metrics::GetAndReset(&hists);
```

详细接口：
- APM：`modules/audio_processing/include/audio_processing.h`
- NetEQ：`api/neteq/neteq.h`
- Metrics / FieldTrial：`system_wrappers/include/metrics.h`、`system_wrappers/include/field_trial.h`

---

## 示例程序

编译后位于 `build/Release/`，无命令行参数，运行即生成合成音频并处理：

| 示例 | 功能说明 |
|------|---------|
| `basic_apm` | HPF + NS，每帧 InputRMS / OutputRMS / 衰减 dB |
| `apm_pipeline` | HPF→AEC→NS→AGC→VAD 全流水线 |
| `hpf_aec_ns_agc_vad` | 同上 + ERLE / 噪声衰减 dB / VAD 概率 |
| `aec_demo` | ERLE (dB)，模拟回声消除效果 |
| `ns_demo` | 噪声衰减量 (dB) |
| `agc_demo` | 输入/输出电平对比 |
| `vad_demo` | 每帧语音概率 0.0 – 1.0 |

---

## 跨平台支持

| 平台 | 宏 | 链接库 |
|------|----|--------|
| **Windows** | `WEBRTC_WIN` + `NOMINMAX` | `winmm ws2_32` |
| **Linux** | `WEBRTC_LINUX` + `WEBRTC_POSIX` | `pthread` |
| **macOS** | `WEBRTC_APPLE` + `WEBRTC_OSX` + `WEBRTC_POSIX` | `pthread` + `CoreAudio` + `AudioToolbox` |
| **iOS** | `WEBRTC_APPLE` + `WEBRTC_IOS` + `WEBRTC_POSIX` | `pthread` + `CoreAudio` + `AudioToolbox` |
| **Android** | `WEBRTC_ANDROID` + `WEBRTC_POSIX` | `pthread log` |
| **Emscripten** | `WEBRTC_LINUX` + `WEBRTC_POSIX` | (无) |

架构自动检测（`CMakeLists.txt`）：

- x86 / x86_64 → `WEBRTC_USE_SSE2=ON`，NEON 源码被过滤
- ARM32 / ARM64 → `WEBRTC_USE_NEON=ON`，SSE 源码被过滤
- MIPS → 默认过滤

非目标平台的汇编源码（`*_mips*`、`*_neon*`、`*_sse*`）在 CMake configure 阶段被排除。

---

## 源码来源与筛选

### 源码来源

整个工程从以下目录提取：

```
D:\newwebrtc\webrtc-checkout\src
```

**参考源码目录只读**，绝不删除任何文件——本工程是独立工作副本，包含相同的 APM + NetEQ 源码和手工调优的 CMakeLists.txt。

### 筛选规则（在 CMakeLists.txt 中应用）

| 规则 | 排除内容 |
|------|---------|
| `unittest` 正则 | 所有单元测试源码 |
| `_test\.cc$` 正则 | 所有测试可执行文件 |
| `mips` 正则 | MIPS SIMD 源码 |
| `neon` 正则 | ARM NEON 源码（非 ARM 构建） |
| `sse` 正则 | SSE 源码（非 x86 构建） |
| `/mock/` 正则 | Mock 实现 |
| `/aec_dump/` 正则 | AEC dump 子系统（保留 `null_aec_dump_factory.cc` stub） |
| `field_trial.cc` / `metrics.cc` | 从 `system_wrappers/source/` 编译（WebRTC 默认实现），**不是** `third_party/webrtc_overrides/`（Chromium 桥接层） |
| ISAC MIPS / Neutrino | 通过显式文件列表排除 |
| Android 保护 | `thread_registry.cc` / `warn_current_thread_is_deadlocked.cc` 仅 Android |

### 为什么 `field_trial` / `metrics` 在 `system_wrappers` 里

WebRTC 提供两种实现路径：

| 位置 | 类型 | 依赖 |
|------|------|------|
| `system_wrappers/source/field_trial.cc` | **WebRTC 默认实现** | 无依赖（纯键值字符串解析） |
| `system_wrappers/source/metrics.cc` | **WebRTC 默认实现** | 无依赖（进程内 `RtcHistogramMap` + `std::map`） |
| `third_party/webrtc_overrides/field_trial.cc` | Chromium 桥接层 | `base/metrics/field_trial.h` |
| `third_party/webrtc_overrides/metrics.cc` | Chromium 桥接层 | `base/metrics/histogram.h` |

本工程用 **WebRTC 默认实现**——**零 Chromium 依赖**。`third_party/webrtc_overrides/` 里的 Chromium 桥接文件虽然在源码树里存在，但**故意不编译**。

### 头文件处理

每个 `add_library` target 都通过 `file(GLOB ... *_HEADERS CONFIGURE_DEPENDS ...)` 把对应 `.h` 头文件也收进来。这样 Visual Studio Solution Explorer 里显示完整物理目录树，IntelliSense 完全可索引、可跳转。

### 构建 target 统计

| Target | 源文件数 | 输出 |
|--------|---------|------|
| `rtc_base` | ~63 .cc | `rtc_base.lib` |
| `common_audio` | ~30 .cc + .c | `common_audio.lib` |
| `system_wrappers` | ~7 .cc / .c（含 `field_trial.cc` + `metrics.cc`） | `system_wrappers.lib` |
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

## 许可证

BSD 3‑Clause License，与 WebRTC 原始代码一致。