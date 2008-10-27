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

#ifndef PARAGRAPHFRAGMENT_H
#define PARAGRAPHFRAGMENT_H

#include "Ruler.h"
#include "RulerFragment.h"

#include <QRectF>
#include <QTextBlock>

class KoParagraphStyle;
class KoShape;

// this enum defines the order in which the rulers will be focused when tab is pressed
typedef enum {
    topMarginRuler,
    rightMarginRuler,
    bottomMarginRuler,
    followingIndentRuler,
    firstIndentRuler,
    lineSpacingRuler,
    maxRuler,
    noRuler
} RulerIndex;

/* ParagraphFragment is used by ParagraphTool to store information about a
 * paragraph which is specific to a shape. As the width of shapes may be
 * different the positions and sizes of its ruler will be different, too.
 * This class takes care of these differences when painting the rulers or
 * when handling input events.
 */
class ParagraphFragment
{
public:
    ParagraphFragment() {};
    ParagraphFragment(Ruler* rulers, KoShape *shape, QTextBlock textBlock, KoParagraphStyle *style);

    ~ParagraphFragment() {};

    KoShape *shape() const { return m_shape; }

protected:
    void initDimensions(QTextBlock textBlock, KoParagraphStyle *paragraphStyle);
    void initRulers();

    // wrapper method for textShapeData->documentOffset()
    qreal shapeTop() const;
    qreal shapeBottom() const;

    QPointF mapTextToDocument(QPointF point) const;
    QLineF mapTextToDocument(QLineF line) const;

private:
    KoShape *m_shape;
    Ruler *m_rulers;

    bool m_isSingleLine;

    QRectF m_counter,
    m_firstLine,
    m_followingLines,
    m_border;
};

#endif

