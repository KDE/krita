/*
 * SPDX-FileCopyrightText: 2020 Mathias Wein <lynx.mw+kde@gmail.com>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef KIS_VISUAL_DIAMOND_SELECTOR_SHAPE_H
#define KIS_VISUAL_DIAMOND_SELECTOR_SHAPE_H

#include "KisVisualColorSelectorShape.h"

class KisVisualDiamondSelectorShape : public KisVisualColorSelectorShape
{
    Q_OBJECT
public:
    explicit KisVisualDiamondSelectorShape(KisVisualColorSelector *parent,
                                            Dimensions dimension,
                                            int channel1, int channel2,
                                            int margin = 5
            );
    ~KisVisualDiamondSelectorShape() override;

    void setBorderWidth(int /*width*/) override;

    /**
     * @brief getSpaceForSquare
     * @param geom the full widget rectangle
     * @return rectangle with enough space for second widget
     */
    QRect getSpaceForSquare(QRect geom) override;
    QRect getSpaceForCircle(QRect geom) override;
    QRect getSpaceForTriangle(QRect geom) override;

protected:
    QImage renderAlphaMask() const override;

private:

    QPointF convertShapeCoordinateToWidgetCoordinate(QPointF coordinate) const override;
    QPointF convertWidgetCoordinateToShapeCoordinate(QPointF coordinate) const override;

    QRegion getMaskMap() override;
    void drawCursor(QPainter &painter) override;

    int m_margin { 5 };
};
#endif
