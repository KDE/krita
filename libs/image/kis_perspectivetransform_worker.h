/*
 *  Copyright (c) 2006 Cyrille Berger <cberger@cberger.net>
 *  Copyright (c) 2010 Marc Pegon <pe.marc@free.fr>
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
