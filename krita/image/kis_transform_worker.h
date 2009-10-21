/*
 *  Copyright (c) 2004 Michael Thaler <michael.thaler@physik.tu-muenchen.de>
 *  Copyright (c) 2005 Casper Boemann <cbr@boemann.dk>
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

#ifndef KIS_TRANSFORM_WORKER_H_
#define KIS_TRANSFORM_WORKER_H_

#include "kis_types.h"
#include "krita_export.h"

#include <QRect>

#include <KoUpdater.h>
typedef QPointer<KoUpdater> KoUpdaterPtr;

class KisPaintDevice;
class KisFilterStrategy;
class KisSelection;

class KRITAIMAGE_EXPORT KisTransformWorker
{

public:
    KisTransformWorker(KisPaintDeviceSP dev,
                       double  xscale, double  yscale,
                       double  xshear, double  yshear, double rotation,
                       qint32  xtranslate, qint32  ytranslate,
                       KoUpdaterPtr progress,
                       KisFilterStrategy *filter, bool fixBorderAlpha = false);
    ~KisTransformWorker();

    /**
     * Mirror the specified device along the X axis
     */
    static QRect mirrorX(KisPaintDeviceSP dev, const KisSelection* selection = 0);

    /**
     * Mirror the specified device along the Y axis
     */
    static QRect mirrorY(KisPaintDeviceSP dev, const KisSelection* selection = 0);


public:

    bool run();

private:
    // XXX (BSAR): Why didn't we use the shared-pointer versions of the paint device classes?
    // CBR: because the template functions used within don't work if it's not true pointers
    template <class T> void transformPass(KisPaintDevice* src,
                                          KisPaintDevice* dst,
                                          double xscale,
                                          double  shear,
                                          qint32 dx,
                                          KisFilterStrategy *filterStrategy, bool fixBorderAlpha);

    void rotateNone(KisPaintDeviceSP src, KisPaintDeviceSP dst);
    void rotateRight90(KisPaintDeviceSP src, KisPaintDeviceSP dst);
    void rotateLeft90(KisPaintDeviceSP src, KisPaintDeviceSP dst);
    void rotate180(KisPaintDeviceSP src, KisPaintDeviceSP dst);

private:
    KisPaintDeviceSP m_dev;
    double  m_xscale, m_yscale;
    double  m_xshear, m_yshear, m_rotation;
    qint32  m_xtranslate, m_ytranslate;
    KoUpdaterPtr m_progressUpdater;
    KisFilterStrategy *m_filter;
    int m_progressTotalSteps;
    int m_progressStep;
    int m_lastProgressReport;

    bool m_fixBorderAlpha;
};

#endif // KIS_TRANSFORM_VISITOR_H_
