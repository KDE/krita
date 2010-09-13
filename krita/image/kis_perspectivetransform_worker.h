/*
 * This file is part of Krita
 *
 *  Copyright (c) 2006 Cyrille Berger <cberger@cberger.net>
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

#ifndef KIS_PERSPECTIVETRANSFORM_WORKER_H
#define KIS_PERSPECTIVETRANSFORM_WORKER_H

#include "kis_types.h"
#include "krita_export.h"
#include <QRect>
#include <QTransform>

class QPointF;
class KoUpdater;

class KRITAIMAGE_EXPORT KisPerspectiveTransformWorker : public QObject
{

    Q_OBJECT

public:
    KisPerspectiveTransformWorker(KisPaintDeviceSP dev, KisSelectionSP selection, const QPointF& topLeft, const QPointF& topRight, const QPointF& bottomLeft, const QPointF& bottomRight, KoUpdater *progress);
    KisPerspectiveTransformWorker(KisPaintDeviceSP dev, QRect r, QPointF center, double aX, double aY, double distance, KoUpdater *progress);

    ~KisPerspectiveTransformWorker();

    void run();
private:
    qint32 m_progressTotalSteps;
    qint32 m_lastProgressReport;
    qint32 m_progressStep;
    double m_xcenter, m_ycenter, m_p, m_q;
    QTransform m_transform;
    KisPaintDeviceSP m_dev;
    KoUpdater *m_progress;
    KisSelectionSP m_selection;
    double m_matrix[3][3];
    QRect m_r;
};

#endif
