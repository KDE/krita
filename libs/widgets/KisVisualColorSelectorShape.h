/*
 * SPDX-FileCopyrightText: 2016 Wolthera van Hovell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KIS_VISUAL_COLOR_SELECTOR_SHAPE_H
#define KIS_VISUAL_COLOR_SELECTOR_SHAPE_H

#include <QWidget>
#include <QScopedPointer>
#include <QPixmap>
#include <QRegion>
#include <QMouseEvent>

#include <KoColor.h>
#include "KoColorDisplayRendererInterface.h"

#include "KisVisualColorSelector.h"
#include "KisColorSelectorConfiguration.h"

/**
 * @brief The KisVisualColorSelectorShape class
 * A 2d widget can represent at maximum 2 coordinates.
 * So first decide how many coordinates you need. (onedimensional, or twodimensional)
 * Then, select the channels you wish to be affected. This uses the model, so for cmyk
 * the channel is c=0, m=1, y=2, k=3, but for hsv, hue=0, sat=1, and val=2
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
    explicit KisVisualColorSelectorShape(KisVisualColorSelector *parent,
                                         KisVisualColorSelectorShape::Dimensions dimension,
                                         int channel1, int channel2);
    ~KisVisualColorSelectorShape() override;

    /**
     * @brief getCursorPosition
     * @return current cursor position in shape-coordinates.
     */
    QPointF getCursorPosition() const;
    /**
     * @brief getDimensions
     * @return whether this is a single or twodimensional widget.
     */
    Dimensions getDimensions() const;
    /**
     * @brief getImageMap returns  the updated base image
     * @return the final image of the shape content, before the handle gets drawn.
     * the pixmap will not change until a redraw is required, which depends on
     * whether the shape is static or changes depending on other color channels.
     */
    const QImage& getImageMap();
    /**
     * @brief getCurrentColor
     * @return the current kocolor
     */
    KoColor getCurrentColor();
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

    bool isHueControl() const;
    virtual bool supportsGamutMask() const;

    /**
     * @brief forceImageUpdate
     * force the image to recache.
     */
    void forceImageUpdate();

    /**
     * @brief Notify shape that the gamut mask changed
     *
     * The gamut mask shall be updated and the widget repainted if necessary.
     * This includes removal of gamut masks
     */
    virtual void updateGamutMask();

    /**
     * @brief setBorderWidth
     * set the border of the single dimensional selector.
     * @param width
     */
    virtual void setBorderWidth(int width) = 0;

    /**
     * @brief channel
     * Get the channel index associated with a selector shape dimension
     * @param dimension A shape dimension that can be controlled by the cursor
     * @return
     */
    int channel(int dimension) const;

    quint32 channelMask() const;

    /**
      * @brief setCursorPosition
      * Set the cursor to normalized shape coordinates. This will only repaint the cursor.
      * @param position normalized shape coordinates ([0,1] range, not yet transformed to actual channel values!)
      * @param signal if true, Q_EMIT a sigCursorMoved signal
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
    void setChannelValues(QVector4D channelValues, quint32 channelFlags);

    void setAcceptTabletEvents(bool on);

Q_SIGNALS:
    void sigCursorMoved(QPointF pos);

protected:
    KisVisualColorSelector* colorSelector() const;
    KisVisualColorModel* selectorModel() const;
    /**
     * @brief convertImageMap
     * convert image data containing raw KoColor data into a QImage
     * @param data must point to memory of size width()*height()*pixelSize
     * @param size the number of bytes to read from data, must match aforementioned criteria
     * @return the converted QImage guaranteed to match the widget size (black content on failure)
     */
    QImage convertImageMap(const quint8 *rawColor, quint32 bufferSize, QSize imgSize) const;
    /**
     * @brief renderBackground
     * Render the widget background visible inside the widget's mask in current color space
     * Rendering shall be done with the conversion functions of KisVisualColorSelector
     * @param data points to zero-initialized memory of size width()*height()*pixelSize
     * @param pixelSize the data size to transfer from KoColor::data() to data per pixel
     * in the current color space
     * @param channelValues the normalized channel values of the currently selected color
     */
    virtual QImage renderBackground(const QVector4D &channelValues, const QImage &alpha) const;
    virtual QImage compositeBackground() const;
    /**
     * @brief render the alpha mask for the widget background
     * the returned image is expected to be QImage::Format_Alpha8
     */
    virtual QImage renderAlphaMask() const;
    virtual QImage renderStaticAlphaMask() const;
    /**
     * @brief default implementation just calls convertWidgetCoordinateToShapeCoordinate(pos)
    */
    virtual QPointF mousePositionToShapeCoordinate(const QPointF &pos, const QPointF &dragStart) const;

    void mousePressEvent(QMouseEvent *e) override;
    void mouseMoveEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e) override;
    void tabletEvent(QTabletEvent* event) override;
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
    virtual QPointF convertWidgetCoordinateToShapeCoordinate(QPointF coordinate) const = 0;

    /**
     * @brief getPixmap
     * @return the pixmap of this shape.
     */
    virtual QRegion getMaskMap() = 0;
    virtual void drawCursor(QPainter &painter) = 0;
    virtual void drawGamutMask(QPainter &painter);
};

#endif
