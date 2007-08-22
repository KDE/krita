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

#include <krita_export.h>
#include "kis_paintop_settings.h"
class QWidget;

class QPointF;
class KoColorSpace;
class KoInputDevice;
class KoPointerEvent;

class KisQImagemask;
class KisPainter;
class KisPaintInformation;

/**
 * KisPaintOp are use by tools to draw on a paint device.
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
     */
    virtual void paintAt(const KisPaintInformation& info) = 0;
    void setSource(KisPaintDeviceSP p);

    /**
     * A painterly paintop must have a PainterlyInformation structure,
     * handle the painterly overlays by its own and implement bidirectionality,
     * that is, it will pick up colors from the canvas and change its own color
     * while drawing.
     * @return true if the current paintop is painterly.
     */
    virtual bool painterly() const {return false;}


    /**
     * Draw a line between pos1 and pos2 using the currently set brush and color.
     * If savedDist is less than zero, the brush is painted at pos1 before being
     * painted along the line using the spacing setting.
     * @return the drag distance, that is the remains of the distance between p1 and p2 not covered
     * because the currenlty set brush has a spacing greater than that distance.
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
    virtual bool incremental() { return false; }

protected:

    virtual KisPaintDeviceSP computeDab(KisQImagemaskSP mask);
    virtual KisPaintDeviceSP computeDab(KisQImagemaskSP mask, KoColorSpace *cs);

    /**
     * Split the coordinate into whole + fraction, where fraction is always >= 0.
     */
    virtual void splitCoordinate(double coordinate, qint32 *whole, double *fraction);
    KisPainter* painter();
    KisPaintDeviceSP source();

private:
    Private* const d;
};
/**
 * The paintop factory is responsible for creating paintops of the specified class.
 * If there is an optionWidget, the derived paintop itself must support settings,
 * and it's up to the factory to do that.
 */
class KRITAIMAGE_EXPORT KisPaintOpFactory  : public KisShared
{

public:
    KisPaintOpFactory() {}
    virtual ~KisPaintOpFactory() {}

    /**
     * Create a KisPaintOp with the given settings and painter.
     * @param settings the settings associated with the input device
     * @param painter the painter used to draw
     */
    virtual KisPaintOp * createOp(const KisPaintOpSettings *settings, KisPainter * painter, KisImageSP image) = 0;
    virtual QString id() const = 0;
    virtual QString name() const = 0;

    /**
     * The filename of the pixmap we can use to represent this paintop in the ui.
     */
    virtual QString pixmap();

    /**
     * Whether this paintop is internal to a certain tool or can be used
     * in various tools. If false, it won't show up in the toolchest.
     * The KoColorSpace argument can be used when certain paintops only support a specific cs
     */
    virtual bool userVisible(KoColorSpace * cs = 0);

    /**
     * Create and return an (abstracted) widget with options for this paintop when used with the
     * specified input device. Return 0 if there are no settings available for the given
     * device.
     */
    virtual KisPaintOpSettings* settings(QWidget* parent, const KoInputDevice& inputDevice, KisImageSP image);

};
#endif // KIS_PAINTOP_H_
