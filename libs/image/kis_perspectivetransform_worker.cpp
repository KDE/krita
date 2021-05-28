/*
 * This file is part of Krita
 *
 *  SPDX-FileCopyrightText: 2006 Cyrille Berger <cberger@cberger.net>
 *  SPDX-FileCopyrightText: 2009 Edward Apap <schumifer@hotmail.com>
 *  SPDX-FileCopyrightText: 2010 Marc Pegon <pe.marc@free.fr>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */


#include "kis_perspectivetransform_worker.h"

#include <QMatrix4x4>
#include <QTransform>
#include <QVector3D>
#include <QPolygonF>

#include <KoUpdater.h>
#include <KoColor.h>
#include <KoCompositeOpRegistry.h>

#include "kis_paint_device.h"
#include "kis_perspective_math.h"
#include "kis_random_accessor_ng.h"
#include "kis_random_sub_accessor.h"
#include "kis_selection.h"
#include <kis_iterator_ng.h>
#include "krita_utils.h"
#include "kis_progress_update_helper.h"
#include "kis_painter.h"
#include "kis_image.h"


KisPerspectiveTransformWorker::KisPerspectiveTransformWorker(KisPaintDeviceSP dev, QPointF center, double aX, double aY, double distance, KoUpdaterPtr progress)
        : m_dev(dev), m_progressUpdater(progress)

{
    QMatrix4x4 m;
    m.rotate(180. * aX / M_PI, QVector3D(1, 0, 0));
    m.rotate(180. * aY / M_PI, QVector3D(0, 1, 0));

    QTransform project = m.toTransform(distance);
    QTransform t = QTransform::fromTranslate(center.x(), center.y());

    QTransform forwardTransform = t.inverted() * project * t;

    init(forwardTransform);
}

KisPerspectiveTransformWorker::KisPerspectiveTransformWorker(KisPaintDeviceSP dev, const QTransform &transform, KoUpdaterPtr progress)
    : m_dev(dev), m_progressUpdater(progress)
{
    init(transform);
}

void KisPerspectiveTransformWorker::fillParams(const QRectF &srcRect,
                                               const QRect &dstBaseClipRect,
                                               KisRegion *dstRegion,
                                               QPolygonF *dstClipPolygon)
{
    QPolygonF bounds = srcRect;
    QPolygonF newBounds = m_forwardTransform.map(bounds);

    newBounds = newBounds.intersected(QRectF(dstBaseClipRect));

    QPainterPath path;
    path.addPolygon(newBounds);
    *dstRegion = KritaUtils::splitPath(path);
    *dstClipPolygon = newBounds;
}

void KisPerspectiveTransformWorker::init(const QTransform &transform)
{
    m_isIdentity = transform.isIdentity();
    m_isTranslating = transform.type() == QTransform::TxTranslate;

    m_forwardTransform = transform;
    m_backwardTransform = transform.inverted();

    if (m_dev) {
        m_srcRect = m_dev->exactBounds();

        QPolygonF dstClipPolygonUnused;

        fillParams(m_srcRect,
                   m_dev->defaultBounds()->bounds(),
                   &m_dstRegion,
                   &dstClipPolygonUnused);
    }
}

KisPerspectiveTransformWorker::~KisPerspectiveTransformWorker()
{
}

void KisPerspectiveTransformWorker::setForwardTransform(const QTransform &transform)
{
    init(transform);
}


struct BilinearWrapper
{
    using SrcAccessorSP = KisRandomSubAccessorSP;

    BilinearWrapper(KisPaintDeviceSP device)
        : m_accessor(device->createRandomSubAccessor())
    {
    }

    void samplePixel(const QPointF &pt, quint8 *dst) {
        m_accessor->moveTo(pt.x(), pt.y());
        m_accessor->sampledOldRawData(dst);
    }

    KisRandomSubAccessorSP m_accessor;
};

struct NearestNeighbourWrapper
{
    using SrcAccessorSP = KisRandomAccessorSP;

    NearestNeighbourWrapper(KisPaintDeviceSP device)
        : m_accessor(device->createRandomConstAccessorNG()),
          m_pixelSize(device->pixelSize())
    {
    }

    void samplePixel(const QPointF &pt, quint8 *dst) {
        m_accessor->moveTo(qRound(pt.x()), qRound(pt.y()));
        memcpy(dst, m_accessor->oldRawData(), m_pixelSize);
    }

    KisRandomConstAccessorSP m_accessor;
    int m_pixelSize;
};

template <class SrcAccessorWrapper>
void KisPerspectiveTransformWorker::runImpl()
{
    KIS_ASSERT_RECOVER_RETURN(m_dev);

    if (m_isIdentity) return;

    // TODO: check if this optimization is possible. The only blocking issue might be if
    //       some other thread also accesses this device (which should not be the case,
    //       theoretically
    //
    // if (m_isTranslating) {
    //     m_dev->moveTo(m_dev->offset() + QPoint(qRound(m_forwardTransform.dx()), qRound(m_forwardTransform.dy())));
    //     return;
    // }

    KisPaintDeviceSP cloneDevice = new KisPaintDevice(*m_dev.data());

    // Clear the destination device, since all the tiles are already
    // shared with cloneDevice
    m_dev->clear();

    KIS_ASSERT_RECOVER_NOOP(!m_isIdentity);

    KisProgressUpdateHelper progressHelper(m_progressUpdater, 100, m_dstRegion.rectCount());

    SrcAccessorWrapper srcAcc(cloneDevice);
    KisRandomAccessorSP accessor = m_dev->createRandomAccessorNG();

    Q_FOREACH (const QRect &rect, m_dstRegion.rects()) {
        for (int y = rect.y(); y < rect.y() + rect.height(); ++y) {
            for (int x = rect.x(); x < rect.x() + rect.width(); ++x) {

                QPointF dstPoint(x, y);
                QPointF srcPoint = m_backwardTransform.map(dstPoint);

                if (m_srcRect.contains(srcPoint)) {
                    accessor->moveTo(dstPoint.x(), dstPoint.y());
                    srcAcc.samplePixel(srcPoint, accessor->rawData());
                }
            }
        }
        progressHelper.step();
    }
}

void KisPerspectiveTransformWorker::run(SampleType sampleType)
{
    if (sampleType == Bilinear) {
        runImpl<BilinearWrapper>();
    } else {
        runImpl<NearestNeighbourWrapper>();
    }
}

void KisPerspectiveTransformWorker::runPartialDst(KisPaintDeviceSP srcDev,
                                                  KisPaintDeviceSP dstDev,
                                                  const QRect &dstRect)
{
    KIS_SAFE_ASSERT_RECOVER_RETURN(srcDev->pixelSize() == dstDev->pixelSize());
    KIS_SAFE_ASSERT_RECOVER_NOOP(*srcDev->colorSpace() == *dstDev->colorSpace());

    QRectF srcClipRect = srcDev->exactBounds() | srcDev->defaultBounds()->imageBorderRect();
    if (srcClipRect.isEmpty()) return;

    if (m_isIdentity || m_isTranslating) {
        KisPainter gc(dstDev);
        gc.setCompositeOp(COMPOSITE_COPY);
        gc.bitBlt(dstRect.topLeft(), srcDev, m_backwardTransform.mapRect(dstRect));
    } else {
        KisProgressUpdateHelper progressHelper(m_progressUpdater, 100, dstRect.height());

        KisRandomSubAccessorSP srcAcc = srcDev->createRandomSubAccessor();
        KisRandomAccessorSP accessor = dstDev->createRandomAccessorNG();

        for (int y = dstRect.y(); y < dstRect.y() + dstRect.height(); ++y) {
            for (int x = dstRect.x(); x < dstRect.x() + dstRect.width(); ++x) {

                QPointF dstPoint(x, y);
                QPointF srcPoint = m_backwardTransform.map(dstPoint);

                if (srcClipRect.contains(srcPoint) || srcDev->defaultBounds()->wrapAroundMode()) {
                    accessor->moveTo(dstPoint.x(), dstPoint.y());
                    srcAcc->moveTo(srcPoint.x(), srcPoint.y());
                    srcAcc->sampledOldRawData(accessor->rawData());
                }
            }
            progressHelper.step();
        }
    }
}

QTransform KisPerspectiveTransformWorker::forwardTransform() const
{
    return m_forwardTransform;
}

QTransform KisPerspectiveTransformWorker::backwardTransform() const
{
    return m_backwardTransform;
}
