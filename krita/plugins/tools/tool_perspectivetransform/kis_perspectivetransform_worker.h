/*
 * This file is part of Krita
 *
 *  Copyright (c) 2006 Cyrille Berger <cberger@cberger.net>
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
#include <QRect>

class QPointF;
class KoUpdater;

class KisPerspectiveTransformWorker : public QObject
{

    Q_OBJECT

public:
    KisPerspectiveTransformWorker(KisPaintDeviceSP dev, KisSelectionSP selection, const QPointF& topLeft, const QPointF& topRight, const QPointF& bottomLeft, const QPointF& bottomRight, KoUpdater *progress);

    ~KisPerspectiveTransformWorker();

    void run();
private:
    qint32 m_progressTotalSteps;
    qint32 m_lastProgressReport;
    qint32 m_progressStep;
    double m_xcenter, m_ycenter, m_p, m_q;
    KisPaintDeviceSP m_dev;
    KisSelectionSP m_selection;
    KoUpdater *m_progress;
    double m_matrix[3][3];
    QRect m_r;
};

#endif
