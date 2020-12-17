/*
 *  SPDX-FileCopyrightText: 2020 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
