/*
 *  SPDX-FileCopyrightText: 2014 Dmitry Kazakov <dimula73@gmail.com>
 *  SPDX-FileCopyrightText: 2022 L. E. Segovia <amy@amyspark.me>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_ANTIALIASING_FADE_MAKER_H
#define __KIS_ANTIALIASING_FADE_MAKER_H

#include <kis_global.h>

#include <xsimd_extensions/xsimd.hpp>

template<class BaseFade>
class KisAntialiasingFadeMaker1D
{
public:
    KisAntialiasingFadeMaker1D(const BaseFade &baseFade, bool enableAntialiasing)
        : m_radius(0.0)
        , m_fadeStartValue(0)
        , m_antialiasingFadeStart(0)
        , m_antialiasingFadeCoeff(0)
        , m_enableAntialiasing(enableAntialiasing)
        , m_baseFade(baseFade)
    {
    }

    KisAntialiasingFadeMaker1D(const KisAntialiasingFadeMaker1D &rhs, const BaseFade &baseFade)
        : m_radius(rhs.m_radius)
        , m_fadeStartValue(rhs.m_fadeStartValue)
        , m_antialiasingFadeStart(rhs.m_antialiasingFadeStart)
        , m_antialiasingFadeCoeff(rhs.m_antialiasingFadeCoeff)
        , m_enableAntialiasing(rhs.m_enableAntialiasing)
        , m_baseFade(baseFade)
    {
    }

    void setSquareNormCoeffs(qreal xcoeff, qreal ycoeff)
    {
        m_radius = 1.0;

        const qreal xf = qMax(0.0, ((1.0 / xcoeff) - 1.0) * xcoeff);
        const qreal yf = qMax(0.0, ((1.0 / ycoeff) - 1.0) * ycoeff);

        m_antialiasingFadeStart = pow2(0.5 * (xf + yf));

        m_fadeStartValue = m_baseFade.value(m_antialiasingFadeStart);
        m_antialiasingFadeCoeff = qMax(0.0, 255.0 - m_fadeStartValue) / (m_radius - m_antialiasingFadeStart);
    }

    void setRadius(qreal radius)
    {
        m_radius = radius;
        m_antialiasingFadeStart = qMax(0.0, m_radius - 1.0);

        m_fadeStartValue = m_baseFade.value(m_antialiasingFadeStart);
        m_antialiasingFadeCoeff = qMax(0.0, 255.0 - m_fadeStartValue) / (m_radius - m_antialiasingFadeStart);
    }

    inline bool needFade(qreal dist, quint8 *value)
    {
        if (dist > m_radius) {
            *value = 255;
            return true;
        }

        if (!m_enableAntialiasing) {
            return false;
        }

        if (dist > m_antialiasingFadeStart) {
            *value = m_fadeStartValue + (dist - m_antialiasingFadeStart) * m_antialiasingFadeCoeff;
            return true;
        }

        return false;
    }

#if defined(HAVE_XSIMD) && !defined(XSIMD_NO_SUPPORTED_ARCHITECTURE)
    template<typename A>
    xsimd::batch_bool<float, A> needFade(xsimd::batch<float, A> &dist)
    {
        using float_v = xsimd::batch<float, A>;
        using float_m = typename float_v::batch_bool_type;

        const float_v vOne(1);
        const float_v vValMax(255.f);

        const float_v vRadius(m_radius);
        const float_v vFadeStartValue(m_fadeStartValue);
        const float_v vAntialiasingFadeStart(m_antialiasingFadeStart);
        const float_v vAntialiasingFadeCoeff(m_antialiasingFadeCoeff);

        const float_m outsideMask = dist > vRadius;
        dist = xsimd::set_one(dist, outsideMask);

        float_m fadeStartMask(false);

        if (m_enableAntialiasing) {
            fadeStartMask = dist > vAntialiasingFadeStart;
            dist = xsimd::select((outsideMask ^ fadeStartMask) & fadeStartMask,
                                 (vFadeStartValue + (dist - vAntialiasingFadeStart) * vAntialiasingFadeCoeff) / vValMax,
                                 dist);
        }
        return (outsideMask | fadeStartMask);
    }

#endif /* defined HAVE_XSIMD */

private:
    qreal m_radius;
    quint8 m_fadeStartValue;
    qreal m_antialiasingFadeStart;
    qreal m_antialiasingFadeCoeff;
    bool m_enableAntialiasing;
    const BaseFade &m_baseFade;
};

template<class BaseFade>
class KisAntialiasingFadeMaker2D
{
public:
    KisAntialiasingFadeMaker2D(const BaseFade &baseFade, bool enableAntialiasing)
        : m_xLimit(0)
        , m_yLimit(0)
        , m_xFadeLimitStart(0)
        , m_yFadeLimitStart(0)
        , m_xFadeCoeff(0)
        , m_yFadeCoeff(0)
        , m_enableAntialiasing(enableAntialiasing)
        , m_baseFade(baseFade)
    {
    }

    KisAntialiasingFadeMaker2D(const KisAntialiasingFadeMaker2D &rhs, const BaseFade &baseFade)
        : m_xLimit(rhs.m_xLimit)
        , m_yLimit(rhs.m_yLimit)
        , m_xFadeLimitStart(rhs.m_xFadeLimitStart)
        , m_yFadeLimitStart(rhs.m_yFadeLimitStart)
        , m_xFadeCoeff(rhs.m_xFadeCoeff)
        , m_yFadeCoeff(rhs.m_yFadeCoeff)
        , m_enableAntialiasing(rhs.m_enableAntialiasing)
        , m_baseFade(baseFade)
    {
    }

    void setLimits(qreal halfWidth, qreal halfHeight)
    {
        m_xLimit = halfWidth;
        m_yLimit = halfHeight;

        m_xFadeLimitStart = m_xLimit - 1.0;
        m_yFadeLimitStart = m_yLimit - 1.0;

        m_xFadeCoeff = 1.0 / (m_xLimit - m_xFadeLimitStart);
        m_yFadeCoeff = 1.0 / (m_yLimit - m_yFadeLimitStart);
    }

    inline bool needFade(qreal x, qreal y, quint8 *value)
    {
        x = qAbs(x);
        y = qAbs(y);

        if (x > m_xLimit) {
            *value = 255;
            return true;
        }

        if (y > m_yLimit) {
            *value = 255;
            return true;
        }

        if (!m_enableAntialiasing) {
            return false;
        }

        if (x > m_xFadeLimitStart) {
            quint8 baseValue = m_baseFade.value(x, y);
            *value = baseValue + (255.0 - baseValue) * (x - m_xFadeLimitStart) * m_xFadeCoeff;

            if (y > m_yFadeLimitStart && *value < 255) {
                *value += (255.0 - *value) * (y - m_yFadeLimitStart) * m_yFadeCoeff;
            }

            return true;
        }

        if (y > m_yFadeLimitStart) {
            quint8 baseValue = m_baseFade.value(x, y);
            *value = baseValue + (255.0 - baseValue) * (y - m_yFadeLimitStart) * m_yFadeCoeff;

            if (x > m_xFadeLimitStart && *value < 255) {
                *value += (255.0 - *value) * (x - m_xFadeLimitStart) * m_xFadeCoeff;
            }

            return true;
        }

        return false;
    }

#if defined(HAVE_XSIMD) && !defined(XSIMD_NO_SUPPORTED_ARCHITECTURE)
    template<typename A>
    xsimd::batch_bool<float, A> needFade(xsimd::batch<float, A> &xr, xsimd::batch<float, A> &yr) const
    {
        using float_v = xsimd::batch<float, A>;
        using float_m = typename float_v::batch_bool_type;

        const float_v vXLimit(m_xLimit);
        const float_v vYLimit(m_yLimit);

        const float_m outXMask = xsimd::abs(xr) > vXLimit;
        const float_m outYMask = xsimd::abs(yr) > vYLimit;

        return (outXMask | outYMask);
    }

    // Apply fader separately to avoid calculating vValue twice.
    template<typename A>
    void apply2DFader(xsimd::batch<float, A> &vValue, xsimd::batch_bool<float, A> &excludeMask, xsimd::batch<float, A> &xr, xsimd::batch<float, A> &yr) const
    {
        using float_v = xsimd::batch<float, A>;
        using float_m = typename float_v::batch_bool_type;

        const float_v vValMax(255.f);

        if (m_enableAntialiasing) {
            const float_v vXFadeLimitStart(m_xFadeLimitStart);
            const float_v vYFadeLimitStart(m_yFadeLimitStart);
            const float_v vXFadeCoeff(m_xFadeCoeff);
            const float_v vYFadeCoeff(m_yFadeCoeff);

            const float_v xra = xsimd::abs(xr);
            float_m fadeXStartMask(false);
            float_m fadeYStartMask(false);

            float_v fadeValue(0);
            const float_v vBaseValue =
                xsimd::truncate_to_type<uint16_t>(vValue);

            fadeXStartMask = xra > vXFadeLimitStart;
            fadeXStartMask = (fadeXStartMask ^ excludeMask) & fadeXStartMask;
            if (!xsimd::all(fadeXStartMask)) {
                fadeValue = vBaseValue + (vValMax - vBaseValue) * (xra - vXFadeLimitStart) * vXFadeCoeff;
                fadeValue = xsimd::select(fadeXStartMask & ((yr > vYFadeLimitStart) & (fadeValue < vValMax)),
                                          fadeValue + (vValMax - fadeValue) * (yr - vYFadeLimitStart) * vYFadeCoeff,
                                          fadeValue);
                vValue = xsimd::select(fadeXStartMask, fadeValue, vValue);
            }

            fadeYStartMask = yr > vYFadeLimitStart;
            fadeYStartMask = (fadeYStartMask ^ fadeXStartMask) & fadeYStartMask;
            if (!xsimd::all(fadeYStartMask)) {
                fadeValue = vBaseValue + (vValMax - vBaseValue) * (yr - vYFadeLimitStart) * vYFadeCoeff;
                fadeValue = xsimd::select(fadeYStartMask & ((xra > vXFadeLimitStart) & (fadeValue < vValMax)),
                                          fadeValue + (vValMax - fadeValue) * (xra - vXFadeLimitStart) * vXFadeCoeff,
                                          fadeValue);
                vValue = xsimd::select(fadeYStartMask, fadeValue, vValue);
            }
        }
    }

#endif /* defined HAVE_XSIMD */

private:
    qreal m_xLimit;
    qreal m_yLimit;

    qreal m_xFadeLimitStart;
    qreal m_yFadeLimitStart;

    qreal m_xFadeCoeff;
    qreal m_yFadeCoeff;

    bool m_enableAntialiasing;

    const BaseFade &m_baseFade;
};

#endif /* __KIS_ANTIALIASING_FADE_MAKER_H */
