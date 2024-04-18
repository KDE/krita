/*
 *  SPDX-FileCopyrightText: 2004 Michael Thaler <michael.thaler@physik.tu-muenchen.de>
 *  SPDX-FileCopyrightText: 2005 C. Boemann <cbo@boemann.dk>
 *  SPDX-FileCopyrightText: 2010 Marc Pegon <pe.marc@free.fr>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_TRANSFORM_WORKER_H_
#define KIS_TRANSFORM_WORKER_H_

#include "kis_types.h"
#include "kritaimage_export.h"

#include <QRect>
#include <KoUpdater.h>

class KisPaintDevice;
class KisFilterStrategy;
class QTransform;

class KRITAIMAGE_EXPORT KisTransformWorker
{

public:
    KisTransformWorker(KisPaintDeviceSP dev,
                       double  xscale, double  yscale,
                       double  xshear, double  yshear,
                       double rotation,
                       qreal xtranslate, qreal ytranslate,
                       KoUpdaterPtr progress,
                       KisFilterStrategy *filter);
    ~KisTransformWorker();


    /**
     * Mirror the specified device along the X or Y axis at the
     * coordinate \p axis.
     */
    static void mirror(KisPaintDeviceSP dev, qreal axis, Qt::Orientation orientation);

    /**
     * Convenience methods for mirror(dev, axis, orientation)
     */
    static void mirrorX(KisPaintDeviceSP dev, qreal axis);
    static void mirrorY(KisPaintDeviceSP dev, qreal axis);

    /**
     * Mirror the device relative to the center of its exactBounds()
     */
    static void mirrorX(KisPaintDeviceSP dev);
    static void mirrorY(KisPaintDeviceSP dev);

    /**
     * Offset the specified device with wrapping around edges of rect specified as QRect(0,0,wrapSize.width, wrapSize.height)*
     * @param device device to be offset
     * @param offsetPosition position where the new origin will be
     * @param wrapRect width and height of the wrap edge, usual scenario is to use canvas width&height
     *
     **/
    static void offset(KisPaintDeviceSP device, const QPoint &offsetPosition, const QRect &wrapRect);


public:

    // returns false if interrupted
    bool run();
    bool runPartial(const QRect &processRect);

    /**
     * Returns a matrix of the transformation executed by the worker.
     * Resulting transformation has the following form (in Qt's matrix
     * notation (all the matrices are transposed)):
     *
     * transform = SC * S * R * T
     *
     * ,where:
     * SC - scale
     * S  - shear itself (shearX * shearY)
     * R  - rotation (@p rotation parameter)
     * T  - translation (@p xtranslate, @p ytranslate)
     *
     * WARNING: due to some rounding problems in the worker
     * the work it does not correspond to the matrix exactly!
     * The result always differs 1-3 pixel. So be careful with it
     * (or fix it)
     */
    QTransform transform() const;

    /**
     * Transforms the outline of the pixel selection (if it is valid)
     */
    void transformPixelSelectionOutline(KisPixelSelectionSP pixelSelection) const;

    bool forceSubPixelTranslation() const;
    void setForceSubPixelTranslation(bool value);

private:
    // XXX (BSAR): Why didn't we use the shared-pointer versions of the paint device classes?
    // CBR: because the template functions used within don't work if it's not true pointers
    template <class T> void transformPass(KisPaintDevice* src,
                                          KisPaintDevice* dst,
                                          double xscale,
                                          double  shear,
                                          double dx,
                                          KisFilterStrategy *filterStrategy,
                                          int portion);

    friend class KisTransformWorkerTest;

    static QRect rotateRight90(KisPaintDeviceSP dev,
                               QRect boundRect,
                               KoUpdaterPtr progressUpdater,
                               int portion);

    static QRect rotateLeft90(KisPaintDeviceSP dev,
                              QRect boundRect,
                              KoUpdaterPtr progressUpdater,
                              int portion);

    static QRect rotate180(KisPaintDeviceSP dev,
                           QRect boundRect,
                           KoUpdaterPtr progressUpdater,
                           int portion);

private:
    KisPaintDeviceSP m_dev;
    double  m_xscale, m_yscale;
    double  m_xshear, m_yshear, m_rotation;
    double  m_xshearOrigin, m_yshearOrigin;
    qreal  m_xtranslate, m_ytranslate;
    KoUpdaterPtr m_progressUpdater;
    KisFilterStrategy *m_filter;
    QRect m_boundRect;
    bool m_forceSubPixelTranslation {false};
};

#endif // KIS_TRANSFORM_VISITOR_H_
