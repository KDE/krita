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
#ifndef KISVISUALCOLORSELECTOR_H
#define KISVISUALCOLORSELECTOR_H

#include <QWidget>
#include <QScopedPointer>
#include <QPixmap>
#include <QRegion>
#include <QMouseEvent>

#include <KoColor.h>
#include <KoColorSpace.h>
#include "KoColorDisplayRendererInterface.h"


#include "kritaui_export.h"

/**
 * @brief The KisVisualColorSelector class
 * this gives a color selector box that draws gradients and everything.
 * Unlike other color selectors, this one draws the full gamut of the given colorspace.
 */
class KRITAUI_EXPORT KisVisualColorSelector : public QWidget
{
    Q_OBJECT
public:
    explicit KisVisualColorSelector(QWidget *parent = 0);
    ~KisVisualColorSelector();

Q_SIGNALS:
    void sigNewColor(KoColor c);

public Q_SLOTS:

    void slotSetColor(KoColor c);
    void slotsetColorSpace(const KoColorSpace *cs);
    void setDisplayRenderer (const KoColorDisplayRendererInterface *displayRenderer);
private Q_SLOTS:
    void updateFromWidgets(KoColor c);
protected:
    void leaveEvent(QEvent *);
private:
    struct Private;
    const QScopedPointer<Private> m_d;

    void updateSelectorElements();
    void drawGradients();

};

//kis_visual_color_selector_shape

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
     * Wether or not the shape is single or two dimensional.
     **/
    enum Dimensions{onedimensional, twodimensional};
    enum ColorModel{Channel, HSV, HSL, HSI, HSY, YUV};
    explicit KisVisualColorSelectorShape(QWidget *parent,
                                         KisVisualColorSelectorShape::Dimensions dimension,
                                         KisVisualColorSelectorShape::ColorModel model,
                                         const KoColorSpace *cs,
                                         int channel1, int channel2,
                                         const KoColorDisplayRendererInterface *displayRenderer = KoDumbColorDisplayRenderer::instance());
    ~KisVisualColorSelectorShape();

    /**
     * @brief getCursorPosition
     * @return current cursor position in shape-coordinates.
     */
    QPointF getCursorPosition();
    /**
     * @brief getDimensions
     * @return whether this is a single or twodimensional widget.
     */
    Dimensions getDimensions();
    /**
     * @brief getColorModel
     * @return the model of this widget.
     */
    ColorModel getColorModel();
    /**
     * @brief getPixmap
     * @return the pixmap of the gradient, for drawing on with a subclass.
     * the pixmap will not change unless 'm_d->setPixmap=true' which is toggled by
     * refresh and update functions.
     */
    QPixmap getPixmap();
    /**
     * @brief setFullImage
     * Set the full widget image to be painted.
     * @param full this should be the full image.
     */
    void setFullImage(QPixmap full);
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
Q_SIGNALS:
    void sigNewColor(KoColor col);

public Q_SLOTS:
    /**
     * @brief setColor
     * Set this widget's current color and chang ethe cursor position.
     * @param c
     */
    void setColor(KoColor c);
    /**
     * @brief setColorFromSibling
     * set this widget's current color, but don't change the cursor position,
     * instead sent out a signal of the new color.
     * @param c
     */
    void setColorFromSibling(KoColor c);
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
    void mousePressEvent(QMouseEvent *e);
    void mouseMoveEvent(QMouseEvent *e);
    void mouseReleaseEvent(QMouseEvent *e);
    void paintEvent(QPaintEvent*);
private:
    struct Private;
    const QScopedPointer<Private> m_d;

    /**
     * @brief setHSX
     * This is for the cursor not to change when selecting
     * black, white, and desaturated values. Will not change the non-native values.
     * @param hsx the hsx value.
     */
    void setHSX(QVector <qreal> hsx);
    /**
     * @brief getHSX sets the sat and hue so they won't
     * switch around much.
     * @param hsx the hsx values.
     * @return returns hsx, corrected.
     */
    QVector <qreal> getHSX(QVector <qreal> hsx);

    /**
     * @brief convertShapeCoordinateToWidgetCoordinate
     * @return take the position in the shape and convert it to screen coordinates.
     */
    virtual QPointF convertShapeCoordinateToWidgetCoordinate(QPointF) = 0;

    /**
     * @brief convertWidgetCoordinateToShapeCoordinate
     * Convert a coordinate in the widget's height/width to a shape coordinate.
     * @param coordinate the position your wish to have the shape coordinates of.
     */
    virtual QPointF convertWidgetCoordinateToShapeCoordinate(QPoint coordinate) = 0;

    /**
     * @brief updateCursor
     * Update the cursor position.
     */
    void updateCursor();

    QPointF convertKoColorToShapeCoordinate(KoColor c);
    KoColor convertShapeCoordinateToKoColor(QPointF coordinates, bool cursor=false);

    /**
     * @brief getPixmap
     * @return the pixmap of this shape.
     */
    virtual QRegion getMaskMap() = 0;
    virtual void drawCursor() = 0;

    QVector <float> convertvectorqrealTofloat(QVector<qreal> real);
    QVector <qreal> convertvectorfloatToqreal(QVector <float> vloat);
};

class KisVisualRectangleSelectorShape : public KisVisualColorSelectorShape
{
    Q_OBJECT
public:
    enum singelDTypes{vertical, horizontal, border, borderMirrored};
    explicit KisVisualRectangleSelectorShape(QWidget *parent,
                                         Dimensions dimension,
                                         ColorModel model,
                                         const KoColorSpace *cs,
                                         int channel1, int channel2,
                                         const KoColorDisplayRendererInterface *displayRenderer = KoDumbColorDisplayRenderer::instance(), int width=20,
                                         KisVisualRectangleSelectorShape::singelDTypes d = KisVisualRectangleSelectorShape::vertical
                                         );
    ~KisVisualRectangleSelectorShape();

    void setBarWidth(int width);

private:
    virtual QPointF convertShapeCoordinateToWidgetCoordinate(QPointF coordinate);
    virtual QPointF convertWidgetCoordinateToShapeCoordinate(QPoint coordinate);

    singelDTypes m_type;
    int m_barWidth;
    virtual QRegion getMaskMap();
    virtual void drawCursor();
};

class KisVisualEllipticalSelectorShape : public KisVisualColorSelectorShape
{
    Q_OBJECT
public:
    enum singelDTypes{border, borderMirrored};
    explicit KisVisualEllipticalSelectorShape(QWidget *parent,
                                         Dimensions dimension,
                                         ColorModel model,
                                         const KoColorSpace *cs,
                                         int channel1, int channel2,
                                         const KoColorDisplayRendererInterface *displayRenderer = KoDumbColorDisplayRenderer::instance(), int borwidth=20,
                                         KisVisualEllipticalSelectorShape::singelDTypes d = KisVisualEllipticalSelectorShape::border
                                         );
    ~KisVisualEllipticalSelectorShape();

    void setBarWidth(int width);

private:
    virtual QPointF convertShapeCoordinateToWidgetCoordinate(QPointF coordinate);
    virtual QPointF convertWidgetCoordinateToShapeCoordinate(QPoint coordinate);

    singelDTypes m_type;
    int m_barWidth;
    virtual QRegion getMaskMap();
    virtual void drawCursor();
    QSize sizeHint() const;
};

class KisVisualTriangleSelectorShape : public KisVisualColorSelectorShape
{
    Q_OBJECT
public:
    enum singelDTypes{border, borderMirrored};
    explicit KisVisualTriangleSelectorShape(QWidget *parent,
                                         Dimensions dimension,
                                         ColorModel model,
                                         const KoColorSpace *cs,
                                         int channel1, int channel2,
                                         const KoColorDisplayRendererInterface *displayRenderer = KoDumbColorDisplayRenderer::instance(),
                                         int borwidth=20
                                         );
    ~KisVisualTriangleSelectorShape();

    void setBarWidth(int width);
    void setTriangle();
    QRect setGeometryByRadius(QLineF radius);

private:
    virtual QPointF convertShapeCoordinateToWidgetCoordinate(QPointF coordinate);
    virtual QPointF convertWidgetCoordinateToShapeCoordinate(QPoint coordinate);

    singelDTypes m_type;
    int m_barWidth;
    QPolygon m_triangle;
    virtual QRegion getMaskMap();
    virtual void drawCursor();
};
#endif // KISVISUALCOLORSELECTOR_H
