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
#include "kis_datamanager.h"
#include <kis_iterator_ng.h>

#include <KoProgressUpdater.h>
#include <KoUpdater.h>
#include <KoColorSpace.h>
#include <KoColor.h>

KisPerspectiveTransformWorker::KisPerspectiveTransformWorker(KisPaintDeviceSP dev, KisSelectionSP selection, const QPointF& topLeft, const QPointF& topRight, const QPointF& bottomLeft, const QPointF& bottomRight, KoUpdaterPtr progress)
        : m_dev(dev), m_progress(progress), m_selection(selection)

{
    if (selection)
        m_r = m_selection->selectedExactRect();
    else {
        KoColor defaultPixel(m_dev->defaultPixel(), m_dev->colorSpace());
        if (defaultPixel.opacityU8() != OPACITY_TRANSPARENT_U8)
            m_r = m_dev->dataManager()->extent();
        else
            m_r = m_dev->exactBounds();
    }

    m_xcenter = 0;
    m_ycenter = 0;
    // below was commented
    /*    if(m_dev->hasSelection())
            m_dev->selection()->clear();*/

    Matrix3qreal b = KisPerspectiveMath::computeMatrixTransfoToPerspective(topLeft, topRight, bottomLeft, bottomRight, m_r);
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            m_matrix[i][j] = b(i, j);
        }
    }
}

KisPerspectiveTransformWorker::KisPerspectiveTransformWorker(KisPaintDeviceSP dev, QRect r, QPointF center, double aX, double aY, double distance, KoUpdaterPtr progress)
        : m_dev(dev), m_progress(progress), m_selection(KisSelectionSP())

{
    QMatrix4x4 m;

    m_r = r;

    m.rotate(180. * aX / M_PI, QVector3D(1, 0, 0));
    m.rotate(180. * aY / M_PI, QVector3D(0, 1, 0));
    m_transform = m.toTransform(distance);
    bool invertible;
    m_transform = m_transform.inverted(&invertible);
    if (!invertible)
        m_transform = QTransform();
    m_xcenter = center.x();
    m_ycenter = center.y();

    m_matrix[0][0] = m_transform.m11();
    m_matrix[0][1] = m_transform.m21();
    m_matrix[0][2] = m_transform.m31();
    m_matrix[1][0] = m_transform.m12();
    m_matrix[1][1] = m_transform.m22();
    m_matrix[1][2] = m_transform.m32();
    m_matrix[2][0] = m_transform.m13();
    m_matrix[2][1] = m_transform.m23();
    m_matrix[2][2] = m_transform.m33();
}

KisPerspectiveTransformWorker::~KisPerspectiveTransformWorker()
{
}

void KisPerspectiveTransformWorker::run()
{
    if (m_r.isNull()) {
        if (!m_progress.isNull()) {
            m_progress->setProgress(100);
        }
        return;
    }

    KisPaintDeviceSP srcdev = new KisPaintDevice(*m_dev.data());
    KisRandomSubAccessorSP srcAcc = srcdev->createRandomSubAccessor();

    // Initialise progress
    m_lastProgressReport = 0;
    m_progressStep = 0;
    m_progressTotalSteps = m_r.width() * m_r.height();

    // Action
    KisRandomAccessorSP accessor = m_dev->createRandomAccessorNG(m_r.x(), m_r.y());

    for (int y = m_r.y(); y < m_r.height(); ++y) {
        for (int x = m_r.x(); x < m_r.width(); ++x) {
            QPointF p = m_transform.map(QPointF(x - m_xcenter, y - m_ycenter));
            double dstX = x - m_xcenter;
            double dstY = y - m_ycenter;
            double sf = (dstX * m_matrix[2][0] + dstY * m_matrix[2][1] + m_matrix[2][2]);
            if (sf != 0) {
                sf = 1.0 / sf;
                p.setX((dstX * m_matrix[0][0] + dstY * m_matrix[0][1] + m_matrix[0][2]) * sf + m_xcenter);
                p.setY((dstX * m_matrix[1][0] + dstY * m_matrix[1][1] + m_matrix[1][2]) * sf + m_ycenter);

                accessor->moveTo(x, y);
                srcAcc->moveTo(p);
                srcAcc->sampledOldRawData(accessor->rawData());
            }

            // TODO: Should set alpha = alpha*(1-selectedness)
            //                 cs->setAlpha( dstIt->rawData(), 255, 1);

            m_progressStep ++;

            if (m_lastProgressReport != (m_progressStep * 100) / m_progressTotalSteps) {
                m_lastProgressReport = (m_progressStep * 100) / m_progressTotalSteps;
                if (!m_progress.isNull()) {
                    m_progress->setProgress(m_lastProgressReport);
                }
            }

            if (!m_progress.isNull() && m_progress->interrupted()) {
                break;
            }

        }
    }
}

#include "kis_perspectivetransform_worker.moc"
