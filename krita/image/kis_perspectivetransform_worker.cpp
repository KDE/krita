/*
 * This file is part of Krita
 *
 *  Copyright (c) 2006 Cyrille Berger <cberger@cberger.net>
 *  Copyright (c) 2009 Edward Apap <schumifer@hotmail.com>
 *  Copyright (c) 2010 Marc Pegon <pe.marc@free.fr>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2 of the License.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "kis_perspectivetransform_worker.h"

#include <QMatrix4x4>
#include <QTransform>
#include <QVector3D>

#include "kis_paint_device.h"
#include "kis_perspective_math.h"
#include "kis_random_accessor_ng.h"
#include "kis_random_sub_accessor.h"
#include "kis_selection.h"
#include <kis_iterator_ng.h>
#include "krita_utils.h"
#include "kis_progress_update_helper.h"

#include <KoProgressUpdater.h>
#include <KoUpdater.h>
#include <KoColorSpace.h>
#include <KoColor.h>

KisPerspectiveTransformWorker::KisPerspectiveTransformWorker(KisPaintDeviceSP dev, QPointF center, double aX, double aY, double distance, KoUpdaterPtr progress)
        : m_dev(dev), m_progressUpdater(progress)

{
    QMatrix4x4 m;
    m.rotate(180. * aX / M_PI, QVector3D(1, 0, 0));
    m.rotate(180. * aY / M_PI, QVector3D(0, 1, 0));

    QTransform project = m.toTransform(distance);
    QTransform t = QTransform::fromTranslate(center.x(), center.y());

    QTransform forwardTransform = t.inverted() * project * t;

    QPolygon bounds(dev->exactBounds());
    QPolygon newBounds = forwardTransform.map(bounds);

    m_isIdentity = forwardTransform.isIdentity();

    if (!m_isIdentity && forwardTransform.isInvertible()) {
        m_newTransform = forwardTransform.inverted();
        m_srcRect = dev->exactBounds();

        QPainterPath path;
        path.addPolygon(newBounds);
        m_dstRegion = KritaUtils::splitPath(path);
    }
}

KisPerspectiveTransformWorker::~KisPerspectiveTransformWorker()
{
}

void KisPerspectiveTransformWorker::run()
{
    KisProgressUpdateHelper progressHelper(m_progressUpdater, 100, m_dstRegion.rectCount());
    if (m_isIdentity) return;


    KisPaintDeviceSP cloneDevice = new KisPaintDevice(*m_dev.data());

    // Clear the destination device, since all the tiles are already
    // shared with cloneDevice
    m_dev->clear();

    KisRandomSubAccessorSP srcAcc = cloneDevice->createRandomSubAccessor();
    KisRandomAccessorSP accessor = m_dev->createRandomAccessorNG(0, 0);

    foreach(const QRect &rect, m_dstRegion.rects()) {
        for (int y = rect.y(); y < rect.y() + rect.height(); ++y) {
            for (int x = rect.x(); x < rect.x() + rect.width(); ++x) {

                QPointF dstPoint(x, y);
                QPointF srcPoint = m_newTransform.map(dstPoint);

                if (m_srcRect.contains(srcPoint)) {
                    accessor->moveTo(dstPoint.x(), dstPoint.y());
                    srcAcc->moveTo(srcPoint.x(), srcPoint.y());
                    srcAcc->sampledOldRawData(accessor->rawData());
                }


            }
        }
        progressHelper.step();
    }
}

#include "kis_perspectivetransform_worker.moc"
