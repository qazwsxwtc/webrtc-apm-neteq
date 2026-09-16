# Patch Queue

本仓库是从 WebRTC 源码树硬拷贝后手工修补的。**所有 patch 按以下分类维护**。每次同步上游时，先 re-apply 这些 patch 再编译。

## 上游锁定

| 项目 | 值 |
|------|-----|
| **上游仓库本地路径** | `D:\newwebrtc\webrtc-checkout\src` |
| **WebRTC branch** | `branch-heads/4147` |
| **上游 HEAD commit** | `65e8d9facab05de13634d777702b2c93288f8849` |
| **对应 Chromium** | M131 |
| **拷贝日期** | 2026-09-11 |

> **注意**：上游 `D:\newwebrtc\webrtc-checkout\src` 最后两个 commit 是 Agora 镜像替换（DEPS 里下载地址改为 `webrtc.bj2.agoralab.co`），不影响 C++ 源码。原始 WebRTC baseline 为上述 commit 的 parent。

## Patch 分类

共 **14 个文件**有改动（本仓库 1110 个 tracking 非 third_party 源文件中）。

| 类 | 数量 | 说明 |
|----|------|------|
| **A — api/video/ 删除后的注释化** | 6 | 删除所有 video 模块后，注释掉 rtp_headers、experiments 中引用 VideoCodecType/VideoEncoder 等的声明 |
| **B — 修编译错误加 include** | 2 | absl 头文件不再传递性引入 `<memory>` / `<cstddef>`，需显式添加 |
| **C — Android Bionic 跨平台补丁** | 1 | `platform_thread_types.cc` 中的 `prctl`/`syscall` guard 原仅 `WEBRTC_LINUX`，扩展为 `WEBRTC_ANDROID` |
| **D — 添加公共 API** | 1 | `neteq_impl.cc` 文件末尾追加 `NetEq::Create()` 工厂函数 + `DefaultNetEqControllerFactory` |
| **E — 仅换行符/BOM，无功能** | 4 | Windows 编辑器编辑后产生，diff 仅末尾 `\ No newline at end of file` 差异 |

## A 类 — api/video/ 删除后的注释化

### A1: `api/rtp_headers.h`

**原因**：删除了 `api/video/` 目录（本仓库不含 video codec / renderer 相关模块），`ColorSpace` / `VideoRotation` / `VideoContentType` / `VideoSendTiming` / `FrameMarking` 类型定义消失。RTP header 本身仍在音频 RTP 中使用，但这些可选 video 字段必须注释。

**改动**：注释掉 5 个 `#include "api/video/..."` + 结构体中 5 组 video 相关成员字段（`hasVideoRotation` / `videoRotation` / `hasVideoContentType` / `videoContentType` / `has_video_timing` / `video_timing` / `has_frame_marking` / `frame_marking` / `color_space`）。

```diff
-#include "api/video/color_space.h"
+// #include "api/video/color_space.h"  // DISABLED - video not needed
-  bool hasVideoRotation;
-  VideoRotation videoRotation;
+  // DISABLED - video not needed
+  // bool hasVideoRotation;
+  // VideoRotation videoRotation;
```

### A2: `api/rtp_headers.cc`

**原因**：对应 A1，注释掉构造函数中 video 字段的初始化列表。

```diff
-      hasVideoRotation(false)
-      videoRotation(kVideoRotation_0)
+      // DISABLED - video not needed
+      // hasVideoRotation(false)
+      // videoRotation(kVideoRotation_0)
+      {}
```

### A3: `rtc_base/experiments/min_video_bitrate_experiment.h`

**原因**：`VideoCodecType` 定义已随 `api/video/` 删除。`GetExperimentalMinVideoBitrate(VideoCodecType)` 函数声明无法编译。

### A4: `rtc_base/experiments/balanced_degradation_settings.h`

**原因**：同上，`VideoCodecType` + `VideoEncoder::QpThresholds` 类型消失。注释掉 `MinFps` / `MaxFps` / `CanAdaptUp` / `CanAdaptUpResolution` / `GetQpThresholds` 五个方法声明。

### A5: `rtc_base/experiments/quality_scaling_experiment.h`

**原因**：同上，注释掉 `GetQpThresholds(VideoCodecType)` 静态方法声明。

### A6: `rtc_base/experiments/rate_control_settings.h`

**原因**：同上，注释掉 `WebRtcKeyValueConfig`（其实和 video 无关但一起处理了）、`VideoCodecMode`、`VideoEncoderConfig::ContentType` 相关声明。

## B 类 — 修编译错误加 include

### B1: `modules/audio_processing/aec3/clockdrift_detector.h`

**原因**：上游版本间接 include `<cstddef>`（通过旧版 absl）。新 absl 不再传递性引入。`std::size_t` 需要显式 include。

```diff
 #include <array>
+#include <cstddef>
```

### B2: `modules/audio_processing/aec3/reverb_model_estimator.h`

**原因**：同上。新 WebRTC/absl 不再自动引入 `<memory>`，但代码里用到 `std::unique_ptr`（通过头文件中 template 或 friend 声明）。

```diff
 #include <array>
+#include <memory>
 #include <vector>
```

## C 类 — Android Bionic 跨平台补丁

### C1: `rtc_base/platform_thread_types.cc`

**原因**：原始 guard 仅 `#if defined(WEBRTC_LINUX)`，但 Android Bionic libc 同样提供 `prctl()` + `SYS_gettid` syscall。Android 目标上编译时 `WEBRTC_LINUX` 未定义（只定义 `WEBRTC_ANDROID`），导致 `prctl` / `gettid` 未声明错误。

```diff
-#if defined(WEBRTC_LINUX)
+#if defined(WEBRTC_LINUX) || defined(WEBRTC_ANDROID)
+#ifndef _GNU_SOURCE
+#define _GNU_SOURCE
+#endif
 #include <sys/prctl.h>
 #include <sys/syscall.h>
```

**影响**：`rtc::GetCurrentThreadId()` 在 Android 上现在返回正确的 `gettid()` 值；`rtc::SetThreadName()` 可以通过 `prctl(PR_SET_NAME)` 设置线程名。

## D 类 — 添加公共 API

### D1: `modules/audio_coding/neteq/neteq_impl.cc`

**原因**：上游 WebRTC 的 `NetEq::Create()` 工厂函数声明在 `api/neteq/neteq_factory.h` 里，但实现**不在**标准 `neteq_impl.cc` 中——它在 `api/neteq/default_neteq_controller_factory.cc` 里（通过 GN target 分开编译）。本仓库把所有 neteq 源文件合并进一个 target，`default_neteq_controller_factory.cc` 提供的符号在本仓库 target 链接后不可见（实际上它是编进去的... 待确认）。

**改动**：在 `neteq_impl.cc` 文件末尾 `}  // namespace webrtc` 之前追加：

```cpp
// ---------- NetEq::Create ----------
namespace {
class DefaultNetEqControllerFactory : public NetEqControllerFactory {
 public:
  std::unique_ptr<NetEqController> CreateNetEqController(
      const NetEqController::Config& config) const override {
    return std::make_unique<DecisionLogic>(config);
  }
};
}  // namespace

NetEq* NetEq::Create(
    const NetEq::Config& config,
    Clock* clock,
    const rtc::scoped_refptr<AudioDecoderFactory>& decoder_factory) {
  DefaultNetEqControllerFactory controller_factory;
  NetEqImpl::Dependencies deps(config, clock, decoder_factory,
                                controller_factory);
  return new NetEqImpl(config, std::move(deps));
}
```

## E 类 — 仅换行符/BOM（无功能）

| 文件 | 差异 |
|------|------|
| `api/neteq/neteq_factory.h` | 末尾 `\n` 缺失 |
| `modules/audio_processing/include/audio_processing.h` | 末尾 `\n` 缺失 |
| `rtc_base/strings/audio_format_to_string.cc` | 末尾 `\n` 缺失 |
| `rtc_base/synchronization/rw_lock_posix.cc` | 末尾 `\n` 缺失 |

> reverb_model_estimator.h 额外有 UTF-8 BOM（`\xEF\xBB\xBF`），Windows 编辑器保存时自动添加，对 GCC/Clang/MSVC 都无影响。

## 同步上游操作手册

当上游 WebRTC 有安全补丁或 AEC3/NetEQ bug fix 需要合入时：

```bash
# 1. 从上游路径拷贝更新后的源文件
xcopy /SY D:\newwebrtc\webrtc-checkout\src\modules\audio_processing\aec3\* modules\audio_processing\aec3\
xcopy /SY D:\newwebrtc\webrtc-checkout\src\modules\audio_coding\neteq\* modules\audio_coding\neteq\
...

# 2. 对每个被更新的 patch 文件，重新应用 patch
#    A 类（api/video/ 删除后的注释化）：手动重注释
#    B 类（缺 include）：手动重加 include
#    C 类（platform_thread_types）：改 guard + 加 _GNU_SOURCE
#    D 类（neteq_impl.cc 末尾追加）：确认 NetEq::Create() 是否仍有效
#    E 类（换行符）：无影响

# 3. 编译验证
cmake --build build --config Release --parallel 8
```

**风险点**：每次上游更新后，`api/rtp_headers.h` 和 `neteq_impl.cc` 是最高风险——前者改动多（video 字段注释散在文件各处），后者 NetEq::Create() 签名在上游改了会编译失败。