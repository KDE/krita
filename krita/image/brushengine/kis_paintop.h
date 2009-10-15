/*
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
 *  Copyright (c) 2004 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (c) 2004 Clarence Dang <dang@kde.org>
 *  Copyright (c) 2004 Adrian Page <adrian@pagenet.plus.com>
 *  Copyright (c) 2004 Cyrille Berger <cberger@cberger.net>
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

#ifndef KIS_PAINTOP_H_
#define KIS_PAINTOP_H_

#include "kis_shared.h"
#include "kis_types.h"
#include "kis_paintop_settings.h"

#include <krita_export.h>

class QPointF;
class KoColorSpace;
class KoInputDevice;
class KoPointerEvent;

class KisPainter;
class KisPaintInformation;

/**
 * KisPaintOp are use by tools to draw on a paint device. A paintop takes settings
 * and input information, like pressure, tilt or motion and uses that to draw pixels
 */
class KRITAIMAGE_EXPORT KisPaintOp : public KisShared
{
    struct Private;
public:

    KisPaintOp(KisPainter * painter);
    virtual ~KisPaintOp();

    /**
     * Paint at the subpixel point pos using the specified paint
     * information..
     *
     * The distance between two calls of the paintAt is always specified by spacing;
     * xSpacing and ySpacing is 1.0 by default, negative values causes infinite loops (it is checked by Q_ASSERT)
     */
    virtual void paintAt(const KisPaintInformation& info) = 0;

    /**
     * A painterly paintop must have a PainterlyInformation structure,
     * handle the painterly overlays by its own and implement bidirectionality,
     * that is, it will pick up colors from the canvas and change its own color
     * while drawing.
     * @return true if the current paintop is painterly.
     */
    virtual bool painterly() const {
        return false;
    }

    /**
     * Draw a line between pos1 and pos2 using the currently set brush and color.
     * If savedDist is less than zero, the brush is painted at pos1 before being
     * painted along the line using the spacing setting.
     *
     * @return the drag distance, that is the remains of the distance
     * between p1 and p2 not covered because the currenlty set brush
     * has a spacing greater than that distance.
     */
    virtual double paintLine(const KisPaintInformation &pi1,
                             const KisPaintInformation &pi2,
                             double savedDist = -1);

    /**
     * Draw a Bezier curve between pos1 and pos2 using control points 1 and 2.
     * If savedDist is less than zero, the brush is painted at pos1 before being
     * painted along the curve using the spacing setting.
     * @return the drag distance, that is the remains of the distance between p1 and p2 not covered
     * because the currenlty set brush has a spacing greater than that distance.
     */
    virtual double paintBezierCurve(const KisPaintInformation &pi1,
                                    const QPointF &control1,
                                    const QPointF &control2,
                                    const KisPaintInformation &pi2,
                                    const double savedDist = -1);

    /**
     * Whether this paintop wants to deposit paint even when not moving, i.e. the
     * tool needs to activate its timer.
     */
    virtual bool incremental() const {
        return false;
    }

protected:

    static double scaleForPressure(double pressure);

    KisFixedPaintDeviceSP cachedDab();
    KisFixedPaintDeviceSP cachedDab(const KoColorSpace *cs);

    /**
     * Split the coordinate into whole + fraction, where fraction is always >= 0.
     */
    virtual void splitCoordinate(double coordinate, qint32 *whole, double *fraction)const ;

    /**
     * Determine the x and y spacing between two calls to paintAt.
     * @return the spacing
     */
    virtual double spacing(double & xSpacing, double & ySpacing, double pressure1, double pressure2) const = 0;

    /**
     * Return the painter this paintop is owned by
     */
    KisPainter* painter() const;

    /**
     * Return the paintdevice the painter this paintop is owned by
     */
    KisPaintDeviceSP source() const;

private:
    Private* const d;
};


#endif // KIS_PAINTOP_H_
