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

class KisPaintDevice;
class KoUpdater;
class KisFilterStrategy;

class KRITAIMAGE_EXPORT KisTransformWorker {

public:
    KisTransformWorker(KisPaintDeviceSP dev,
                       double  xscale, double  yscale,
                       double  xshear, double  yshear, double rotation,
                       qint32  xtranslate, qint32  ytranslate,
                       KoUpdater *progress,
                       KisFilterStrategy *filter, bool fixBorderAlpha=false);
    ~KisTransformWorker();

public:

    bool run();

private:
    // XXX (BSAR): Why didn't we use the shared-pointer versions of the paint device classes?
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
    KoUpdater *m_progress;
    KisFilterStrategy *m_filter;

    bool m_fixBorderAlpha;
};


inline KisTransformWorker::~KisTransformWorker()
{
}

#endif // KIS_TRANSFORM_VISITOR_H_
