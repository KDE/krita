/*
 * KDE. Krita Project.
 *
 * SPDX-FileCopyrightText: 2020 Deif Lou <ginoba@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISSCREENTONEBRIGHTNESCONTRASTFUNCTIONS_H
#define KISSCREENTONEBRIGHTNESCONTRASTFUNCTIONS_H

#include <QtGlobal>

namespace KisScreentoneBrightnessContrastFunctions {

class Identity
{
public:
    inline qreal operator()(qreal x) const
    {
        return x;
    }
};

class BrightnessContrast
{
public:
    // brightness and contrast expected to be in the range [-1, 1]
    BrightnessContrast(qreal brightness, qreal contrast);
    qreal operator()(qreal x) const;
private:
    qreal m_m, m_b;
};

class Threshold
{
public:
    // threshold value expected to be in the range [0, 1]
    Threshold(qreal threshold);
    qreal operator()(qreal x) const;
private:
    const qreal m_threshold;
    const bool m_thresholdIsOne;
};

}

#endif
