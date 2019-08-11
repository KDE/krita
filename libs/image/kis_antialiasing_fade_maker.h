/*
 *  Copyright (c) 2014 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef __KIS_ANTIALIASING_FADE_MAKER_H
#define __KIS_ANTIALIASING_FADE_MAKER_H

#include "kis_global.h"

template <class BaseFade>
class KisAntialiasingFadeMaker1D
{
public:
    KisAntialiasingFadeMaker1D(const BaseFade &baseFade, bool enableAntialiasing)
        : m_radius(0.0),
          m_fadeStartValue(0),
          m_antialiasingFadeStart(0),
          m_antialiasingFadeCoeff(0),
          m_enableAntialiasing(enableAntialiasing),
          m_baseFade(baseFade)
    {
    }

    KisAntialiasingFadeMaker1D(const KisAntialiasingFadeMaker1D &rhs, const BaseFade &baseFade)
        : m_radius(rhs.m_radius),
          m_fadeStartValue(rhs.m_fadeStartValue),
          m_antialiasingFadeStart(rhs.m_antialiasingFadeStart),
          m_antialiasingFadeCoeff(rhs.m_antialiasingFadeCoeff),
          m_enableAntialiasing(rhs.m_enableAntialiasing),
          m_baseFade(baseFade)
    {
    }

    void setSquareNormCoeffs(qreal xcoeff, qreal ycoeff) {
        m_radius = 1.0;

        qreal xf = qMax(0.0, ((1.0 / xcoeff) - 1.0) * xcoeff);
        qreal yf = qMax(0.0, ((1.0 / ycoeff) - 1.0) * ycoeff);

        m_antialiasingFadeStart = pow2(0.5 * (xf + yf));

        m_fadeStartValue = m_baseFade.value(m_antialiasingFadeStart);
        m_antialiasingFadeCoeff = qMax(0.0, 255.0 - m_fadeStartValue) / (m_radius - m_antialiasingFadeStart);
    }

    void setRadius(qreal radius) {
        m_radius = radius;
        m_antialiasingFadeStart = qMax(0.0, m_radius - 1.0);

        m_fadeStartValue = m_baseFade.value(m_antialiasingFadeStart);
        m_antialiasingFadeCoeff = qMax(0.0, 255.0 - m_fadeStartValue) / (m_radius - m_antialiasingFadeStart);
    }

    inline bool needFade(qreal dist, quint8 *value) {
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

#if defined HAVE_VC
    Vc::float_m needFade(Vc::float_v &dist) {
        const Vc::float_v vOne(Vc::One);
        const Vc::float_v vValMax(255.f);

        Vc::float_v vRadius(m_radius);
        Vc::float_v vFadeStartValue(m_fadeStartValue);
        Vc::float_v vAntialiasingFadeStart(m_antialiasingFadeStart);
        Vc::float_v vAntialiasingFadeCoeff(m_antialiasingFadeCoeff);

        Vc::float_m outsideMask = dist > vRadius;
        dist(outsideMask) = vOne;

        Vc::float_m fadeStartMask(false);

        if(m_enableAntialiasing){
            fadeStartMask = dist > vAntialiasingFadeStart;
            dist((outsideMask ^ fadeStartMask) & fadeStartMask) = (vFadeStartValue +
                                                                (dist - vAntialiasingFadeStart) * vAntialiasingFadeCoeff) / vValMax;
        }
        return (outsideMask | fadeStartMask);
    }

#endif /* defined HAVE_VC */

private:
    qreal m_radius;
    quint8 m_fadeStartValue;
    qreal m_antialiasingFadeStart;
    qreal m_antialiasingFadeCoeff;
    bool m_enableAntialiasing;
    const BaseFade &m_baseFade;
};

template <class BaseFade>
class KisAntialiasingFadeMaker2D
{
public:
    KisAntialiasingFadeMaker2D(const BaseFade &baseFade, bool enableAntialiasing)
        : m_xLimit(0),
          m_yLimit(0),
          m_xFadeLimitStart(0),
          m_yFadeLimitStart(0),
          m_xFadeCoeff(0),
          m_yFadeCoeff(0),
          m_enableAntialiasing(enableAntialiasing),
          m_baseFade(baseFade)
    {
    }

    KisAntialiasingFadeMaker2D(const KisAntialiasingFadeMaker2D &rhs, const BaseFade &baseFade)
        : m_xLimit(rhs.m_xLimit),
          m_yLimit(rhs.m_yLimit),
          m_xFadeLimitStart(rhs.m_xFadeLimitStart),
          m_yFadeLimitStart(rhs.m_yFadeLimitStart),
          m_xFadeCoeff(rhs.m_xFadeCoeff),
          m_yFadeCoeff(rhs.m_yFadeCoeff),
          m_enableAntialiasing(rhs.m_enableAntialiasing),
          m_baseFade(baseFade)
    {
    }

    void setLimits(qreal halfWidth, qreal halfHeight) {
        m_xLimit = halfWidth;
        m_yLimit = halfHeight;

        m_xFadeLimitStart = m_xLimit - 1.0;
        m_yFadeLimitStart = m_yLimit - 1.0;

        m_xFadeCoeff = 1.0 / (m_xLimit - m_xFadeLimitStart);
        m_yFadeCoeff = 1.0 / (m_yLimit - m_yFadeLimitStart);
    }

    inline bool needFade(qreal x, qreal y, quint8 *value) {
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

#if defined HAVE_VC
    Vc::float_m needFade(Vc::float_v &xr, Vc::float_v &yr) const {

        Vc::float_v vXLimit(m_xLimit);
        Vc::float_v vYLimit(m_yLimit);

        Vc::float_m outXMask = Vc::abs(xr) > vXLimit;
        Vc::float_m outYMask = Vc::abs(yr) > vYLimit;

        return (outXMask | outYMask);
    }

    // Apply fader separately to avoid calculating vValue twice.
    void apply2DFader(Vc::float_v &vValue, Vc::float_m &excludeMask, Vc::float_v &xr, Vc::float_v &yr) const {
        const Vc::float_v vValMax(255.f);

        if(m_enableAntialiasing){
            Vc::float_v vXFadeLimitStart(m_xFadeLimitStart);
            Vc::float_v vYFadeLimitStart(m_yFadeLimitStart);
            Vc::float_v vXFadeCoeff(m_xFadeCoeff);
            Vc::float_v vYFadeCoeff(m_yFadeCoeff);

            Vc::float_v xra = abs(xr);
            Vc::float_m fadeXStartMask(false);
            Vc::float_m fadeYStartMask(false);

            Vc::float_v fadeValue;
            Vc::SimdArray<quint16,Vc::float_v::size()> vBaseValue(vValue);

            fadeXStartMask = xra > vXFadeLimitStart;
            fadeXStartMask = (fadeXStartMask ^ excludeMask) & fadeXStartMask;
            if (!fadeXStartMask.isFull()) {
                fadeValue = vBaseValue + (vValMax - vBaseValue) * (xra - vXFadeLimitStart) * vXFadeCoeff;
                fadeValue(fadeXStartMask & ((yr > vYFadeLimitStart) & (fadeValue < vValMax)) ) =
                        fadeValue + (vValMax - fadeValue) * (yr - vYFadeLimitStart) * vYFadeCoeff;
                vValue(fadeXStartMask) = fadeValue;
            }

            fadeYStartMask = yr > vYFadeLimitStart;
            fadeYStartMask = (fadeYStartMask ^ fadeXStartMask) & fadeYStartMask;
            if (!fadeYStartMask.isFull()) {
                fadeValue = vBaseValue + (vValMax - vBaseValue) * (yr - vYFadeLimitStart) * vYFadeCoeff;
                fadeValue(fadeYStartMask & ((xra > vXFadeLimitStart) & (fadeValue < vValMax)) ) =
                        fadeValue + (vValMax - fadeValue) * (xra - vXFadeLimitStart) * vXFadeCoeff;
                vValue(fadeYStartMask) = fadeValue;
            }
        }
        return;
    }

#endif /* defined HAVE_VC */

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
