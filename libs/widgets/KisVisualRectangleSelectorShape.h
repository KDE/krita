/*
 * SPDX-FileCopyrightText: 2016 Wolthera van Hovell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KIS_VISUAL_RECTANGLE_SELECTOR_SHAPE_H
#define KIS_VISUAL_RECTANGLE_SELECTOR_SHAPE_H

#include "KisVisualColorSelectorShape.h"

class KisVisualRectangleSelectorShape : public KisVisualColorSelectorShape
{
    Q_OBJECT
public:
    enum singelDTypes{vertical, horizontal, border, borderMirrored};
    explicit KisVisualRectangleSelectorShape(KisVisualColorSelector *parent,
                                             Dimensions dimension,
                                             int channel1, int channel2, int width=20,
                                             KisVisualRectangleSelectorShape::singelDTypes d = KisVisualRectangleSelectorShape::vertical
            );
    ~KisVisualRectangleSelectorShape() override;

    void setBorderWidth(int width) override;
    void setOneDimensionalType(singelDTypes type);

    /**
     * @brief getSpaceForSquare
     * @param geom the full widget rectangle
     * @return rectangle with enough space for second widget
     */
    QRect getSpaceForSquare(QRect geom) override;
    QRect getSpaceForCircle(QRect geom) override;
    QRect getSpaceForTriangle(QRect geom) override;
protected:
    QRect getAvailableSpace(QRect geom, bool stretch);
    QImage renderAlphaMask() const override;

private:
    QPointF convertShapeCoordinateToWidgetCoordinate(QPointF coordinate) const override;
    QPointF convertWidgetCoordinateToShapeCoordinate(QPointF coordinate) const override;

    singelDTypes m_type;
    int m_barWidth;
    QRegion getMaskMap() override;
    void drawCursor(QPainter &painter) override;
};

#endif 
