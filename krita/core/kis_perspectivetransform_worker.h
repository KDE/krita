/*
 * This file is part of Krita
 *
 *  Copyright (c) 2006 Cyrille Berger <cberger@cberger.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; version 2 of the License.
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
#include "kis_progress_subject.h"

class KisPoint;
class KisProgressDisplayInterface;

class KisPerspectiveTransformWorker : public KisProgressSubject
{
    public:
        KisPerspectiveTransformWorker(KisPaintDeviceSP dev, const KisPoint& topLeft, const KisPoint& topRight, const KisPoint& bottomLeft, const KisPoint& bottomRight, KisProgressDisplayInterface *progress);
    
        ~KisPerspectiveTransformWorker();
        
        void run();
        bool isCanceled() { return m_cancelRequested; };
    private:
        virtual void cancel() { m_cancelRequested = true; }
    private:
        bool m_cancelRequested;
        Q_INT32 m_progressTotalSteps;
        Q_INT32 m_lastProgressReport;
        Q_INT32 m_progressStep;
        double m_xcenter, m_ycenter, m_p, m_q;
        KisPaintDeviceSP m_dev;
        KisProgressDisplayInterface *m_progress;
        double m_matrix[3][3];
        QRect m_r;
};

#endif
