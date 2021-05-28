/*
 *  SPDX-FileCopyrightText: 2006 Cyrille Berger <cberger@cberger.net>
 *  SPDX-FileCopyrightText: 2010 Marc Pegon <pe.marc@free.fr>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_PERSPECTIVETRANSFORM_WORKER_H
#define KIS_PERSPECTIVETRANSFORM_WORKER_H

#include "kis_types.h"
#include "kritaimage_export.h"

#include <QRect>
#include <KisRegion.h>
#include <QTransform>
#include <KoUpdater.h>


class KRITAIMAGE_EXPORT KisPerspectiveTransformWorker
{
public:
    KisPerspectiveTransformWorker(KisPaintDeviceSP dev, QPointF center, double aX, double aY, double distance, KoUpdaterPtr progress);
    KisPerspectiveTransformWorker(KisPaintDeviceSP dev, const QTransform &transform, KoUpdaterPtr progress);

    ~KisPerspectiveTransformWorker();

    enum SampleType {
        NearestNeighbour = 0,
        Bilinear
    };

    void run(SampleType sampleType = Bilinear);
    void runPartialDst(KisPaintDeviceSP srcDev,
                       KisPaintDeviceSP dstDev,
                       const QRect &dstRect);

    void setForwardTransform(const QTransform &transform);

    QTransform forwardTransform() const;
    QTransform backwardTransform() const;

private:
    void init(const QTransform &transform);

    void fillParams(const QRectF &srcRect,
                    const QRect &dstBaseClipRect,
                    KisRegion *dstRegion,
                    QPolygonF *dstClipPolygon);

    template <class SrcAccessorPolicy>
    void runImpl();

private:
    KisPaintDeviceSP m_dev;
    KoUpdaterPtr m_progressUpdater;
    KisRegion m_dstRegion;
    QRectF m_srcRect;
    QTransform m_backwardTransform;
    QTransform m_forwardTransform;
    bool m_isIdentity;
    bool m_isTranslating;
};

#endif
