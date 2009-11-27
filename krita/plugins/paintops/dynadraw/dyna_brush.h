/*
 *  Copyright (c) 2008 Lukas Tvrdy <lukast.dev@gmail.com>
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

#ifndef _BRUSH_H_
#define _BRUSH_H_

#include <QVector>

#include <KoColor.h>

#include <kis_paint_device.h>
#include <kis_painter.h>
#include <kis_image.h>

#include "filter.h"

class DynaBrush
{

public:
    DynaBrush();
    ~DynaBrush();
    DynaBrush(KoColor inkColor);
    void paint(KisPaintDeviceSP dev, qreal x, qreal y, const KoColor &color);


    int applyFilter(qreal mx, qreal my);

    //setters
    void setImage(KisImageWSP image) {
        m_image = image;
    }

    void setInitialWidth(qreal width) {
        m_width = width;
    }

    void setMass(qreal mass) {
        m_cursorFilter.setMass(mass);
    }

    void setDrag(qreal drag) {
        m_cursorFilter.setDrag(drag);
    }

    void useFixedAngle(bool useFixedAngle) {
        m_cursorFilter.setUseFixedAngle(useFixedAngle);
    }

    void setAngle(qreal xangle, qreal yangle) {
        m_cursorFilter.setFixedAngles(xangle, yangle);
    }

    void setWidthRange(qreal widthRange) {
        m_maxWidth = widthRange;
    }

    void setAction(int action) {
        m_action = action;
    }

    void setCircleRadius(int circleRadius) {
        m_circleRadius = circleRadius;
    }

    void setLineCount(int lineCount) {
        m_lineCount = lineCount;
    }

    void setLineSpacing(qreal spacing) {
        m_lineSpacing = spacing;
    }

    void enableLine(bool enableLine) {
        m_enableLine = enableLine;
    }

    void enableTwoCircles(bool twoCircles) {
        m_twoCircles = twoCircles;
    }

    void updateCursorPosition(const QPointF &point) {
        m_cursorPos.setX(point.x() / m_image->width());
        m_cursorPos.setY(point.y() / m_image->height());
    }

    void drawSegment(KisPainter &painter);


private:
    void drawCircle(KisPainter &painter, qreal x, qreal y, int radius, int steps);

    void drawQuad(KisPainter &painter,
                  QPointF &topRight, QPointF &topLeft,
                  QPointF &bottomLeft, QPointF &bottomRight);

    void drawWire(KisPainter &painter,
                  QPointF &topRight,
                  QPointF &topLeft,
                  QPointF &bottomLeft,
                  QPointF &bottomRight);


    void drawLines(KisPainter &painter,
                   QPointF &prev,
                   QPointF &now,
                   int count);

    KoColor m_inkColor;
    KisImageWSP m_image;

    int m_counter;
    int m_pixelSize;

    QVector<QPointF> m_prevPosition;

    qreal m_odelx, m_odely;

    // cursor position in document in relative coordinates
    QPointF m_cursorPos;
    // filters cursor position
    DynaFilter m_cursorFilter;

    bool m_initialized;
    // settings variables
    qreal m_width;
    qreal m_maxWidth;

    // drawing various primitives
    int m_action;
    bool m_enableLine, m_twoCircles;
    qreal m_circleRadius;

    int m_lineCount;
    qreal m_lineSpacing;

};

#endif
