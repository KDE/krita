/*
 * KDE. Krita Project.
 *
 * SPDX-FileCopyrightText: 2020 Deif Lou <ginoba@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisScreentoneBrightnessContrastFunctions.h"

namespace KisScreentoneBrightnessContrastFunctions {

BrightnessContrast::BrightnessContrast(qreal brightness, qreal contrast)
{
    if (contrast > 0.0) {
        if (qFuzzyCompare(contrast, 1.0)) {
            m_m = 10000.0;
        } else {
            m_m = 1.0 / (1.0 - contrast);
        }
        m_b = -m_m * (contrast / 2.0);
    } else {
        m_m = 1.0 + contrast;
        m_b = -contrast / 2.0;
    }
    m_b += (1.0 - m_b) * brightness;
}

qreal BrightnessContrast::operator()(qreal x) const
{
    return m_m * x + m_b;
}

Threshold::Threshold(qreal threshold)
    : m_threshold(threshold)
{}

qreal Threshold::operator()(qreal x) const
{
    // In the extreme case where the threshold value is 1.0, we need to compare
    // the value with 1.0, otherwise a value of 1.0 with a threshold of 1.0 will
    // produce 1.0 as an output. The effect would be some white dots in an all
    // black image, something not desirable.
    return x < m_threshold ? 0.0 : (qFuzzyCompare(x, 1.0) ? 0.0 : 1.0);
}

}
