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

#include "KisColorSelectorConfiguration.h"
#include "KisVisualColorSelectorShape.h"

class KisVisualEllipticalSelectorShape : public KisVisualColorSelectorShape
{
    Q_OBJECT
public:
    enum singelDTypes{border, borderMirrored};
    explicit KisVisualEllipticalSelectorShape(QWidget *parent,
                                              Dimensions dimension,
                                              const KoColorSpace *cs,
                                              int channel1, int channel2,
                                              const KoColorDisplayRendererInterface *displayRenderer = KoDumbColorDisplayRenderer::instance(), int barWidth=20,
                                              KisVisualEllipticalSelectorShape::singelDTypes d = KisVisualEllipticalSelectorShape::border
            );
    ~KisVisualEllipticalSelectorShape() override;

    void setBorderWidth(int width) override;

    /**
     * @brief getSpaceForSquare
     * @param geom the full widget rectangle
     * @return rectangle with enough space for second widget
     */
    QRect getSpaceForSquare(QRect geom) override;
    QRect getSpaceForCircle(QRect geom) override;
    QRect getSpaceForTriangle(QRect geom) override;
protected:
private:
    QPointF convertShapeCoordinateToWidgetCoordinate(QPointF coordinate) const override;
    QPointF convertWidgetCoordinateToShapeCoordinate(QPoint coordinate) const override;


    singelDTypes m_type;
    int m_barWidth;
    QRegion getMaskMap() override;
    void drawCursor() override;
    QSize sizeHint() const override;
};

#endif // KISVISUALCOLORSELECTOR_H
