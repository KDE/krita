/* This file is part of the KDE project
 * Copyright (C) 2008 Florian Merz <florianmerz@web.de>
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

#include <QRectF>

/* ShapeSpecificData is used by ParagraphTool to store information about a paragraph which is specific to a shape.
 * As the width of shapes may be different the positions and sizes of its ruler will be different, too.
 * This class takes care of these differences when painting the rulers or when handling input events.
 */
class ShapeSpecificData
{
public:
    ShapeSpecificData() : m_textShape(NULL) {};
    ~ShapeSpecificData() {};

    qreal shapeStartOffset() const;
    qreal shapeEndOffset() const;
    bool shapeContainsBlock(QTextBlock textBlock) const;

    QPointF mapDocumentToShape(QPointF point) const;
    QRectF dirtyRectangle() const;

    TextShape *textShape() const { Q_ASSERT(m_textShape != NULL); return m_textShape; }
    void setTextShape(TextShape *textShape) { m_textShape = textShape; }

private:
    TextShape *m_textShape;

    QRectF m_counter,
           m_firstLine,
           m_followingLines,
           m_border;
};

#endif

