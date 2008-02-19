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

#include <KoColorSpaceConstants.h>

#include "kis_global.h"
#include "kis_types.h"
#include "kis_paint_device.h"

#include <krita_export.h>

class QRegion;
class QUndoCommand;
class QRect;
class QRectF;
class QStringList;
class QBitArray;
class QPainterPath;

class KoAbstractGradient;
class KoUpdater;
class KoColor;
class KoCompositeOp;

class KisTransaction;
class KisBrush;
class KisComplexColor;
class KisPattern;
class KisPaintOp;
class KisPaintInformation;

/**
 * KisPainter contains the graphics primitives necessary to draw on a
 * KisPaintDevice. This is the same kind of abstraction as used in Qt
 * itself, where you have QPainter and QPaintDevice.
 *
 * However, KisPainter works on a tiled image and supports different
 * color models, and that's a lot more complicated.
 *
 * KisPainter supports transactions that can group various paint operations
 * in one undoable step.
 *
 * For more complex operations, you might want to have a look at the subclasses
 * of KisPainter: KisConvolutionPainter, KisFillPainter and KisGradientPainter
 */
class KRITAIMAGE_EXPORT KisPainter {


public:
    /// Construct painter without a device
    KisPainter();

    /// Construct a painter, and begin painting on the device
    KisPainter( KisPaintDeviceSP device );

    /// Construct a painter, and begin painting on the device. All actions will be masked by the given selection.
    KisPainter(KisPaintDeviceSP device, KisSelectionSP selection);
    virtual ~KisPainter();

public:
    /**
     * Start painting on the specified device. Not undoable.
     */
    void begin( KisPaintDeviceSP device );

    /**
     * Start painting on the specified paint device. All actions will be masked by the given selection.
     */
    void begin( KisPaintDeviceSP device, KisSelectionSP selection );

    /**
     * Finish painting on the current device
     */
    QUndoCommand *end();

    /**
     * If set, the painter action is cancelable, if the action supports that.
     */
    void setProgress(KoUpdater * progressUpdater);

    /// Begin an undoable paint operation
    void beginTransaction(const QString& customName = QString::null);

    /// Finish the undoable paint operation
    QUndoCommand *endTransaction();

    /// begin a transaction with the given command
    void beginTransaction( KisTransaction* command);

    /// Return the current transcation
    KisTransaction  * transaction();


    /// Returns the current paint device.
    const KisPaintDeviceSP device() const;
    KisPaintDeviceSP device();

    /**
     * Blast the specified region from src onto the current paint device.
     * @param dx the destination x-coordinate
     * @param dy the destination y-coordinate
     * @param op a pointer to the composite op use to blast the pixels from src on dst
     * @param src the source device
     * @param opacity the opacity of the source pixel
     * @param sx the source x-coordinate
     * @param sy the source y-coordinate
     * @param sw the width of the region
     * @param sh the height of the region
     */
    void bitBlt(qint32 dx, qint32 dy,
                const KoCompositeOp* op,
                const KisPaintDeviceSP src,
                quint8 opacity,
                qint32 sx, qint32 sy,
                qint32 sw, qint32 sh);

    /**
     * Convenience method that uses the opacity and composite op set
     * in the painter. If nothing is set, opaque and OVER are assumed.
     */
    void bitBlt(const QPoint & pos, const KisPaintDeviceSP src, const QRect & srcRect );

    /**
     * Overloaded function of the previous which differs that you can pass the composite op using
     * the name
     * @param dx the destination x-coordinate
     * @param dy the destination y-coordinate
     * @param op the name of composite op use to blast the pixels from src on dst
     * @param src the source device
     * @param opacity the opacity of the source pixel
     * @param sx the source x-coordinate
     * @param sy the source y-coordinate
     * @param sw the width of the region
     * @param sh the height of the region
     */
    void bitBlt(qint32 dx, qint32 dy,
                const QString & op,
                const KisPaintDeviceSP src,
                quint8 opacity,
                qint32 sx, qint32 sy,
                qint32 sw, qint32 sh);
    /**
     * Overloaded version of the previous, differs in that the opacity is forced to OPACITY_OPAQUE
     * @param dx the destination x-coordinate
     * @param dy the destination y-coordinate
     * @param op a pointer to the composite op use to blast the pixels from src on dst
     * @param src the source device
     * @param sx the source x-coordinate
     * @param sy the source y-coordinate
     * @param sw the width of the region
     * @param sh the height of the region
     */
    void bitBlt(qint32 dx, qint32 dy,
                const KoCompositeOp* op,
                const KisPaintDeviceSP src,
                qint32 sx, qint32 sy,
                qint32 sw, qint32 sh);

    /**
     * Overloaded function of the previous which differs that you can pass the composite op using
     * the name and the opacity is forced to OPACITY_OPAQUE
     * @param dx the destination x-coordinate
     * @param dy the destination y-coordinate
     * @param op the name of composite op use to blast the pixels from src on dst
     * @param src the source device
     * @param sx the source x-coordinate
     * @param sy the source y-coordinate
     * @param sw the width of the region
     * @param sh the height of the region
     */
    inline void bitBlt(qint32 dx, qint32 dy,
                const QString & op,
                const KisPaintDeviceSP src,
                qint32 sx, qint32 sy,
                qint32 sw, qint32 sh)
    {
        bitBlt(dx, dy, op, src, OPACITY_OPAQUE, sx, sy, sw, sh);
    }

    /**
     * Overloaded function of the previous that take a KisSelection instead of a KisPaintDevice.
     * @param dx the destination x-coordinate
     * @param dy the destination y-coordinate
     * @param op a pointer to the composite op use to blast the pixels from src on dst
     * @param src the source device
     * @param selMask the mask
     * @param opacity the opacity of the source pixel
     * @param sx the source x-coordinate
     * @param sy the source y-coordinate
     * @param sw the width of the region
     * @param sh the height of the region
     */
    void bltSelection(qint32 dx, qint32 dy,
                      const KoCompositeOp  *op,
                      const KisPaintDeviceSP src,
                      const KisSelectionSP seldev,
                      quint8 opacity,
                      qint32 sx, qint32 sy,
                      qint32 sw, qint32 sh);

    /**
     * Convenience method that uses the opacity and composite op set
     * in the painter. If noting is set, opaque and OVER are assumed.
     */
    void bltSelection(const QPoint & pos, const KisPaintDeviceSP src, const KisSelectionSP selDev, const QRect & srcRect );

    /**
     * Overloaded function of the previous that takes a KisSelection
     * instead of a KisPaintDevice and you can pass the composite op
     * using the name.
     *
     * @param dx the destination x-coordinate
     * @param dy the destination y-coordinate
     * @param op the name of the composite op use to blast the pixels from src on dst
     * @param src the source device
     * @param selMask the mask
     * @param opacity the opacity of the source pixel
     * @param sx the source x-coordinate
     * @param sy the source y-coordinate
     * @param sw the width of the region
     * @param sh the height of the region
     */
    void bltSelection(qint32 dx, qint32 dy,
                      const QString & op,
                      const KisPaintDeviceSP src,
                      const KisSelectionSP selMask,
                      quint8 opacity,
                      qint32 sx, qint32 sy,
                      qint32 sw, qint32 sh);

    /**
     * A version of bitBlt that renders using the src device's selection mask, if it has one.
     * @param dx the destination x-coordinate
     * @param dy the destination y-coordinate
     * @param op a pointer to the composite op use to blast the pixels from src on dst
     * @param src the source device
     * @param opacity the opacity of the source pixel
     * @param sx the source x-coordinate
     * @param sy the source y-coordinate
     * @param sw the width of the region
     * @param sh the height of the region
     */
    void bltSelection(qint32 dx, qint32 dy,
                      const KoCompositeOp *op,
                      const KisPaintDeviceSP srcdev,
                      quint8 opacity,
                      qint32 sx, qint32 sy,
                      qint32 sw, qint32 sh);

    /**
     * Overloaded function of the previous that takes the name of the composite op using the name
     * @param dx the destination x-coordinate
     * @param dy the destination y-coordinate
     * @param op the name of the composite op use to blast the pixels from src on dst
     * @param src the source device
     * @param opacity the opacity of the source pixel
     * @param sx the source x-coordinate
     * @param sy the source y-coordinate
     * @param sw the width of the region
     * @param sh the height of the region
     */
    void bltSelection(qint32 dx, qint32 dy,
                      const QString & op,
                      const KisPaintDeviceSP src,
                      quint8 opacity,
                      qint32 sx, qint32 sy,
                      qint32 sw, qint32 sh);

    /**
     * The methods below are 'higher' level than the above methods. They need brushes, colors
     * etc. set before they can be called. The methods do not directly tell the image to
     * update, but you can call dirtyRegion() to get the region that needs to be notified by your
     * painting code.
     *
     * Call will RESET the dirtyRegion!
    */
    QRegion dirtyRegion();

    /**
     * Paint a line that connects the dots in points
     */
    void paintPolyline(const QVector <QPointF> &points,
                       int index = 0, int numPoints = -1);

    /**
     * Draw a line between pos1 and pos2 using the currently set brush and color.
     * If savedDist is less than zero, the brush is painted at pos1 before being
     * painted along the line using the spacing setting.
     * @return the drag distance, that is the remains of the distance between p1 and p2 not covered
     * because the currenlty set brush has a spacing greater than that distance.
     */
    double paintLine(const KisPaintInformation &pi1,
                     const KisPaintInformation &pi2,
                     double savedDist = -1);

    /**
     * Draw a Bezier curve between pos1 and pos2 using control points 1 and 2.
     * If savedDist is less than zero, the brush is painted at pos1 before being
     * painted along the curve using the spacing setting.
     * @return the drag distance, that is the remains of the distance between p1 and p2 not covered
     * because the currenlty set brush has a spacing greater than that distance.
     */
    double paintBezierCurve(const KisPaintInformation &pi1,
                const QPointF &control1,
                const QPointF &control2,
                const KisPaintInformation &pi2,
                const double savedDist = -1);
    /**
     * Fill the given vector points with the points needed to draw the Bezier curve between
     * pos1 and pos2 using control points 1 and 2, excluding the final pos2.
     */
    void getBezierCurvePoints(const QPointF &pos1,
                const QPointF &control1,
                const QPointF &control2,
                const QPointF &pos2,
                vQPointF& points) const;

    /**
     * Paint a rectangle.
     * @param rect the rectangle to paint.
     */
    void paintRect(const QRectF &rect,
                   const double pressure,
                   const double xTilt,
                   const double yTilt);

    /**
    * Paint a rectangle.
    * @param x x coordinate of the top-left corner
    * @param y y coordinate of the top-left corner
    * @param w the rectangle width
    * @param h the rectangle height
    */
    void paintRect(const double x,
                   const double y,
                   const double w,
                   const double h,
                   const double pressure,
                   const double xTilt,
                   const double yTilt);

    /**
     * Paint the ellipse that fills the given rectangle.
     * @param rect the rectangle containing the ellipse to paint.
     */
    void paintEllipse(const QRectF &rect,
                      const double pressure,
                      const double xTilt,
                      const double yTilt);

    /**
    * Paint the ellipse that fills the given rectangle.
    * @param x x coordinate of the top-left corner
    * @param y y coordinate of the top-left corner
    * @param w the rectangle width
    * @param h the rectangle height
    */
    void paintEllipse(const double x,
                      const double y,
                      const double w,
                      const double h,
                      const double pressure,
                      const double xTilt,
                      const double yTilt);

    /**
     * Paint the polygon with the points given in points. It automatically closes the polygon
     * by drawing the line from the last point to the first.
     */
    void paintPolygon(const vQPointF& points);

    /** Draw a spot at pos using the currently set paint op, brush and color */
    void paintAt(const KisPaintInformation &pos);

    /**
     * Stroke the given QPainterPath.
     */
    void paintPainterPath(const QPainterPath& path);

    /**
     * Fills the area enclosed by the given QPainterPath
     */
    void fillPainterPath(const QPainterPath& path);
    // ------------------------------------------------------------------------
    // Set the parameters for the higher level graphics primitives.

    /**
     * Set the channelflags: a bit array where true means that the
     * channel corresponding in position with the bit will be read
     * by the operation, and false means that it will not be affected.
     *
     * An empty channelFlags parameter means that all channels are
     * affected.
     *
     * @param the bit array that masks the source channels; only
     * the channels where the corresponding bit is true will will be
     * composited onto the destination device.
     */
    void setChannelFlags( QBitArray channelFlags );


    QBitArray channelFlags();

    // Set the current brush
    void setBrush(KisBrush* brush);
    // Returns the currently set brush
    KisBrush * brush() const;

    // Set the current pattern
    void setPattern(KisPattern * pattern);
    // Returns the currently set pattern
    KisPattern * pattern() const;

    // Set the color that will be used to paint with
    void setPaintColor(const KoColor& color);
    /// Returns the color that will be used to paint with
    KoColor paintColor() const;

    // Set the current background color
    void setBackgroundColor(const KoColor& color);
    // Returns the current background color
    KoColor backgroundColor() const;

    // Set the current fill color
    void setFillColor(const KoColor& color);
    // Returns the current fill color
    KoColor fillColor() const;

    // XXX
    void setComplexColor(KisComplexColor *color);
    KisComplexColor *complexColor() ;

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
    void setFillStyle(FillStyle fillStyle);

    /// Returns the current fill style
    FillStyle fillStyle() const;

    /// Set whether a polygon's filled area should be anti-aliased or not. The default is true.
    void setAntiAliasPolygonFill(bool antiAliasPolygonFill);

    /// Return whether a polygon's filled area should be anti-aliased or not
    bool antiAliasPolygonFill();

    /// The style of the brush stroke around polygons and so
    enum StrokeStyle {
        StrokeStyleNone,
        StrokeStyleBrush
    };

    /// Set the current brush stroke style
    void setStrokeStyle(StrokeStyle strokeStyle);
    /// Returns the current brush stroke style
    StrokeStyle strokeStyle() const;

    /// Set the opacity which is used in painting (like filling polygons)
    void setOpacity(quint8 opacity);
    /// Returns the opacity that is used in painting
    quint8 opacity() const;

    /// Sets the current pressure for things that like to use this
    void setPressure(double pressure);
    /// Returns the current pressure
    double pressure();

    /// Sets the bounds of the painter area; if not set, the painter
    /// will happily paint where you ask it, making the paint device
    /// larger as it goes
    void setBounds( const QRect & bounds );
    QRect bounds();

    /**
     * Set the current paint operation. This is used for all drawing functions.
     * The painter will DELETE the paint op itself!!
     * That means no that you should not delete it yourself (or put it on the stack)
     */
    void setPaintOp(KisPaintOp * paintOp);
    /// Returns the current paint operation
    KisPaintOp * paintOp() const;

    /// Set a current 'dab'. This usually is a paint device containing a rendered brush
    void setDab(KisPaintDeviceSP dab);
    /// Get the currently set dab
    KisPaintDeviceSP dab() const;

    /// Set the composite op for this painter
    void setCompositeOp(const KoCompositeOp * op);
    const KoCompositeOp * compositeOp();

    /// Calculate the distance that point p is from the line made by connecting l0 and l1
    static double pointToLineDistance(const QPointF& p, const QPointF& l0, const QPointF& l1);

    /**
     * Add the r to the current dirty rect, and return the dirtyRegion after adding r to it.
     */
    QRegion addDirtyRect(const QRect & r);

    /**
     * Reset the selection to the given selection. All painter actions will be
     * masked by the specified selection.
     */
    void setSelection(KisSelectionSP selection);

    /**
     * @return the selection set on this painter.
     */
    KisSelectionSP selection();

    void setGradient(KoAbstractGradient* gradient);
    KoAbstractGradient* gradient();


protected:
    /// Initialize, set everything to '0' or defaults
    void init();

    /// Fill the polygon defined by points with the fillStyle
    void fillPolygon(const vQPointF& points, FillStyle fillStyle);

private:

    KisPainter(const KisPainter&);
    KisPainter& operator=(const KisPainter&);

protected:
    KoUpdater * progressUpdater();
private:

    struct Private;
    Private* const d;
};


#endif // KIS_PAINTER_H_

