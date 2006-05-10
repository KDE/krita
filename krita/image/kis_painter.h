/*
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
 *  Copyright (c) 2004 Clarence Dang <dang@kde.org>
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

#ifndef KIS_PAINTER_H_
#define KIS_PAINTER_H_

#include <kcommand.h>

#include "kis_color.h"
#include "kis_global.h"
#include "kis_types.h"
#include "kis_paint_device.h"
#include "kis_point.h"
#include "kis_filter.h"
#include "kis_progress_subject.h"
#include "kis_paintop.h"
#include "kis_color.h"

#include <krita_export.h>

class QRect;
class KisTransaction;
class KisBrush;
class KisPattern;

/**
 * KisPainter contains the graphics primitives necessary to draw on a
 * KisPaintDevice. This is the same kind of abstraction as used in Qt
 * itself, where you have QPainter and QPaintDevice.
 *
 * However, KisPainter works on a tiled image and supports different
 * colour models, and that's a lot more complicated.
 *
 * KisPainter supports transactions that can group various paint operations
 * in one undoable step.
 *
 * For more complex operations, you might want to have a look at the subclasses
 * of KisPainter: KisConvolutionPainter, KisFillPainter and KisGradientPainter
 */
class KRITAIMAGE_EXPORT KisPainter : public KisProgressSubject {
    typedef KisProgressSubject super;

public:
    /// Construct painter without a device
    KisPainter();
    /// Construct a painter, and begin painting on the device
    KisPainter(KisPaintDeviceSP device);
    virtual ~KisPainter();

private:
    // Implement KisProgressSubject
    virtual void cancel() { m_cancelRequested = true; }

public:
    /**
     * Start painting on the specified device. Not undoable.
     */
    void begin(KisPaintDeviceSP device);

    /**
     * Finish painting on the current device
     */
    KCommand *end();

    /// Begin an undoable paint operation
    void beginTransaction(const QString& customName = QString::null);

    /// Finish the undoable paint operation
    KCommand *endTransaction();

    /// begin a transaction with the given command
    void beginTransaction( KisTransaction* command);

    /// Return the current transcation
    KisTransaction  * transaction() { return m_transaction; }


    /// Returns the current paint device.
    KisPaintDeviceSP device() const { return m_device; }


    // -----------------------------------------------------------------
    //  Native paint methods that are undo/redo-able,
    // use the color strategies and the composite operations.

    /**
     * Blast the specified region from src onto the current paint device.
     */
    void bitBlt(qint32 dx, qint32 dy,
                const KisCompositeOp& op,
                KisPaintDeviceSP src,
                qint32 sx, qint32 sy,
                qint32 sw, qint32 sh)
    {
        bitBlt(dx, dy, op, src, OPACITY_OPAQUE, sx, sy, sw, sh);
    }

    /**
     * Overloaded version of the previous, differs in that it is possible to specify
     * a value for opacity
     */
    void bitBlt(qint32 dx, qint32 dy,
                const KisCompositeOp& op,
                KisPaintDeviceSP src,
                quint8 opacity,
                qint32 sx, qint32 sy,
                qint32 sw, qint32 sh);


    /**
     * A version of bitBlt that renders using an external selection mask, ignoring
     * the src device's own selection, if it has one.
     */
    void bltSelection(qint32 dx, qint32 dy,
                      const KisCompositeOp &op,
                      KisPaintDeviceSP src,
                      KisSelectionSP selMask,
                      quint8 opacity,
                      qint32 sx, qint32 sy,
                      qint32 sw, qint32 sh);


    /**
     * A version of bitBlt that renders using the src device's selection mask, if it has one.
     */
    void bltSelection(qint32 dx, qint32 dy,
                      const KisCompositeOp &op,
                      KisPaintDeviceSP src,
                      quint8 opacity,
                      qint32 sx, qint32 sy,
                      qint32 sw, qint32 sh);


    /**
     * The methods below are 'higher' level than the above methods. They need brushes, colors
     * etc. set before they can be called. The methods do not directly tell the image to
     * update, but you can call dirtyRect() to get the rect that needs to be notified by your
     * painting code.
     *
     * Call will RESET the dirtyRect!
    */
    QRect dirtyRect();

    /**
     * Add the r to the current dirty rect, and return the dirtyRect after adding r to it.
     */
    QRect addDirtyRect(QRect r) { m_dirtyRect |= r; return m_dirtyRect; }



    /**
     * Paint a line that connects the dots in points
     */
    void paintPolyline(const Q3ValueVector <KisPoint> &points,
                       int index = 0, int numPoints = -1);

    /**
     * Draw a line between pos1 and pos2 using the currently set brush and color.
     * If savedDist is less than zero, the brush is painted at pos1 before being
     * painted along the line using the spacing setting.
     * @return the drag distance, that is the remains of the distance between p1 and p2 not covered
     * because the currenlty set brush has a spacing greater than that distance.
     */
    double paintLine(const KisPoint &pos1,
             const double pressure1,
             const double xTilt1,
             const double yTilt1,
             const KisPoint &pos2,
             const double pressure2,
             const double xTilt2,
             const double yTilt2,
             const double savedDist = -1);

    /**
     * Draw a Bezier curve between pos1 and pos2 using control points 1 and 2.
     * If savedDist is less than zero, the brush is painted at pos1 before being
     * painted along the curve using the spacing setting.
     * @return the drag distance, that is the remains of the distance between p1 and p2 not covered
     * because the currenlty set brush has a spacing greater than that distance.
     */
    double paintBezierCurve(const KisPoint &pos1,
                const double pressure1,
                const double xTilt1,
                const double yTilt1,
                const KisPoint &control1,
                const KisPoint &control2,
                const KisPoint &pos2,
                const double pressure2,
                const double xTilt2,
                const double yTilt2,
                const double savedDist = -1);

    /**
     * Fill the given vector points with the points needed to draw the Bezier curve between
     * pos1 and pos2 using control points 1 and 2, excluding the final pos2.
     */
    void getBezierCurvePoints(const KisPoint &pos1,
                  const KisPoint &control1,
                  const KisPoint &control2,
                  const KisPoint &pos2,
                  vKisPoint& points);


    /**
     * Paint the rectangle with given begin and end points
     */
    void paintRect(const KisPoint &startPoint,
               const KisPoint &endPoint,
               const double pressure,
               const double xTilt,
               const double yTilt);


    /**
     * Paint the ellipse with given begin and end points
     */
    void paintEllipse(const KisPoint &startPoint,
                      const KisPoint &endPoint,
                      const double pressure,
                      const double /*xTilt*/,
                      const double /*yTilt*/);

    /**
     * Paint the polygon with the points given in points. It automatically closes the polygon
     * by drawing the line from the last point to the first.
     */
    void paintPolygon(const vKisPoint& points);

    /** Draw a spot at pos using the currently set paint op, brush and color */
    void paintAt(const KisPoint &pos,
             const double pressure,
             const double /*xTilt*/,
             const double /*yTilt*/);


    // ------------------------------------------------------------------------
    // Set the parameters for the higher level graphics primitives.

    /// Set the current brush
    void setBrush(KisBrush* brush) { m_brush = brush; }
    /// Returns the currently set brush
    KisBrush * brush() const { return m_brush; }

    /// Set the current pattern
    void setPattern(KisPattern * pattern) { m_pattern = pattern; }
    /// Returns the currently set pattern
    KisPattern * pattern() const { return m_pattern; }

    /// Set the color that will be used to paint with
    void setPaintColor(const KisColor& color) { m_paintColor = color;}

    /// Returns the color that will be used to paint with
    KisColor paintColor() const { return m_paintColor; }

    /// Set the current background color
    void setBackgroundColor(const KisColor& color) {m_backgroundColor = color; }
    /// Returns the current background color
    KisColor backgroundColor() const { return m_backgroundColor; }

    /// Set the current fill color
    void setFillColor(const KisColor& color) { m_fillColor = color; }
    /// Returns the current fill color
    KisColor fillColor() const { return m_fillColor; }


    /// This enum contains the styles with which we can fill things like polygons and ellipses
    enum FillStyle {
        FillStyleNone,
        FillStyleForegroundColor,
        FillStyleBackgroundColor,
        FillStylePattern,
        FillStyleGradient,
        FillStyleStrokes
    };

    /// Set the current style with which to fill
    void setFillStyle(FillStyle fillStyle) { m_fillStyle = fillStyle; }
    /// Returns the current fill style
    FillStyle fillStyle() const { return m_fillStyle; }

    /// The style of the brush stroke around polygons and so
    enum StrokeStyle {
        StrokeStyleNone,
        StrokeStyleBrush
    };

    /// Set the current brush stroke style
    void setStrokeStyle(StrokeStyle strokeStyle) { m_strokeStyle = strokeStyle; }
    /// Returns the current brush stroke style
    StrokeStyle strokeStyle() const { return m_strokeStyle; }

    /// Set the opacity which is used in painting (like filling polygons)
    void setOpacity(quint8 opacity) { m_opacity = opacity; }
    /// Returns the opacity that is used in painting
    quint8 opacity() const { return m_opacity; }

    /**
     * Sets the current composite operation. Everything painted will be composited on
     * the destination layer with this composite op.
     **/
    void setCompositeOp(const KisCompositeOp& op) { m_compositeOp = op; }
    /// Returns the current composite operation
    KisCompositeOp compositeOp() const { return m_compositeOp; }

    /// Sets the current KisFilter, used by the paintops that support it (like KisFilterOp)
    void setFilter(KisFilterSP filter) { m_filter = filter; }
    /// Returns the current KisFilter
    KisFilterSP filter() { return m_filter; }

    /**
     * The offset for paint operations that use it (like KisDuplicateOp). It will use as source
     * the part of the layer that is at its paintedPosition - duplicateOffset
     */
    void setDuplicateOffset(const KisPoint& offset) { m_duplicateOffset = offset; }
    /// Returns the offset for duplication
    KisPoint duplicateOffset(){ return m_duplicateOffset; }

    /// Sets the current pressure for things that like to use this
    void setPressure(double pressure) { m_pressure = pressure; }
    /// Returns the current pressure
    double pressure() { return m_pressure; }

    /**
     * Set the current paint operation. This is used for all drawing functions.
     * The painter will DELETE the paint op itself!!
     * That means no that you should not delete it yourself (or put it on the stack)
     */
    void setPaintOp(KisPaintOp * paintOp) { delete m_paintOp; m_paintOp = paintOp; }
    /// Returns the current paint operation
    KisPaintOp * paintOp() const { return m_paintOp; }

    /// Set a current 'dab'. This usually is a paint device containing a rendered brush
    void setDab(KisPaintDeviceSP dab) { m_dab = dab; }
    /// Get the currently set dab
    KisPaintDeviceSP dab() const { return m_dab; }

    /// Is cancel Requested by the KisProgressSubject for this painter
    bool cancelRequested() const { return m_cancelRequested; }

protected:
    /// Initialize, set everything to '0' or defaults
    void init();
    KisPainter(const KisPainter&);
    KisPainter& operator=(const KisPainter&);

    /// Calculate the distance that point p is from the line made by connecting l0 and l1
    static double pointToLineDistance(const KisPoint& p, const KisPoint& l0, const KisPoint& l1);

    /// Fill the polygon defined by points with the fillStyle
    void fillPolygon(const vKisPoint& points, FillStyle fillStyle);

protected:
    KisPaintDeviceSP m_device;
    KisTransaction  *m_transaction;

    QRect m_dirtyRect;

    KisColor m_paintColor;
    KisColor m_backgroundColor;
    KisColor m_fillColor;
    FillStyle m_fillStyle;
    StrokeStyle m_strokeStyle;
    KisBrush *m_brush;
    KisPattern *m_pattern;
    KisPoint m_duplicateOffset;
    quint8 m_opacity;
    KisCompositeOp m_compositeOp;
    KisFilterSP m_filter;
    KisPaintOp * m_paintOp;
    double m_pressure;
    bool m_cancelRequested;
    qint32 m_pixelSize;
    KisColorSpace * m_colorSpace;
    KisProfile *  m_profile;
    KisPaintDeviceSP m_dab;

};


#endif // KIS_PAINTER_H_

