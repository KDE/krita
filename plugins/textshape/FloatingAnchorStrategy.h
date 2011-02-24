/* This file is part of the KDE project
 * Copyright (C) 2007, 2009, 2010 Thomas Zander <zander@kde.org>
 * Copyright (C) 2011 Matus Hanzes <matus.hanzes@ixonos.com>
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

#ifndef FLOATINGANCHORSTRATEGY_H
#define FLOATINGANCHORSTRATEGY_H

#include "KoTextAnchor.h"

class KoTextShapeData;

class FloatingAnchorStrategy  : public KoAnchorStrategy
{
public:
    FloatingAnchorStrategy(KoTextAnchor *anchor);
    ~FloatingAnchorStrategy();

    /**
     * This function calculates position for linked shape.
     *
     * @return true if new position for shape was wound
     */
    virtual bool positionShape(KoTextDocumentLayout::LayoutState *state);

    /**
     *
     * @return true if position for shape was wound
     */
    virtual bool isPositioned();

    //reset the state of this class
    virtual void reset();

    /**
     *
     * @return true if linked shape intersects with text
     */
    virtual bool isRelayoutNeeded();

    /**
     *
     * @return top most position of linked shape and text intersection
     */
    virtual QPointF relayoutPosition();

private:

    void calculateKnowledgePoint(); //calculate minimal text position from which enough information is ready to position the shape

    inline bool countHorizontalRel(QRectF &anchorBoundingRect, QRectF containerBoundingRect, KoTextDocumentLayout::LayoutState *state,
                                   QTextBlock &block, QTextLayout *layout);
    inline void countHorizontalPos(QPointF &newPosition, QRectF anchorBoundingRect, QRectF containerBoundingRect);
    inline bool countVerticalRel(QRectF &anchorBoundingRect, QRectF containerBoundingRect,
                                 KoTextShapeData *data, QTextBlock &block, QTextLayout *layout);
    inline void countVerticalPos(QPointF &newPosition, QRectF anchorBoundingRect, QRectF containerBoundingRect);

    //check the border of page an move the shape back to have it visible
    inline void checkPageBorder(QPointF &newPosition, QRectF containerBoundingRect);

    // true if shape is inside layouted text area
    inline bool checkTextIntersecion(QPointF &relayoutPos, QRectF shpRect, QRectF contRect, KoTextDocumentLayout::LayoutState *state,
                                     KoTextShapeData *data);

    KoTextAnchor *const m_anchor;

    int m_knowledgePoint; // the cursor position at which the layout process has gathered enough info to do our work

    bool m_finished; // true if shape position was found
    bool m_relayoutNeeded; // true if shape intersected text when positioned
    QPointF m_relayoutPosition; // top most position of text and shape intersection
};

#endif // FLOATINGANCHORSTRATEGY_H
