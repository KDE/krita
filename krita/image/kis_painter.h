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

#include <math.h>

#include <KoColorSpaceConstants.h>

#include "kis_global.h"
#include "kis_types.h"
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
class KisPattern;
class KisFilterConfiguration;
class KisPaintInformation;
class KisPaintOp;
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
 *
 * KisPainter sets a number of default values, like COMPOSITE_OVER for compositeop,
 * OPACITY_OPAQUE for opacity and no selection for selection.
 */
class KRITAIMAGE_EXPORT KisPainter
{


public:
    /// Construct painter without a device
    KisPainter();

    /// Construct a painter, and begin painting on the device
    KisPainter(KisPaintDeviceSP device);

    /// Construct a painter, and begin painting on the device. All actions will be masked by the given selection.
    KisPainter(KisPaintDeviceSP device, KisSelectionSP selection);
    virtual ~KisPainter();

public:
    /**
     * Start painting on the specified device. Not undoable.
     */
    void begin(KisPaintDeviceSP device);

    /**
     * Start painting on the specified paint device. All actions will be masked by the given selection.
     */
    void begin(KisPaintDeviceSP device, KisSelectionSP selection);

    /**
     * Finish painting on the current device
     */
    QUndoCommand *end();

    /**
     * If set, the painter action is cancelable, if the action supports that.
     */
    void setProgress(KoUpdater * progressUpdater);

    /// Begin an undoable paint operation
    void beginTransaction(const QString& customName = "");

    /// Finish the undoable paint operation
    QUndoCommand *endTransaction();

    /// begin a transaction with the given command
    void beginTransaction(KisTransaction* command);

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
     * @param sx the source x-coordinate
     * @param sy the source y-coordinate
     * @param sw the width of the region
     * @param sh the height of the region
     */
    void bitBlt(qint32 dx, qint32 dy,
                const KisPaintDeviceSP src,
                qint32 sx, qint32 sy,
                qint32 sw, qint32 sh);

    /**
     * Convenience method that uses QPoint and QRect
     */
    void bitBlt(const QPoint & pos, const KisPaintDeviceSP src, const QRect & srcRect);

    /**
     * Blast the specified region from src onto the current paint device. Src is a
     * fixed-size paint device: this means that src must have the same colorspace as
     * the destination device.
     *
     * @param dx the destination x-coordinate
     * @param dy the destination y-coordinate
     * @param op a pointer to the composite op use to blast the pixels from src on dst
     * @param src the source device
     * @param sx the source x-coordinate
     * @param sy the source y-coordinate
     * @param sw the width of the region
     * @param sh the height of the region
     */
    void bltFixed(qint32 dx, qint32 dy,
                  const KisFixedPaintDeviceSP src,
                  qint32 sx, qint32 sy,
                  qint32 sw, qint32 sh);

    /**
     * Convenience method that uses QPoint and QRect
     */
    void bltFixed(const QPoint & pos, const KisFixedPaintDeviceSP src, const QRect & srcRect);


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
    void paintRect(const QRectF &rect);

    /**
     * Paint a rectangle.
     *
     * @param x x coordinate of the top-left corner
     * @param y y coordinate of the top-left corner
     * @param w the rectangle width
     * @param h the rectangle height
     */
    void paintRect(const double x,
                   const double y,
                   const double w,
                   const double h);

    /**
     * Paint the ellipse that fills the given rectangle.
     *
     * @param rect the rectangle containing the ellipse to paint.
     */
    void paintEllipse(const QRectF &rect);

    /**
     * Paint the ellipse that fills the given rectangle.
     *
     * @param x x coordinate of the top-left corner
     * @param y y coordinate of the top-left corner
     * @param w the rectangle width
     * @param h the rectangle height
     */
    void paintEllipse(const double x,
                      const double y,
                      const double w,
                      const double h);

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

    /**
     * paint an unstroked one-pixel wide line from specified start position to the
     * specified end position.
     *
     * XXX: this method really should work with subpixel precision for start and end position
     * XXX: this method does not use the composite op
     */
    void drawLine(const QPointF & start, const QPointF & end);

    /**
     * paints an unstroked one-pixel line using the DDA algorithm from specified start position to the
     * specified end position.
     *
     * XXX: this method really should work with subpixel precision for start and end position
     * XXX: this method does not use the composite op
     */
    void drawDDALine(const QPointF & start, const QPointF & end);

    /**
     * Paint an unstroked, wobbly one-pixel wide line from the specified start to the specified
     * end position.
     *
     * XXX: this method really should work with subpixel precision for start and end position
     * XXX: this method does not use the composite op
     */
    void drawWobblyLine(const QPointF & start, const QPointF & end);

    /**
     * Paint an unstroked, one-pixel wide line from the specified start to the specified
     * end position using the Wu algorithm
     *
     * XXX: this method does not use the composite op
     */
    void drawWuLine(const QPointF & start, const QPointF & end);

    /**
     * Paint an unstroked wide line from the specified start to the specified
     * end position with width varying from @param w1 at the start to @param w2 at
     * the end.
     *
     * XXX: the width should be set in doubles, not integers.
     * XXX: this method really should work with subpixel precision for start and end position
     * XXX: this method does not use the composite op
     */
    void drawThickLine(const QPointF & start, const QPointF & end, int startWidth, int endWidth);

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
    void setChannelFlags(QBitArray channelFlags);

    /// @return the channel flags
    QBitArray channelFlags();

    /**
     * Set the paintop preset to use. If @param image is given,
     * the paintop will be created using this image as parameter.
     * Some paintops really want to know about the image they work
     * for, e.g. the duplicate paintop.
     */
    void setPaintOpPreset(KisPaintOpPresetSP preset, KisImageWSP image);

    /// Return the paintop preset
    KisPaintOpPresetSP preset() const;

    /**
     * Return the active paintop (which is created based on the specified preset and
     * will be deleted as soon as the KisPainter instance dies).
     */
    KisPaintOp* paintOp() const;

    /// Set the current pattern
    void setPattern(KisPattern * pattern);

    /// Returns the currently set pattern
    KisPattern * pattern() const;

    /**
     * Set the color that will be used to paint with, and convert it
     * to the color space of the current paint device.
     */
    void setPaintColor(const KoColor& color);

    /// Returns the color that will be used to paint with
    const KoColor &paintColor() const;

    /**
     * Set the current background color, and convert it
     * to the color space of the current paint device.
     */
    void setBackgroundColor(const KoColor& color);

    /// Returns the current background color
    const KoColor &backgroundColor() const;

    /// Set the current fill color
    void setFillColor(const KoColor& color);

    /// Returns the current fill color
    const KoColor &fillColor() const;

    /// Set the current generator (a generator can be used to fill an area
    void setGenerator(KisFilterConfiguration * generator);

    /// @return the current generator configuration
    KisFilterConfiguration * generator() const;

    /// This enum contains the styles with which we can fill things like polygons and ellipses
    enum FillStyle {
        FillStyleNone,
        FillStyleForegroundColor,
        FillStyleBackgroundColor,
        FillStylePattern,
        FillStyleGradient,
        FillStyleStrokes,
        FillStyleGenerator
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

    /// Sets the bounds of the painter area; if not set, the painter
    /// will happily paint where you ask it, making the paint device
    /// larger as it goes
    void setBounds(const QRect & bounds);
    QRect bounds();

    /// Set the composite op for this painter
    void setCompositeOp(const KoCompositeOp * op);
    const KoCompositeOp * compositeOp();

    /// Set the composite op for this painter by string.
    /// Note: the colorspace must be set previously!
    void setCompositeOp(const QString& op);

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

    void setGradient(const KoAbstractGradient* gradient);
    const KoAbstractGradient* gradient();

    /**
    * Set the size of the tile in fillPainterPath, useful when optimizing the use of fillPainterPath
    * e.g. Spray paintop uses more small tiles, although selections uses bigger tiles. QImage::fill
    * is quite expensive so with smaller images you can save instructions
    * Default and maximum size is 256x256 image
    */
    void setMaskImageSize(qint32 width, qint32 height);

    /**
     * If the alpha channel is locked, the alpha values of the paint device we are painting on
     * will not change.
     */
    void setLockAlpha(bool protect);
    bool alphaLocked() const;

protected:
    /// Initialize, set everything to '0' or defaults
    void init();

    /// Fill the polygon defined by points with the fillStyle
    void fillPolygon(const vQPointF& points, FillStyle fillStyle);

private:

    KisPainter(const KisPainter&);
    KisPainter& operator=(const KisPainter&);

    float frac(float value) {
        float tmp = 0;
        return modff(value , &tmp);
    }

    float invertFrac(float value) {
        float tmp = 0;
        return 1.0f - modff(value , &tmp);
    }

protected:
    KoUpdater * progressUpdater();

private:

    struct Private;
    Private* const d;
};


#endif // KIS_PAINTER_H_

