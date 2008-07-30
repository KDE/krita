/* This file is part of the KDE project
 * Copyright (C) 2008 Florian Merz <florianmerz@gmx.de>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef PERSHAPEDATA_H
#define PERSHAPEDATA_H

#include "TextShape.h"

#include "Ruler.h"

#include <QRectF>

class KoParagraphStyle;

/* ShapeSpecificData is used by ParagraphTool to store information about a paragraph which is specific to a shape.
 * As the width of shapes may be different the positions and sizes of its ruler will be different, too.
 * This class takes care of these differences when painting the rulers or when handling input events.
 */
class ShapeSpecificData
{
public:
    ShapeSpecificData() {};
    ShapeSpecificData(Ruler* rulers, TextShape *textShape, QTextBlock textBlock, KoParagraphStyle *style);

    ~ShapeSpecificData() {};

    // return the first ruler at the point
    RulerIndex hitTest(const QPointF &point) const;

    // test if the point is inside of the active area of the ruler
    bool hitTest(RulerIndex ruler, const QPointF &point) const;

    // change the value of the ruler so that it touches the point
    void moveRulerTo(RulerIndex ruler, const QPointF &point, bool smoothMovement) const;

    // return the line which connects the ruler to its label
    QLineF labelConnector(RulerIndex ruler) const;

    // paint all rulers for this shape
    void paint(QPainter &painter, const KoViewConverter &converter) const;

    // returns the rectangle which needs to be repainted to fully refresh the display for this shape
    // currently this is the bounding rectangle of the shape plus a margin for the arrows on all four sides
    QRectF dirtyRectangle() const;

protected:
    void initDimensions(QTextBlock textBlock, KoParagraphStyle *paragraphStyle);
    void initBaselines();

    // wrapper method for textShapeData->documentOffset()
    qreal shapeStartOffset() const;

    QPointF mapDocumentToText(QPointF point) const;
    QPointF mapTextToDocument(QPointF point) const;
    QLineF mapTextToDocument(QLineF line) const;

    QLineF baseline(RulerIndex ruler) const;
    void setBaseline(RulerIndex ruler, QLineF baseline);

    QLineF separatorLine() const;

    TextShape *textShape() const { Q_ASSERT(m_textShape != NULL); return m_textShape; }

private:
    Ruler *m_rulers;
    TextShape *m_textShape;

    bool m_isSingleLine;

    QRectF m_counter,
           m_firstLine,
           m_followingLines,
           m_border;

    QLineF m_baselines[maxRuler];
};

#endif

