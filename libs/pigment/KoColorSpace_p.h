/*
 *  Copyright (c) 2005 Boudewijn Rempt <boud@valdyas.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#ifndef _KOCOLORSPACE_P_H_
#define _KOCOLORSPACE_P_H_

#include "KoColorSpace.h"

struct KoColorSpace::Private {

    QString id;
    quint32 idNumber;
    QString name;
    QHash<QString, KoCompositeOp*> compositeOps;
    QList<KoChannelInfo *> channels;
    KoMixColorsOp* mixColorsOp;
    KoConvolutionOp* convolutionOp;
    QThreadStorage< QVector<quint8>* > conversionCache;

    mutable KoColorConversionTransformation* transfoToRGBA16;
    mutable KoColorConversionTransformation* transfoFromRGBA16;
    mutable KoColorConversionTransformation* transfoToLABA16;
    mutable KoColorConversionTransformation* transfoFromLABA16;

    Deletability deletability;
};

#endif
