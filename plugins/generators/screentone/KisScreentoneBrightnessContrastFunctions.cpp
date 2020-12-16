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

}
