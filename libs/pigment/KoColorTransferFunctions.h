/*
 *  SPDX-FileCopyrightText: 2021 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KOCOLORTRANSFERFUNCTIONS_H
#define KOCOLORTRANSFERFUNCTIONS_H

#include <cmath>

#include <QVector>
#include <QtGlobal>

#include <KoAlwaysInline.h>

/**
 * @brief The KoColorTransferFunctions class
 *
 * A number of often used transferFunctions.
 *
 * These functions can, at the time of writing, not be implemented
 * in ICC profiles, so instead, we apply or remove the curve as
 * necessary.
 */

// Not all embedded nclx color space definitions can be converted to icc, so we
// keep an enum to load those.
enum class LinearizePolicy { KeepTheSame, LinearFromPQ, LinearFromHLG, LinearFromSMPTE428 };

static constexpr uint16_t max12bit = 4095.f;
static constexpr float max16bit = 65535.0f;
static constexpr float multiplier10bit = 1.0f / 1023.0f;
static constexpr float multiplier12bit = 1.0f / 4095.0f;
static constexpr float multiplier16bit = 1.0f / max16bit;

enum class ConversionPolicy { KeepTheSame, ApplyPQ, ApplyHLG, ApplySMPTE428 };

ALWAYS_INLINE float applySmpte2048Curve(float x) noexcept
{
    const float m1 = 2610.0f / 4096.0f / 4.0f;
    const float m2 = 2523.0f / 4096.0f * 128.0f;
    const float a1 = 3424.0f / 4096.0f;
    const float c2 = 2413.0f / 4096.0f * 32.0f;
    const float c3 = 2392.0f / 4096.0f * 32.0f;
    const float a4 = 1.0f;
    const float x_p = powf(0.008f * std::max(0.0f, x), m1);
    const float res = powf((a1 + c2 * x_p) / (a4 + c3 * x_p), m2);
    return res;
}

ALWAYS_INLINE float removeSmpte2048Curve(float x) noexcept
{
    const float m1_r = 4096.0f * 4.0f / 2610.0f;
    const float m2_r = 4096.0f / 2523.0f / 128.0f;
    const float a1 = 3424.0f / 4096.0f;
    const float c2 = 2413.0f / 4096.0f * 32.0f;
    const float c3 = 2392.0f / 4096.0f * 32.0f;

    const float x_p = powf(x, m2_r);
    const float res = powf(qMax(0.0f, x_p - a1) / (c2 - c3 * x_p), m1_r);
    return res * 125.0f;
}

// From ITU Bt. 2390-8 pg. 31, this calculates the gamma for the nominal peak.
// This may differ per display regardless, but this is a good baseline.
ALWAYS_INLINE float HLGOOTFGamma(float nominalPeak) noexcept
{
    const float k = 1.111f;
    return 1.2f * powf(k, log2f(nominalPeak * (1.f / 1000.0f)));
}

// The HLG OOTF needs to be applied to convert from 'display linear' to 'scene linear'.
// Krita doesn't support sending tagged HLG to the display, so we have to pretend
// we're always converting from PQ to HLG.
ALWAYS_INLINE void applyHLGOOTF(float *rgb,
                                const double *lumaCoefficients,
                                float gamma = 1.2f,
                                float nominalPeak = 1000.0f) noexcept
{
    const float luma = (rgb[0] * static_cast<float>(lumaCoefficients[0]))
        + (rgb[1] * static_cast<float>(lumaCoefficients[1]))
        + (rgb[2] * static_cast<float>(lumaCoefficients[2]));
    const float a = nominalPeak * powf(luma, gamma - 1.f);
    rgb[0] *= a;
    rgb[1] *= a;
    rgb[2] *= a;
}

// The HLG OOTF needs to be removed to convert from 'scene linear' to 'display linear'.
// Krita doesn't support sending tagged HLG to the display, so we have to pretend
// we're always converting from HLG to PQ.
ALWAYS_INLINE void removeHLGOOTF(float *rgb,
                                 const double *lumaCoefficients,
                                 float gamma = 1.2f,
                                 float nominalPeak = 1000.0f) noexcept
{
    const float luma = (rgb[0] * static_cast<float>(lumaCoefficients[0]))
        + (rgb[1] * static_cast<float>(lumaCoefficients[1]))
        + (rgb[2] * static_cast<float>(lumaCoefficients[2]));
    const float multiplier = powf(luma * (1.f / nominalPeak), (1.f - gamma) * (1.f / gamma)) * (1.f / nominalPeak);
    rgb[0] *= multiplier;
    rgb[1] *= multiplier;
    rgb[2] *= multiplier;
}

ALWAYS_INLINE float applyHLGCurve(float x) noexcept
{
    const float a = 0.17883277f;
    const float b = 0.28466892f;
    const float c = 0.55991073f;

    if (x > 1.0f / 12.0f) {
        return (a * logf(12.0f * x - b) + c);
    } else {
        // return (sqrt(3.0) * powf(x, 0.5));
        return (sqrtf(3.0f) * sqrtf(x));
    }
}

ALWAYS_INLINE float removeHLGCurve(float x) noexcept
{
    const float a = 0.17883277f;
    const float b = 0.28466892f;
    const float c = 0.55991073f;
    if (x <= 0.5f) {
        // return (powf(x, 2.0) / 3.0);
        return x * x * (1.f / 3.0f);
    } else {
        return (expf((x - c) * (1.f / a)) + b) * (1.f / 12.0f);
    }
}

ALWAYS_INLINE float applySMPTE_ST_428Curve(float x) noexcept
{
    return powf(48.0f * x * (1.f / 52.37f), (1.f / 2.6f));
}

ALWAYS_INLINE float removeSMPTE_ST_428Curve(float x) noexcept
{
    return (52.37f / 48.0f) * powf(x, 2.6f);
}

#include <KoMultiArchBuildSupport.h>

#if defined(HAVE_XSIMD) && !defined(XSIMD_NO_SUPPORTED_ARCHITECTURE)

#include <KoStreamedMath.h>

template<typename Arch>
struct KoColorTransferFunctions {
    using float_v = typename KoStreamedMath<Arch>::float_v;

    static ALWAYS_INLINE void removeSmpte2048Curve(float_v &x) noexcept
    {
        constexpr float m1_r = 4096.0f * 4.0f / 2610.0f;
        constexpr float m2_r = 4096.0f / 2523.0f / 128.0f;
        constexpr float a1 = 3424.0f / 4096.0f;
        constexpr float c2 = 2413.0f / 4096.0f * 32.0f;
        constexpr float c3 = 2392.0f / 4096.0f * 32.0f;

        const float_v x_p = xsimd::pow(x, float_v(m2_r));
        const float_v res =
            xsimd::pow(xsimd::max(float_v(0.0f), x_p - a1) / (c2 - c3 * x_p),
                       float_v(m1_r));
        x = res * 125.0f;
    }

    static ALWAYS_INLINE void removeHLGCurve(float_v &x) noexcept
    {
        constexpr float a = 0.17883277f;
        constexpr float b = 0.28466892f;
        constexpr float c = 0.55991073f;

        const float_v x1 = x * x * (1.f / 3.0f);
        const float_v x2 =
            (xsimd::exp((x - c) * (1.f / a)) + b) * (1.f / 12.0f);
        x = xsimd::select(x <= float_v(0.5f), x1, x2);
    }

    static ALWAYS_INLINE void removeSMPTE_ST_428Curve(float_v &x) noexcept
    {
        x = (52.37f / 48.0f) * xsimd::pow(x, float_v(2.6f));
    }
};

#endif // HAVE_XSIMD

#endif // KOCOLORTRANSFERFUNCTIONS_H
