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

}

#endif
