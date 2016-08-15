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

class KisVisualColorSelectorShape : public QWidget
{
    Q_OBJECT
public:
    /**
     * @brief The Dimensions enum
     * Wether or not the shape is single or two dimensional.
     * A 2d widget can represent at maximum 2 coordinates.
     */
    enum Dimensions{onedimensional, twodimensional};
    enum ColorModel{Channel, HSV, HSL, HSI, HSY, YUV};
    explicit KisVisualColorSelectorShape(QWidget *parent,
                                         KisVisualColorSelectorShape::Dimensions dimension,
                                         KisVisualColorSelectorShape::ColorModel model,
                                         const KoColorSpace *cs,
                                         int channel1, int channel2);
    ~KisVisualColorSelectorShape();

    QPointF getCursorPosition();

    Dimensions getDimensions();
    ColorModel getColorModel();
    QPixmap getPixmap();
    void setFullImage(QPixmap full);
    KoColor getCurrentColor();
Q_SIGNALS:
    void sigNewColor(KoColor col);

public Q_SLOTS:
    void setColor(KoColor c);
    void setColorFromSibling(KoColor c);
    void slotSetActiveChannels(int channel1, int channel2);
protected:
    void mousePressEvent(QMouseEvent *e);
    void mouseMoveEvent(QMouseEvent *e);
    void mouseReleaseEvent(QMouseEvent *e);
    void paintEvent(QPaintEvent*);
    void resizeEvent(QResizeEvent *);
private:
    struct Private;
    const QScopedPointer<Private> m_d;

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
    KoColor convertShapeCoordinateToKoColor(QPointF coordinates);

    /**
     * @brief getPixmap
     * @return the pixmap of this shape.
     */
    virtual QRegion getMaskMap() = 0;
    virtual void drawCursor() = 0;
};

class KisVisualRectangleSelectorShape : public KisVisualColorSelectorShape
{
    Q_OBJECT
public:
    enum singelDTypes{vertical, horizontal, border};
    explicit KisVisualRectangleSelectorShape(QWidget *parent,
                                         Dimensions dimension,
                                         ColorModel model,
                                         const KoColorSpace *cs,
                                         int channel1, int channel2,
                                         KisVisualRectangleSelectorShape::singelDTypes d = KisVisualRectangleSelectorShape::vertical);
    ~KisVisualRectangleSelectorShape();

    void setBarWidth();

private:
    virtual QPointF convertShapeCoordinateToWidgetCoordinate(QPointF coordinate);
    virtual QPointF convertWidgetCoordinateToShapeCoordinate(QPoint coordinate);

    singelDTypes m_type;
    virtual QRegion getMaskMap();
    virtual void drawCursor();
};

#endif // KISVISUALCOLORSELECTOR_H
