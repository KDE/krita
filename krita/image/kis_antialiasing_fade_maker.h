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
