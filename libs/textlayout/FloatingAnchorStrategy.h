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

#include "AnchorStrategy.h"

class KoTextLayoutRootArea;
class KoTextShapeData;
class QTextBlock;
class QTextLayout;
class KoTextLayoutObstruction;
class KoAnchorTextRange;

class FloatingAnchorStrategy  : public AnchorStrategy
{
public:
    FloatingAnchorStrategy(KoAnchorTextRange *anchor, KoTextLayoutRootArea *rootArea);
    ~FloatingAnchorStrategy();

    /**
     * This moves the subject (i.e. shape when used with flake) of the anchor.
     *
     * @return true if subject was moved
     */
    virtual bool moveSubject();

private:

    inline bool countHorizontalRel(QRectF &anchorBoundingRect, const QRectF &containerBoundingRect,
                                   QTextBlock &block, QTextLayout *layout);
    inline void countHorizontalPos(QPointF &newPosition, const QRectF &anchorBoundingRect);
    inline bool countVerticalRel(QRectF &anchorBoundingRect, const QRectF &containerBoundingRect,
                                 KoTextShapeData *data, QTextBlock &block, QTextLayout *layout);
    inline void countVerticalPos(QPointF &newPosition, const QRectF &anchorBoundingRect);

    //check the layout evironment and move the shape back to have it within
    inline void checkLayoutEnvironment(QPointF &newPosition, KoTextShapeData *data);
    //check the border of page and move the shape back to have it visible
    inline void checkPageBorder(QPointF &newPosition);
    //check stacking and reorder to proper position objects according to there z-index
    inline void checkStacking(QPointF &newPosition);

    void updateObstruction(qreal documentOffset);

    KoTextLayoutObstruction *m_obstruction; // the obstruction representation of the subject
    KoAnchorTextRange *m_anchorRange;
};

#endif // FLOATINGANCHORSTRATEGY_H
