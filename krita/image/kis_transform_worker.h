/*
 *  Copyright (c) 2004 Michael Thaler <michael.thaler@physik.tu-muenchen.de>
 *  Copyright (c) 2005 C. Boemann <cbo@boemann.dk>
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
class QTransform;

class KRITAIMAGE_EXPORT KisTransformWorker
{

    /*What are xshearOrigin, yshearOrigin :
* let's keep it simple and say we only have horizontal shearing (it's similar with vertical shearing)
* that means we will apply the transformation :
* x' = x + xshear * y and y' = y, where x,y are the old coordinates of the pixels, and x' y' the new coordinates
* that means, the more we go down in the image (y++), the more x' is different from x
* most of the times, we want to shear a part of the image centered at y = y0 != 0.
* i.e. we want x' = x at y = y0
* in that case, it's good to apply instead x' = x + xshear * (y - yshearOrigin), y' = y.
* please note that it's still possible to obtain the same result by copying the part you want to shear at
* in another paintDevice at y = -y0 and use the transformWorker with yshearOrigin = 0.
*/
public:
    KisTransformWorker(KisPaintDeviceSP dev,
                       double  xscale, double  yscale,
                       double  xshear, double  yshear,
                       double  xshearOrigin, double yshearOrigin,
                       double rotation,
                       qint32  xtranslate, qint32  ytranslate,
                       KoUpdaterPtr progress,
                       KisFilterStrategy *filter, bool fixBorderAlpha = false);
    ~KisTransformWorker();

    /**
     * Mirror the specified device along the X axis
     * @param dev device to be mirrored
     * @param axis the axis around which the device will be mirrored, only used if greater zero
     * @param selection optional selection that will be used for the mirror
     */
    static QRect mirrorX(KisPaintDeviceSP dev, qreal axis = -1.0f, const KisSelection* selection = 0);

    /**
     * Mirror the specified device along the Y axis
     * @param dev device to be mirrored
     * @param axis the axis around which the device will be mirrored, only used if greater zero
     * @param selection optional selection that will be used for the mirror
     */
    static QRect mirrorY(KisPaintDeviceSP dev, qreal axis = -1.0f, const KisSelection* selection = 0);


public:

    // returns false if interrupted
    bool run();

    /**
     * Returns a matrix of the transformation executed by the worker.
     * Resulting transformation has the following form (in Qt's matrix
     * notation (all the matrices are trasposed)):
     *
     * transform = TS.inverted() * S * TS * SC * R * T
     *
     * ,where:
     * TS - shear origin transpose
     * S  - shear itself (shearX * shearY)
     * SC - scale
     * R  - rotation (@rotation parameter)
     * T  - transpose (@xtranslate, @ytranslate)
     *
     * WARNING: due to some rounding problems in the worker
     * the work it does does not correspond to the matrix exactly!
     * The result always differs 1-3 pixel. So be careful with it
     * (or fix it)
     */
    QTransform transform() const;

private:
    // XXX (BSAR): Why didn't we use the shared-pointer versions of the paint device classes?
    // CBR: because the template functions used within don't work if it's not true pointers
    template <class T> void transformPass(KisPaintDevice* src,
                                          KisPaintDevice* dst,
                                          double xscale,
                                          double  shear,
                                          qint32 dx,
                                          KisFilterStrategy *filterStrategy, bool fixBorderAlpha);

    friend class KisTransformWorkerTest;

    static QRect rotateNone(KisPaintDeviceSP src, KisPaintDeviceSP dst,
                            QRect boundRect,
                            KoUpdaterPtr progressUpdater,
                            int &lastProgressReport,
                            int &progressTotalSteps,
                            int &progressStep);

    static QRect rotateRight90(KisPaintDeviceSP src, KisPaintDeviceSP dst,
                               QRect boundRect,
                               KoUpdaterPtr progressUpdater,
                               int &lastProgressReport,
                               int &progressTotalSteps,
                               int &progressStep);

    static QRect rotateLeft90(KisPaintDeviceSP src, KisPaintDeviceSP dst,
                              QRect boundRect,
                              KoUpdaterPtr progressUpdater,
                              int &lastProgressReport,
                              int &progressTotalSteps,
                              int &progressStep);

    static QRect rotate180(KisPaintDeviceSP src, KisPaintDeviceSP dst,
                           QRect boundRect,
                           KoUpdaterPtr progressUpdater,
                           int &lastProgressReport,
                           int &progressTotalSteps,
                           int &progressStep);

private:
    KisPaintDeviceSP m_dev;
    double  m_xscale, m_yscale;
    double  m_xshear, m_yshear, m_rotation;
    double  m_xshearOrigin, m_yshearOrigin;
    qint32  m_xtranslate, m_ytranslate;
    KoUpdaterPtr m_progressUpdater;
    KisFilterStrategy *m_filter;
    int m_progressTotalSteps;
    int m_progressStep;
    int m_lastProgressReport;
    QRect m_boundRect;

    bool m_fixBorderAlpha;
};

#endif // KIS_TRANSFORM_VISITOR_H_
