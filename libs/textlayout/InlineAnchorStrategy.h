/* This file is part of the KDE project
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

#ifndef INLINEANCHORSTRATEGY_H_
#define INLINEANCHORSTRATEGY_H_

#include "KoTextAnchor.h"

class KoTextShapeData;
class QTextBlock;
class QTextLayout;

class InlineAnchorStrategy  : public KoAnchorStrategy
{
public:
    InlineAnchorStrategy(KoTextAnchor *anchor);
    ~InlineAnchorStrategy();

    /**
     * This function calculates position for linked shape.
     *
     * @return true if new position for shape was wound
     */
    virtual bool positionShape(int layoutCursorPosition);

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

    bool countHorizontalPos(QPointF &newPosition, QTextBlock &block, QTextLayout *layout);
    bool countVerticalPos(QPointF &newPosition, KoTextShapeData *data, QTextBlock &block, QTextLayout *layout);
    KoTextAnchor *const m_anchor;

    bool m_finished; // true if shape position was found
    bool m_relayoutNeeded; // true if shape intersected text when positioned
    QPointF m_relayoutPosition; // top most position of text and shape intersection
};

#endif /* INLINEANCHORSTRATEGY_H_ */
