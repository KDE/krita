/*
 * KDE. Krita Project.
 *
 * Copyright (c) 2020 Deif Lou <ginoba@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
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
