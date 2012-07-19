/*
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
 *  Copyright (c) 2004 Clarence Dang <dang@kde.org>
 *  Copyright (c) 2008-2010 Lukáš Tvrdý <lukast.dev@gmail.com>
 *  Copyright (c) 2010 José Luis Vergara Toloza <pentalis@gmail.com>
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

#include <QVector>

#include <KoColorSpaceConstants.h>
#include <KoColorConversionTransformation.h>

#include "kis_distance_information.h"
#include "kis_global.h"
#include "kis_types.h"
#include <krita_export.h>

class QPen;
class KUndo2Command;
class QRect;
class QRectF;
class QStringList;
class QBitArray;
class QPainterPath;

class KoAbstractGradient;
class KoUpdater;
class KoColor;
class KoCompositeOp;

class KisUndoAdapter;
class KisPostExecutionUndoAdapter;
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
    void end();

    /**
     * If set, the painter action is cancelable, if the action supports that.
     */
    void setProgress(KoUpdater * progressUpdater);

    /// Begin an undoable paint operation
    void beginTransaction(const QString& transactionName = "");

    /// Return the transaction's text message
    QString transactionText();

    /// Cancel all the changes made by the painter
    void revertTransaction();

    /// Finish the undoable paint operation
    void endTransaction(KisUndoAdapter *undoAdapter);

    /**
     * Finish transaction and load it to a special adapter for strokes
     */
    void endTransaction(KisPostExecutionUndoAdapter *undoAdapter);

    /**
     * Finish the transaction and delete it's undo information.
     * NOTE: Be careful, because all the previous transactions
     * will become non-undoable after execution of this method.
     */
    void deleteTransaction();

    /// continue a transaction started somewhere else
    void putTransaction(KisTransaction* transaction);

    /// take transaction out of the reach of KisPainter
    KisTransaction* takeTransaction();

    /// Returns the current paint device.
    const KisPaintDeviceSP device() const;
    KisPaintDeviceSP device();



    /**
     * Blast a region of srcWidth @param srcWidth and srcHeight @param srcHeight from @param
     * srcDev onto the current paint device. @param srcX and @param srcY set the x and y
     * positions of the origin top-left corner, @param dstX and @param dstY those of
     * the destination.
     * Any pixel read outside the limits of @param srcDev will return the
     * default pixel, this is a property of \ref KisPaintDevice.
     *
     * @param dstX the destination x-coordinate
     * @param dstY the destination y-coordinate
     * @param srcDev the source device
     * @param srcX the source x-coordinate
     * @param srcY the source y-coordinate
     * @param srcWidth the width of the region to be manipulated
     * @param srcHeight the height of the region to be manipulated
     */
    void bitBlt(qint32 dstX, qint32 dstY,
                const KisPaintDeviceSP srcDev,
                qint32 srcX, qint32 srcY,
                qint32 srcWidth, qint32 srcHeight);

    /**
     * Convenience method that uses QPoint and QRect.
     *
     * @param pos the destination coordinate, it replaces @param dstX and @param dstY.
     * @param srcDev the source device.
     * @param srcRect the rectangle describing the area to blast from @param srcDev into the current paint device.
     * @param srcRect replaces @param srcX, @param srcY, @param srcWidth and @param srcHeight.
     *
     */
    void bitBlt(const QPoint & pos, const KisPaintDeviceSP srcDev, const QRect & srcRect);

    /**
     * The same as @ref bitBlt() but reads data from oldData() part of the device
     *
     * @param dstX the destination x-coordinate
     * @param dstY the destination y-coordinate
     * @param srcDev the source device
     * @param srcX the source x-coordinate
     * @param srcY the source y-coordinate
     * @param srcWidth the width of the region to be manipulated
     * @param srcHeight the height of the region to be manipulated
     */
    void bitBltOldData(qint32 dstX, qint32 dstY,
                       const KisPaintDeviceSP srcDev,
                       qint32 srcX, qint32 srcY,
                       qint32 srcWidth, qint32 srcHeight);

    /**
     * Convenience method that uses QPoint and QRect.
     *
     * @param pos the destination coordinate, it replaces @param dstX and @param dstY.
     * @param srcDev the source device.
     * @param srcRect the rectangle describing the area to blast from @param srcDev into the current paint device.
     * @param srcRect replaces @param srcX, @param srcY, @param srcWidth and @param srcHeight.
     *
     */
    void bitBltOldData(const QPoint & pos, const KisPaintDeviceSP srcDev, const QRect & srcRect);

    /**
     * Blasts a @param selection of srcWidth @param srcWidth and srcHeight @param srcHeight
     * of @param srcDev on the current paint device. There is parameters
     * to control where the area begins in each distinct device, explained below.
     * @param selection can be used as a mask to shape @param srcDev to
     * something interesting in the same step it is rendered to the current
     * paint device. @param selection 's colorspace must be alpha8 (the
     * colorspace for selections/transparency), the rectangle formed by
     * @param selX, @param selY, @param srcWidth and @param srcHeight must not go
     * beyond its limits, and they must be different from zero.
     * @param selection and KisPainter's selection (the user selection) are
     * fused together through the composite operation COMPOSITE_MULT.
     * Any pixel read outside the limits of @param srcDev will return the
     * default pixel, this is a property of \ref KisPaintDevice.
     *
     * @param dstX the destination x-coordinate
     * @param dstY the destination y-coordinate
     * @param srcDev the source device
     * @param selection the custom selection to apply on the source device
     * @param selX the selection x-coordinate
     * @param selY the selection y-coordinate
     * @param srcX the source x-coordinate
     * @param srcY the source y-coordinate
     * @param srcWidth the width of the region to be manipulated
     * @param srcHeight the height of the region to be manipulated
     *
     */
    void bitBltWithFixedSelection(qint32 dstX, qint32 dstY,
                                  const KisPaintDeviceSP srcDev,
                                  const KisFixedPaintDeviceSP selection,
                                  qint32 selX, qint32 selY,
                                  qint32 srcX, qint32 srcY,
                                  quint32 srcWidth, quint32 srcHeight);

    /**
     * Convenience method that assumes @param selX, @param selY, @param srcX and @param srcY are
     * equal to 0. Best used when @param selection and the desired area of @param srcDev have exactly
     * the same dimensions and are specially made for each other.
     *
     * @param dstX the destination x-coordinate
     * @param dstY the destination y-coordinate
     * @param srcDev the source device
     * @param selection the custom selection to apply on the source device
     * @param srcWidth the width of the region to be manipulated
     * @param srcHeight the height of the region to be manipulated
     */
    void bitBltWithFixedSelection(qint32 dstX, qint32 dstY,
                                  const KisPaintDeviceSP srcDev,
                                  const KisFixedPaintDeviceSP selection,
                                  quint32 srcWidth, quint32 srcHeight);

    /**
     * Blast a region of srcWidth @param srcWidth and srcHeight @param srcHeight from @param srcDev onto the current
     * paint device. @param srcX and @param srcY set the x and y positions of the
     * origin top-left corner, @param dstX and @param dstY those of the destination.
     * @param srcDev is a \ref KisFixedPaintDevice: this means that @param srcDev must have the same
     * colorspace as the destination device.
     *
     * @param dstX the destination x-coordinate
     * @param dstY the destination y-coordinate
     * @param srcDev the source device
     * @param srcX the source x-coordinate
     * @param srcY the source y-coordinate
     * @param srcWidth the width of the region to be manipulated
     * @param srcHeight the height of the region to be manipulated
     */
    void bltFixed(qint32 dstX, qint32 dstY,
                  const KisFixedPaintDeviceSP srcDev,
                  qint32 srcX, qint32 srcY,
                  qint32 srcWidth, qint32 srcHeight);

    /**
     * Convenience method that uses QPoint and QRect.
     *
     * @param pos the destination coordinate, it replaces @param dstX and @param dstY.
     * @param srcDev the source device.
     * @param srcRect the rectangle describing the area to blast from @param srcDev into the current paint device.
     * @param srcRect replaces @param srcX, @param srcY, @param srcWidth and @param srcHeight.
     *
     */
    void bltFixed(const QPoint & pos, const KisFixedPaintDeviceSP srcDev, const QRect & srcRect);

    /**
     * Blasts a @param selection of srcWidth @param srcWidth and srcHeight @param srcHeight
     * of @param srcDev on the current paint device. There is parameters to control
     * the top-left corner of the area in each respective paint device (@param dstX,
     * @param dstY, @param srcX, @param srcY).
     * @param selection can be used as a mask to shape @param srcDev to something
     * interesting in the same step it is rendered to the current paint device.
     * @param srcDev is a \ref KisFixedPaintDevice: this means that @param srcDev
     * must have the same colorspace as the destination device.
     * @param selection 's colorspace must be alpha8 (the colorspace for
     * selections/transparency).
     * The rectangle formed by the respective top-left coordinates of each device
     * and @param srcWidth and @param srcHeight must not go beyond their limits, and
     * they must be different from zero.
     * @param selection and KisPainter's selection (the user selection) are
     * fused together through the composite operation COMPOSITE_MULT.
     *
     * @param dstX the destination x-coordinate
     * @param dstY the destination y-coordinate
     * @param srcDev the source device
     * @param selection the selection stored in fixed device
     * @param selX the selection x-coordinate
     * @param selY the selection y-coordinate
     * @param srcX the source x-coordinate
     * @param srcY the source y-coordinate
     * @param srcWidth the width of the region to be manipulated
     * @param srcHeight the height of the region to be manipulated
     */
    void bltFixedWithFixedSelection(qint32 dstX, qint32 dstY,
                                    const KisFixedPaintDeviceSP srcDev,
                                    const KisFixedPaintDeviceSP selection,
                                    qint32 selX, qint32 selY,
                                    qint32 srcX, qint32 srcY,
                                    quint32 srcWidth, quint32 srcHeight);

    /**
     * Convenience method that assumes @param selX, @param selY, @param srcX and @param srcY are
     * equal to 0. Best used when @param selection and @param srcDev have exactly the same
     * dimensions and are specially made for each other.
     *
     * @param dstX the destination x-coordinate
     * @param dstY the destination y-coordinate
     * @param srcDev the source device
     * @param selection the custom selection to apply on the source device
     * @param srcWidth the width of the region to be manipulated
     * @param srcHeight the height of the region to be manipulated
     */
    void bltFixedWithFixedSelection(qint32 dstX, qint32 dstY,
                                    const KisFixedPaintDeviceSP srcDev,
                                    const KisFixedPaintDeviceSP selection,
                                    quint32 srcWidth, quint32 srcHeight);

    /**
     * fills a region of width @param width and height @param height of the current
     * paint device with the color @param color. @param x and @param y set the x and y positions of the
     * origin top-left corner.
     *
     * @param x the destination x-coordinate
     * @param y the destination y-coordinate
     * @param width the width of the region to be manipulated
     * @param height the height of the region to be manipulated
     * @param color the color the area is filled with
     */
    void fill(qint32 x, qint32 y, qint32 width, qint32 height, const KoColor& color);

    /**
     * First you need to setup the painter with setMirrorInformation,
     * then these set of methods provide way to render the devices mirrored
     * according the axisCenter vertically or horizontally or both.
     *
     * @param rc rectangle area covered by dab
     * @param dab this device will be mirrored in-place, it means that it will be changed
     */
    void renderMirrorMask(QRect rc, KisFixedPaintDeviceSP dab);
    void renderMirrorMask(QRect rc, KisFixedPaintDeviceSP dab, KisFixedPaintDeviceSP mask);
    void renderMirrorMask(QRect rc, KisPaintDeviceSP dab);
    void renderMirrorMask(QRect rc, KisPaintDeviceSP dab, KisFixedPaintDeviceSP mask);
    void renderMirrorMask(QRect rc, KisPaintDeviceSP dab, int sx, int sy, KisFixedPaintDeviceSP mask);

    /**
     * Special method for some paintop that needs to know which areas where covered by the dab
     * E.g. experimental (shape) paintop needs to know it to be able to copy appriate regions from
     * internal device to the layer device
     *
     * @param rc rectangle area covered by dab
     * @param dab this device will be mirrored in-place, it means that it will be changed
     * @return vector of rectangular dirty regions of the painter's device
     */
    QVector<QRect> regionsRenderMirrorMask(QRect rc, KisFixedPaintDeviceSP dab);

    /**
      * The methods in this class do not tell the paintdevice to update, but they calculate the
      * dirty area. This method returns this dirty area and resets it.
      */
    QVector<QRect> takeDirtyRegion();

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
    KisDistanceInformation paintLine(const KisPaintInformation &pi1,
                     const KisPaintInformation &pi2,
                     const KisDistanceInformation& savedDist = KisDistanceInformation());

    /**
     * Draw a Bezier curve between pos1 and pos2 using control points 1 and 2.
     * If savedDist is less than zero, the brush is painted at pos1 before being
     * painted along the curve using the spacing setting.
     * @return the drag distance, that is the remains of the distance between p1 and p2 not covered
     * because the currenlty set brush has a spacing greater than that distance.
     */
    KisDistanceInformation paintBezierCurve(const KisPaintInformation &pi1,
                            const QPointF &control1,
                            const QPointF &control2,
                            const KisPaintInformation &pi2,
                            const KisDistanceInformation& savedDist = KisDistanceInformation());
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
    void paintRect(const qreal x,
                   const qreal y,
                   const qreal w,
                   const qreal h);

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
    void paintEllipse(const qreal x,
                      const qreal y,
                      const qreal w,
                      const qreal h);

    /**
     * Paint the polygon with the points given in points. It automatically closes the polygon
     * by drawing the line from the last point to the first.
     */
    void paintPolygon(const vQPointF& points);

    /** Draw a spot at pos using the currently set paint op, brush and color */
    qreal paintAt(const KisPaintInformation &pos);

    /**
     * Stroke the given QPainterPath.
     */
    void paintPainterPath(const QPainterPath& path);

    /**
     * Fills the area enclosed by the given QPainterPath
     */
    void fillPainterPath(const QPainterPath& path);

    /**
     * Draw the path using the Pen
     */
    void drawPainterPath(const QPainterPath& path, const QPen& pen);

    /**
     * paint an unstroked one-pixel wide line from specified start position to the
     * specified end position.
     *
     */
    void drawLine(const QPointF & start, const QPointF & end);

    /**
     * paint an unstroked line with thickness from specified start position to the
     * specified end position. Scanline algorithm is used.
     */
    void drawLine(const QPointF &start, const QPointF &end, qreal width, bool antialias);


    /**
     * paints an unstroked, aliased one-pixel line using the DDA algorithm from specified start position to the
     * specified end position.
     *
     */
    void drawDDALine(const QPointF & start, const QPointF & end);

    /**
     * Paint an unstroked, wobbly one-pixel wide line from the specified start to the specified
     * end position.
     *
     */
    void drawWobblyLine(const QPointF & start, const QPointF & end);

    /**
     * Paint an unstroked, anti-aliased one-pixel wide line from the specified start to the specified
     * end position using the Wu algorithm
     */
    void drawWuLine(const QPointF & start, const QPointF & end);

    /**
     * Paint an unstroked wide line from the specified start to the specified
     * end position with width varying from @param w1 at the start to @param w2 at
     * the end.
     *
     * XXX: the width should be set in doubles, not integers.
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

    void setMirrorInformation(const QPointF &axisCenter, bool mirrorHorizontaly, bool mirrorVerticaly);

    /**
     * copy the mirror information to other painter
     */
    void copyMirrorInformation(KisPainter * painter);

    /// Set the current pattern
    void setPattern(const KisPattern * pattern);

    /// Returns the currently set pattern
    const KisPattern * pattern() const;

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
    void setGenerator(const KisFilterConfiguration * generator);

    /// @return the current generator configuration
    const KisFilterConfiguration * generator() const;

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

    void setFlow(quint8 flow);

    quint8 flow() const;

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
     * Add the r to the current dirty rect.
     */
    void addDirtyRect(const QRect & r);

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
    const KoAbstractGradient* gradient() const;

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


    /**
     * set the rendering intent in case pixels need to be converted before painting
     */
    void setRenderingIntent(KoColorConversionTransformation::Intent intent);

    /**
     * set the conversion flags in case pixels need to be converted before painting
     */
    void setColorConversionFlags(KoColorConversionTransformation::ConversionFlags conversionFlags);

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
    template <bool useOldSrcData>
        void bitBltImpl(qint32 dstX, qint32 dstY,
                        const KisPaintDeviceSP srcDev,
                        qint32 srcX, qint32 srcY,
                        qint32 srcWidth, qint32 srcHeight);

private:

    struct Private;
    Private* const d;
};


#endif // KIS_PAINTER_H_

