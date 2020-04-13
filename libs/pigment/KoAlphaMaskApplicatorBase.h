/*
 *  Copyright (c) 2020 Dmitry Kazakov <dimula73@gmail.com>
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


#ifndef KOALPHAMASKAPPLICATORBASE_H
#define KOALPHAMASKAPPLICATORBASE_H

#include "kritapigment_export.h"
#include <QtGlobal>
#include <QColor>


class KRITAPIGMENT_EXPORT KoAlphaMaskApplicatorBase
{
public:
    virtual ~KoAlphaMaskApplicatorBase();
    virtual void applyInverseNormedFloatMask(quint8 * pixels, const float * alpha, qint32 nPixels) const = 0;
    virtual void fillInverseAlphaNormedFloatMaskWithColor(quint8 * pixels,
                                                          const float * alpha,
                                                          const quint8 *brushColor,
                                                          qint32 nPixels) const = 0;
    virtual void fillGrayBrushWithColor(quint8 *dst, const QRgb *brush, quint8 *brushColor, qint32 nPixels) const = 0;


};

#endif // KOALPHAMASKAPPLICATORBASE_H
