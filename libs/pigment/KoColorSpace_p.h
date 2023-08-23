/*
 *  SPDX-FileCopyrightText: 2005 Boudewijn Rempt <boud@valdyas.org>
 * SPDX-FileCopyrightText: 2021 L. E. Segovia <amy@amyspark.me> *
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef _KOCOLORSPACE_P_H_
#define _KOCOLORSPACE_P_H_

#include "KoColorSpace.h"
#include "KoColorSpaceEngine.h"
#include "KoColorConversionTransformation.h"
#include <QPair>
#include <QThreadStorage>
#include <QPolygonF>

struct Q_DECL_HIDDEN KoColorSpace::Private {

    QString id;
    quint32 idNumber;
    QString name;
    QHash<QString, KoCompositeOp*> compositeOps;
    QList<KoChannelInfo *> channels;
    KoMixColorsOp* mixColorsOp;
    KoConvolutionOp* convolutionOp;
    QHash<QString, QMap<DitherType, KisDitherOp*>> ditherOps;

    mutable QThreadStorage< QVector<quint8>* > conversionCache;
    mutable QThreadStorage< QVector<quint8>* > channelFlagsHomogenizationCache;

    mutable KoColorConversionTransformation* transfoToRGBA16;
    mutable KoColorConversionTransformation* transfoFromRGBA16;
    mutable KoColorConversionTransformation* transfoToLABA16;
    mutable KoColorConversionTransformation* transfoFromLABA16;
    
    QPolygonF gamutXYY;
    QPolygonF TRCXYY;
    QVector <qreal> colorants;
    QVector <qreal> lumaCoefficients;

    KoColorSpaceEngine *iccEngine;

    Deletability deletability;

    QVector<quint8> * threadLocalChannelFlagsHomogenizationCache(quint32 size)
    {
        QVector<quint8> * ba = 0;
        if (!channelFlagsHomogenizationCache.hasLocalData()) {
            ba = new QVector<quint8>(size, '0');
            channelFlagsHomogenizationCache.setLocalData(ba);
        } else {
            ba = channelFlagsHomogenizationCache.localData();
            if ((quint8)ba->size() < size)
                ba->resize(size);
        }
        return ba;
    }
};

#endif
