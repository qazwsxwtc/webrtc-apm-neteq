#ifdef _MSC_VER
#pragma comment(lib, "legacy_stdio_definitions.lib")
#endif

#include <math.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <string_view>

extern "C" {

int8_t kWebRtcSpl_CountLeadingZeros32_Table[64] = {
    63, 16, 62,  7, 15, 36, 61,  3,  6, 14, 25, 19, 35, 41, 60,  1,
     8,  5,  4, 11, 14, 23, 18, 31, 10, 13, 22, 29, 17, 28, 39,  2,
     0, 17, 63,  6, 16, 37, 62,  4,  7, 15, 26, 20, 36, 42, 61,  1,
     9,  6,  5, 12, 15, 24, 19, 32, 11, 14, 23, 30, 18, 29, 40,  0
};

void WebRtcSpl_MemSetW16(int16_t* v, int16_t val, size_t n) {
    for (size_t i = 0; i < n; ++i) v[i] = val;
}
void WebRtcSpl_MemSetW32(int32_t* v, int32_t val, size_t n) {
    for (size_t i = 0; i < n; ++i) v[i] = val;
}
void WebRtcSpl_MemCpyReversedOrder(int16_t* dst, const int16_t* src, size_t n) {
    for (size_t i = 0; i < n; ++i) dst[i] = src[n - 1 - i];
}
void WebRtcSpl_CopyFromEndW16(const int16_t* src, size_t src_len,
                               size_t n, int16_t* dst) {
    for (size_t i = 0; i < n; ++i) dst[i] = src[src_len - n + i];
}
void WebRtcSpl_ZerosArrayW16(int16_t* v, size_t n) { memset(v, 0, n * 2); }
void WebRtcSpl_ZerosArrayW32(int32_t* v, size_t n) { memset(v, 0, n * 4); }

int16_t WebRtcSpl_MaxAbsValueW16C(const int16_t* v, size_t n) {
    int16_t m = 0;
    for (size_t i = 0; i < n; ++i) {
        int16_t a = v[i] < 0 ? (int16_t)(-v[i]) : v[i];
        if (a > m) m = a;
    }
    return m;
}
int32_t WebRtcSpl_MaxAbsValueW32C(const int32_t* v, size_t n) {
    int32_t m = 0;
    for (size_t i = 0; i < n; ++i) {
        int32_t a = v[i] < 0 ? -v[i] : v[i];
        if (a > m) m = a;
    }
    return m;
}
int16_t WebRtcSpl_MaxValueW16C(const int16_t* v, size_t n) {
    int16_t m = v[0]; for (size_t i = 1; i < n; ++i) if (v[i] > m) m = v[i]; return m;
}
int32_t WebRtcSpl_MaxValueW32C(const int32_t* v, size_t n) {
    int32_t m = v[0]; for (size_t i = 1; i < n; ++i) if (v[i] > m) m = v[i]; return m;
}
int16_t WebRtcSpl_MinValueW16C(const int16_t* v, size_t n) {
    int16_t m = v[0]; for (size_t i = 1; i < n; ++i) if (v[i] < m) m = v[i]; return m;
}
int32_t WebRtcSpl_MinValueW32C(const int32_t* v, size_t n) {
    int32_t m = v[0]; for (size_t i = 1; i < n; ++i) if (v[i] < m) m = v[i]; return m;
}
size_t WebRtcSpl_MaxAbsIndexW16(const int16_t* v, size_t n) {
    size_t idx = 0; int16_t m = v[0] < 0 ? (int16_t)(-v[0]) : v[0];
    for (size_t i = 1; i < n; ++i) {
        int16_t a = v[i] < 0 ? (int16_t)(-v[i]) : v[i];
        if (a > m) { m = a; idx = i; }
    }
    return idx;
}
size_t WebRtcSpl_MaxIndexW16(const int16_t* v, size_t n) {
    size_t idx = 0; for (size_t i = 1; i < n; ++i) if (v[i] > v[idx]) idx = i; return idx;
}
size_t WebRtcSpl_MaxIndexW32(const int32_t* v, size_t n) {
    size_t idx = 0; for (size_t i = 1; i < n; ++i) if (v[i] > v[idx]) idx = i; return idx;
}
size_t WebRtcSpl_MinIndexW16(const int16_t* v, size_t n) {
    size_t idx = 0; for (size_t i = 1; i < n; ++i) if (v[i] < v[idx]) idx = i; return idx;
}
size_t WebRtcSpl_MinIndexW32(const int32_t* v, size_t n) {
    size_t idx = 0; for (size_t i = 1; i < n; ++i) if (v[i] < v[idx]) idx = i; return idx;
}

void WebRtcSpl_VectorBitShiftW16(int16_t* out, size_t n, const int16_t* in, int16_t rsh) {
    for (size_t i = 0; i < n; ++i) {
        if (rsh >= 0) out[i] = (int16_t)(in[i] >> rsh);
        else out[i] = (int16_t)(in[i] << (-rsh));
    }
}
void WebRtcSpl_VectorBitShiftW32(int32_t* out, size_t n, const int32_t* in, int16_t rsh) {
    for (size_t i = 0; i < n; ++i) {
        if (rsh >= 0) out[i] = in[i] >> rsh;
        else out[i] = in[i] << (-rsh);
    }
}
void WebRtcSpl_VectorBitShiftW32ToW16(int16_t* out, size_t n, const int32_t* in, int rsh) {
    for (size_t i = 0; i < n; ++i) {
        int32_t v = rsh >= 0 ? (in[i] >> rsh) : (in[i] << (-rsh));
        if (v > 32767) v = 32767; if (v < -32768) v = -32768;
        out[i] = (int16_t)v;
    }
}
void WebRtcSpl_ScaleVector(const int16_t* in, int16_t* out, int16_t gain, size_t n) {
    for (size_t i = 0; i < n; ++i) {
        int32_t v = (int32_t)in[i] * gain;
        if (v > 32767) v = 32767; if (v < -32768) v = -32768;
        out[i] = (int16_t)v;
    }
}
void WebRtcSpl_ScaleVectorWithSat(const int16_t* in, int16_t* out, int16_t gain, size_t n) {
    WebRtcSpl_ScaleVector(in, out, gain, n);
}
void WebRtcSpl_ScaleAndAddVectors(const int16_t* a, int16_t ga, const int16_t* b, int16_t gb, int16_t* out, size_t n) {
    for (size_t i = 0; i < n; ++i) {
        int32_t v = (int32_t)a[i] * ga + (int32_t)b[i] * gb;
        if (v > 32767) v = 32767; if (v < -32768) v = -32768;
        out[i] = (int16_t)v;
    }
}
int WebRtcSpl_ScaleAndAddVectorsWithRoundC(const int16_t* a, int16_t ga, const int16_t* b, int16_t gb, int rsh, int16_t* out, size_t n) {
    for (size_t i = 0; i < n; ++i) {
        int32_t s = (int32_t)a[i] * ga + (int32_t)b[i] * gb;
        if (rsh >= 0) s = (s + (1 << (rsh - 1))) >> rsh; else s <<= (-rsh);
        if (s > 32767) s = 32767; if (s < -32768) s = -32768;
        out[i] = (int16_t)s;
    }
    return 0;
}

void WebRtcSpl_ReverseOrderMultArrayElements(int16_t* out, const int16_t* in, const int16_t* w, size_t n, int16_t rsh) {
    for (size_t i = 0; i < n; ++i) {
        int32_t v = (int32_t)in[n - 1 - i] * w[i];
        if (rsh > 0) v >>= rsh; out[i] = (int16_t)v;
    }
}
void WebRtcSpl_ElementwiseVectorMult(int16_t* out, const int16_t* in, const int16_t* w, size_t n, int16_t rsh) {
    for (size_t i = 0; i < n; ++i) {
        int32_t v = (int32_t)in[i] * w[i];
        if (rsh > 0) v >>= rsh; out[i] = (int16_t)v;
    }
}
void WebRtcSpl_AddVectorsAndShift(int16_t* out, const int16_t* a, const int16_t* b, int16_t rsh, size_t n) {
    for (size_t i = 0; i < n; ++i) {
        int32_t v = (int32_t)a[i] + b[i]; if (rsh > 0) v >>= rsh; out[i] = (int16_t)v;
    }
}
void WebRtcSpl_AddAffineVectorToVector(int16_t* out, const int16_t* in, int16_t gain, int32_t add_const, int16_t rsh, size_t n) {
    for (size_t i = 0; i < n; ++i) {
        int32_t v = (int32_t)in[i] * gain + add_const; if (rsh > 0) v >>= rsh; out[i] = (int16_t)v;
    }
}
void WebRtcSpl_AffineTransformVector(int16_t* out, const int16_t* in, int16_t gain, int32_t add_const, int16_t rsh, size_t n) {
    for (size_t i = 0; i < n; ++i) {
        int32_t v = (int32_t)in[i] * gain + add_const;
        if (rsh > 0) v >>= rsh;
        if (v > 32767) v = 32767; if (v < -32768) v = -32768;
        out[i] = (int16_t)v;
    }
}

size_t WebRtcSpl_AutoCorrelation(const int16_t* in, size_t in_len, int32_t* auto_corr, int32_t* energies) {
    int energy = 0;
    for (size_t i = 0; i < in_len; ++i) energy += in[i] * in[i];
    if (energies) energies[0] = energy;
    auto_corr[0] = energy;
    for (size_t k = 1; k < in_len; ++k) {
        int32_t s = 0;
        for (size_t i = 0; i < in_len - k; ++i) s += (int32_t)in[i] * in[i + k];
        auto_corr[k] = s;
    }
    return in_len;
}
int16_t WebRtcSpl_LevinsonDurbin(const int32_t* auto_corr, int16_t* lpc_coef, size_t order, int32_t* residual_energy) {
    if (auto_corr[0] == 0) return -1;
    for (size_t i = 0; i <= order; ++i) lpc_coef[i] = 0;
    *residual_energy = auto_corr[0];
    return 0;
}
void WebRtcSpl_ReflCoefToLpc(const int16_t* refl_coef, int16_t* lpc_coef, size_t order) {
    for (size_t i = 0; i < order; ++i) lpc_coef[i] = 0;
}
void WebRtcSpl_LpcToReflCoef(int16_t* lpc_coef, int16_t* refl_coef, size_t order) {
    for (size_t i = 0; i < order; ++i) refl_coef[i] = 0;
}
void WebRtcSpl_AutoCorrToReflCoef(const int32_t* auto_corr, int16_t* refl_coef, size_t order) {
    for (size_t i = 0; i < order; ++i) refl_coef[i] = 0;
}
void WebRtcSpl_CrossCorrelationC(int32_t* cross, const int16_t* seq1, const int16_t* seq2, size_t dim_seq, size_t dim_cross, int rsh, int step_seq2) {
    for (size_t i = 0; i < dim_cross; ++i) {
        int64_t sum = 0;
        for (size_t j = 0; j < dim_seq; ++j) {
            size_t idx = (size_t)i * step_seq2 + j;
            if (idx < dim_seq) sum += (int64_t)seq1[j] * seq2[idx];
        }
        if (rsh >= 0) cross[i] = (int32_t)(sum >> rsh); else cross[i] = (int32_t)(sum << (-rsh));
    }
}
void WebRtcSpl_GetHanningWindow(int16_t* win, size_t n) {
    for (size_t i = 0; i < n; ++i) {
        double v = 0.5 * (1.0 - cos(2.0 * 3.14159265358979323846 * (double)i / (double)(n - 1)));
        win[i] = (int16_t)(v * 16384.0);
    }
}
void WebRtcSpl_SqrtOfOneMinusXSquared(int16_t* in, size_t n, int16_t* out) {
    for (size_t i = 0; i < n; ++i) {
        double x = (double)in[i] / 16384.0;
        double r = sqrt(1.0 - x * x);
        if (r > 1.0) r = 1.0;
        out[i] = (int16_t)(r * 16384.0);
    }
}

int16_t WebRtcSpl_RandU(uint32_t* seed) {
    *seed = *seed * 1103515245u + 12345u;
    return (int16_t)((*seed >> 16) & 0x7FFF);
}
int16_t WebRtcSpl_RandN(uint32_t* seed) {
    return (int16_t)(WebRtcSpl_RandU(seed) - 16384);
}
int16_t WebRtcSpl_RandUArray(int16_t* v, size_t n, uint32_t* seed) {
    int16_t last = 0;
    for (size_t i = 0; i < n; ++i) last = v[i] = WebRtcSpl_RandU(seed);
    return last;
}
int32_t WebRtcSpl_Sqrt(int32_t val) {
    if (val <= 0) return 0;
    return (int32_t)sqrt((double)val);
}
uint32_t WebRtcSpl_DivU32U16(uint32_t num, uint16_t den) {
    return den == 0 ? 0 : num / den;
}
int32_t WebRtcSpl_DivW32W16(int32_t num, int16_t den) {
    return den == 0 ? 0 : num / den;
}
int16_t WebRtcSpl_DivW32W16ResW16(int32_t num, int16_t den) {
    if (den == 0) return 0;
    int32_t r = num / den;
    if (r > 32767) r = 32767; if (r < -32768) r = -32768;
    return (int16_t)r;
}
int32_t WebRtcSpl_DivResultInQ31(int32_t num, int32_t den) {
    if (den == 0) return 0;
    return (int32_t)(((int64_t)num << 31) / den);
}
int32_t WebRtcSpl_DivW32HiLow(int32_t num, int16_t den_hi, int16_t den_low) {
    int32_t den = ((int32_t)den_hi << 16) | (den_low & 0xFFFF);
    return den == 0 ? 0 : num / den;
}
int32_t WebRtcSpl_Energy(int16_t* v, size_t n, int* scale) {
    int32_t e = 0;
    for (size_t i = 0; i < n; ++i) e += (int32_t)v[i] * v[i];
    *scale = 0; return e;
}

size_t WebRtcSpl_FilterAR(const int16_t* ar, size_t ar_len, int16_t* in, size_t in_len, int16_t* out, int16_t* state, size_t state_len) {
    if (state) for (size_t i = 0; i < state_len; ++i) state[i] = 0;
    for (size_t i = 0; i < in_len; ++i) out[i] = in[i];
    return in_len;
}
void WebRtcSpl_FilterMAFastQ12(const int16_t* in, int16_t* out, const int16_t* ma, size_t ma_len, size_t n) {
    for (size_t i = 0; i < n; ++i) {
        int32_t v = 0;
        for (size_t j = 0; j < ma_len && j <= i; ++j) v += (int32_t)ma[j] * in[i - j];
        out[i] = (int16_t)(v >> 12);
    }
}
void WebRtcSpl_FilterARFastQ12(const int16_t* in, int16_t* out, const int16_t* ar, size_t ar_len, size_t n) {
    for (size_t i = 0; i < n; ++i) {
        int32_t v = (int32_t)in[i] * ar[0];
        for (size_t j = 1; j < ar_len; ++j) {
            int32_t src = (i >= j) ? out[i - j] : in[0];
            v += (int32_t)ar[j] * src;
        }
        out[i] = (int16_t)(v >> 12);
    }
}

int WebRtcSpl_DownsampleFastC(const int16_t* in, size_t in_len, int16_t* out, size_t out_len, const int16_t* coef, size_t coef_len, int factor, size_t delay) {
    if (factor <= 0) return -1;
    for (size_t i = 0; i < out_len; ++i) {
        int32_t v = 0;
        for (size_t j = 0; j < coef_len; ++j) {
            size_t idx = (size_t)i * factor + j;
            if (idx < in_len) v += (int32_t)coef[j] * in[idx];
        }
        out[i] = (int16_t)(v >> 12);
    }
    return 0;
}

int WebRtcSpl_ComplexFFT(int16_t v[], int stages, int mode) { return 0; }
int WebRtcSpl_ComplexIFFT(int16_t v[], int stages, int mode) { return 0; }
void WebRtcSpl_ComplexBitReverse(int16_t* data, int stages) {}

void WebRtcSpl_Resample22khzTo16khz(const int16_t* in, size_t in_len, int16_t* out, size_t* out_len, void*) { *out_len = in_len; for (size_t i = 0; i < in_len; ++i) out[i] = in[i]; }
void WebRtcSpl_ResetResample22khzTo16khz(void*) {}
void WebRtcSpl_Resample16khzTo22khz(const int16_t* in, size_t in_len, int16_t* out, size_t* out_len, void*) { *out_len = in_len; for (size_t i = 0; i < in_len; ++i) out[i] = in[i]; }
void WebRtcSpl_ResetResample16khzTo22khz(void*) {}
void WebRtcSpl_Resample22khzTo8khz(const int16_t* in, size_t in_len, int16_t* out, size_t* out_len, void*) { *out_len = in_len; for (size_t i = 0; i < in_len; ++i) out[i] = in[i]; }
void WebRtcSpl_ResetResample22khzTo8khz(void*) {}
void WebRtcSpl_Resample8khzTo22khz(const int16_t* in, size_t in_len, int16_t* out, size_t* out_len, void*) { *out_len = in_len; for (size_t i = 0; i < in_len; ++i) out[i] = in[i]; }
void WebRtcSpl_ResetResample8khzTo22khz(void*) {}
void WebRtcSpl_Resample48khzTo32khz(const int32_t* In, int32_t* Out, size_t K) { for (size_t i = 0; i < K; ++i) Out[i] = In[i]; }
void WebRtcSpl_Resample32khzTo24khz(const int32_t* In, int32_t* Out, size_t K) { for (size_t i = 0; i < K; ++i) Out[i] = In[i]; }
void WebRtcSpl_Resample44khzTo32khz(const int32_t* In, int32_t* Out, size_t K) { for (size_t i = 0; i < K; ++i) Out[i] = In[i]; }
void WebRtcSpl_Resample48khzTo16khz(const int16_t* in, size_t in_len, int16_t* out, size_t* out_len, void*) { *out_len = in_len; for (size_t i = 0; i < in_len; ++i) out[i] = in[i]; }
void WebRtcSpl_ResetResample48khzTo16khz(void*) {}
void WebRtcSpl_Resample16khzTo48khz(const int16_t* in, size_t in_len, int16_t* out, size_t* out_len, void*) { *out_len = in_len; for (size_t i = 0; i < in_len; ++i) out[i] = in[i]; }
void WebRtcSpl_ResetResample16khzTo48khz(void*) {}
void WebRtcSpl_Resample48khzTo8khz(const int16_t* in, size_t in_len, int16_t* out, size_t* out_len, void*) { *out_len = in_len; for (size_t i = 0; i < in_len; ++i) out[i] = in[i]; }
void WebRtcSpl_ResetResample48khzTo8khz(void*) {}
void WebRtcSpl_Resample8khzTo48khz(const int16_t* in, size_t in_len, int16_t* out, size_t* out_len, void*) { *out_len = in_len; for (size_t i = 0; i < in_len; ++i) out[i] = in[i]; }
void WebRtcSpl_ResetResample8khzTo48khz(void*) {}

void WebRtcSpl_DownsampleBy2(const int16_t* in, size_t in_len, int16_t* out, size_t* out_len) {
    *out_len = in_len / 2; for (size_t i = 0; i < *out_len; ++i) out[i] = in[i * 2];
}
void WebRtcSpl_UpsampleBy2(const int16_t* in, size_t in_len, int16_t* out, size_t* out_len) {
    *out_len = in_len * 2; for (size_t i = 0; i < in_len; ++i) { out[i * 2] = in[i]; out[i * 2 + 1] = 0; }
}
void WebRtcSpl_AnalysisQMF(const int16_t* in, size_t in_len, int16_t* lo, int16_t* hi, size_t* lo_len, size_t* hi_len) {
    *lo_len = in_len / 2; *hi_len = in_len / 2;
    for (size_t i = 0; i < *lo_len; ++i) lo[i] = in[i * 2];
    for (size_t i = 0; i < *hi_len; ++i) hi[i] = in[i * 2 + 1];
}
void WebRtcSpl_SynthesisQMF(const int16_t* lo, const int16_t* hi, size_t lo_len, size_t hi_len, int16_t* out, size_t* out_len) {
    *out_len = lo_len + hi_len;
    for (size_t i = 0; i < lo_len; ++i) out[i * 2] = lo[i];
    for (size_t i = 0; i < hi_len; ++i) out[i * 2 + 1] = hi[i];
}

int32_t WebRtcSpl_SqrtFloor(int32_t val) {
    if (val <= 0) return 0;
    return (int32_t)floor(sqrt((double)val));
}

typedef int16_t (*MaxAbsValueW16_fn)(const int16_t*, size_t);
typedef int32_t (*MaxAbsValueW32_fn)(const int32_t*, size_t);
typedef int16_t (*MaxValueW16_fn)(const int16_t*, size_t);
typedef int32_t (*MaxValueW32_fn)(const int32_t*, size_t);
typedef int16_t (*MinValueW16_fn)(const int16_t*, size_t);
typedef int32_t (*MinValueW32_fn)(const int32_t*, size_t);
typedef int (*ScaleAndAddVectorsWithRound_fn)(const int16_t*, int16_t, const int16_t*, int16_t, int, int16_t*, size_t);
typedef void (*CrossCorrelation_fn)(int32_t*, const int16_t*, const int16_t*, size_t, size_t, int, int);
typedef int (*DownsampleFast_fn)(const int16_t*, size_t, int16_t*, size_t, const int16_t*, size_t, int, size_t);

MaxAbsValueW16_fn WebRtcSpl_MaxAbsValueW16 = WebRtcSpl_MaxAbsValueW16C;
MaxAbsValueW32_fn WebRtcSpl_MaxAbsValueW32 = WebRtcSpl_MaxAbsValueW32C;
MaxValueW16_fn WebRtcSpl_MaxValueW16 = WebRtcSpl_MaxValueW16C;
MaxValueW32_fn WebRtcSpl_MaxValueW32 = WebRtcSpl_MaxValueW32C;
MinValueW16_fn WebRtcSpl_MinValueW16 = WebRtcSpl_MinValueW16C;
MinValueW32_fn WebRtcSpl_MinValueW32 = WebRtcSpl_MinValueW32C;
ScaleAndAddVectorsWithRound_fn WebRtcSpl_ScaleAndAddVectorsWithRound = WebRtcSpl_ScaleAndAddVectorsWithRoundC;
CrossCorrelation_fn WebRtcSpl_CrossCorrelation = WebRtcSpl_CrossCorrelationC;
DownsampleFast_fn WebRtcSpl_DownsampleFast = WebRtcSpl_DownsampleFastC;

typedef struct VadInstStruct { int unused; } VadInst;

VadInst* WebRtcVad_Create(void) {
    return (VadInst*)malloc(sizeof(VadInst));
}
void WebRtcVad_Free(VadInst* handle) {
    if (handle) free(handle);
}
int WebRtcVad_Init(VadInst* handle) {
    if (!handle) return -1;
    handle->unused = 0; return 0;
}
int WebRtcVad_set_mode(VadInst* handle, int mode) {
    if (!handle || mode < 0 || mode > 3) return -1; return 0;
}
int WebRtcVad_Process(VadInst* handle, int fs, const int16_t* frame, size_t len) {
    if (!handle || !frame) return -1;
    int32_t sum = 0;
    for (size_t i = 0; i < len; ++i) sum += frame[i];
    return sum == 0 ? 0 : 1;
}
int WebRtcVad_ValidRateAndFrameLength(int rate, size_t len) {
    return (rate == 8000 || rate == 16000 || rate == 32000 || rate == 48000) ? 0 : -1;
}

int32_t WebRtcSpl_DotProductWithScale(const int16_t* vector1,
                                      const int16_t* vector2,
                                      size_t length,
                                      int scaling) {
    int64_t sum = 0;
    for (size_t i = 0; i < length; ++i) {
        sum += static_cast<int64_t>(vector1[i]) * vector2[i];
    }
    if (scaling > 0) {
        sum >>= scaling;
    } else if (scaling < 0) {
        sum <<= (-scaling);
    }
    if (sum > INT32_MAX) sum = INT32_MAX;
    if (sum < INT32_MIN) sum = INT32_MIN;
    return static_cast<int32_t>(sum);
}

}  // extern "C"

namespace absl {
bool EqualsIgnoreCase(std::string_view a, std::string_view b) {
    if (a.size() != b.size()) return false;
    for (size_t i = 0; i < a.size(); ++i) {
        char ca = a[i], cb = b[i];
        if (ca >= 'A' && ca <= 'Z') ca += 32;
        if (cb >= 'A' && cb <= 'Z') cb += 32;
        if (ca != cb) return false;
    }
    return true;
}
}  // namespace absl