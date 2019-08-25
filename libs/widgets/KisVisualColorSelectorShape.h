/*
 * Copyright (C) Wolthera van Hovell tot Westerflier <griffinvalley@gmail.com>, (C) 2016
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
#ifndef KIS_VISUAL_COLOR_SELECTOR_SHAPE_H
#define KIS_VISUAL_COLOR_SELECTOR_SHAPE_H

#include <QWidget>
#include <QScopedPointer>
#include <QPixmap>
#include <QRegion>
#include <QMouseEvent>

#include <KoColor.h>
#include <KoColorSpace.h>
#include "KoColorDisplayRendererInterface.h"

#include "KisVisualColorSelector.h"
#include "KisColorSelectorConfiguration.h"

/**
 * @brief The KisVisualColorSelectorShape class
 * A 2d widget can represent at maximum 2 coordinates.
 * So first decide howmany coordinates you need. (onedimensional, or twodimensional)
 * Then the model, (Channel, HSV, HSL, HSI, YUV). Channel is the raw color channels.
 * When it finds a non-implemented feature it'll return to Channel.
 * Then, select the channels you wish to be affected. This uses the model, so for cmyk
 * the channel is c=0, m=1, y=2, k=3, but for hsv, hue=0, sat=1, and val=2
 * These can also be set with 'slotsetactive channels'.
 * Then finally, connect the displayrenderer, you can also do this with 'setdisplayrenderer'
 *
 * Either way, this class is made to be subclassed, with a few virtuals so that the geometry
 * can be calculated properly.
 */

class KisVisualColorSelectorShape : public QWidget
{
    Q_OBJECT
public:
    /**
     * @brief The Dimensions enum
     * Whether or not the shape is single or two dimensional.
     **/
    enum Dimensions{onedimensional, twodimensional};
    enum ColorModel{Channel, HSV, HSL, HSI, HSY, YUV};
    explicit KisVisualColorSelectorShape(QWidget *parent,
                                         KisVisualColorSelectorShape::Dimensions dimension,
                                         const KoColorSpace *cs,
                                         int channel1, int channel2,
                                         const KoColorDisplayRendererInterface *displayRenderer = KoDumbColorDisplayRenderer::instance());
    ~KisVisualColorSelectorShape() override;

    /**
     * @brief getCursorPosition
     * @return current cursor position in shape-coordinates.
     */
    QPointF getCursorPosition();
    /**
     * @brief getDimensions
     * @return whether this is a single or twodimensional widget.
     */
    Dimensions getDimensions() const;
    /**
     * @brief getPixmap
     * @return the pixmap of the gradient, for drawing on with a subclass.
     * the pixmap will not change unless 'm_d->setPixmap=true' which is toggled by
     * refresh and update functions.
     */
    bool imagesNeedUpdate() const;
    QImage getImageMap();
    /**
     * @brief setFullImage
     * Set the full widget image to be painted.
     * @param full this should be the full image.
     */
    void setFullImage(QImage full);
    /**
     * @brief getCurrentColor
     * @return the current kocolor
     */
    KoColor getCurrentColor();
    /**
     * @brief setDisplayRenderer
     * disconnect the old display renderer if needed and connect the new one.
     * @param displayRenderer
     */
    void setDisplayRenderer (const KoColorDisplayRendererInterface *displayRenderer);
    /**
     * @brief getColorFromConverter
     * @param c a koColor.
     * @return get the qcolor from the given kocolorusing this widget's display renderer.
     */
    QColor getColorFromConverter(KoColor c);

    /**
     * @brief getSpaceForSquare
     * @param geom the full widget rectangle
     * @return rectangle with enough space for second widget
     */
    virtual QRect getSpaceForSquare(QRect geom) = 0;
    virtual QRect getSpaceForCircle(QRect geom) = 0;
    virtual QRect getSpaceForTriangle(QRect geom) = 0;

    /**
     * @brief forceImageUpdate
     * force the image to recache.
     */
    void forceImageUpdate();

    /**
     * @brief setBorderWidth
     * set the border of the single dimensional selector.
     * @param width
     */
    virtual void setBorderWidth(int width) = 0;

    /**
     * @brief getChannels
     * get used channels
     * @return
     */
    QVector <int> getChannels() const;

    /**
      * @brief setCursorPosition
      * Set the cursor to normalized shape coordinates. This will only repaint the cursor.
      * @param position normalized shape coordinates ([0,1] range, not yet transformed to actual channel values!)
      * @param signal if true, emit a sigCursorMoved signal
      */
    void setCursorPosition(QPointF position, bool signal = false);

    /**
      * @brief setChannelValues
      * Set the current channel values;
      * Note that channel values controlled by the shape itself have no effect unless setCursor is true.
      * This will trigger a full widget repaint.
      * @param position normalized shape coordinates ([0,1] range)
      * these are not yet transformed to color space specific ranges!
      * @param setCursor if true, sets the cursor too, otherwise the shape-controlled channels are not set
      */
    void setChannelValues(QVector4D channelValues, bool setCursor);


Q_SIGNALS:
    void sigCursorMoved(QPointF pos);

public Q_SLOTS:
    /**
     * @brief slotSetActiveChannels
     * Change the active channels if necessary.
     * @param channel1 used by single and twodimensional widgets.
     * @param channel2 only used by twodimensional widgets.
     */
    void slotSetActiveChannels(int channel1, int channel2);
    /**
     * @brief updateFromChangedDisplayRenderer
     * for updating from the display renderer... not sure why this one is public.
     */
    void updateFromChangedDisplayRenderer();

protected:
    /**
     * @brief convertImageMap
     * convert image data containing raw KoColor data into a QImage
     * @param data must point to memory of size width()*height()*pixelSize
     * @param size the number of bytes to read from data, must match aforementioned cirteria
     * @return the converted QImage guaranteed to match the widget size (black content on failure)
     */
    QImage convertImageMap(const quint8 *rawColor, quint32 size) const;
    /**
     * @brief renderBackground
     * Render the widget background visible inside the widget's mask in current color space
     * Rendering shall be done with the conversion functions of KisVisualColorSelector
     * @param data points to zero-initialized memory of size width()*height()*pixelSize
     * @param pixelSize the data size to transfer from KoColor::data() to data per pixel
     * in the current color space
     * @param channelValues the normalized channel values of the currently picked color
     */
    virtual QImage renderBackground(const QVector4D &channelValues, quint32 pixelSize) const;

    void mousePressEvent(QMouseEvent *e) override;
    void mouseMoveEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e) override;
    void paintEvent(QPaintEvent*) override;
    void resizeEvent(QResizeEvent *) override;

private:
    struct Private;
    const QScopedPointer<Private> m_d;

    /**
     * @brief convertShapeCoordinateToWidgetCoordinate
     * @return take the position in the shape and convert it to screen coordinates.
     */
    virtual QPointF convertShapeCoordinateToWidgetCoordinate(QPointF) const = 0;

    /**
     * @brief convertWidgetCoordinateToShapeCoordinate
     * Convert a coordinate in the widget's height/width to a shape coordinate.
     * @param coordinate the position your wish to have the shape coordinates of.
     */
    virtual QPointF convertWidgetCoordinateToShapeCoordinate(QPoint coordinate) const = 0;

    /**
     * @brief getPixmap
     * @return the pixmap of this shape.
     */
    virtual QRegion getMaskMap() = 0;
    virtual void drawCursor() = 0;
};

#endif
