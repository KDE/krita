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

#include "AnchorStrategy.h"

class KoTextLayoutRootArea;
class KoTextShapeData;
class QTextBlock;
class QTextLayout;

class InlineAnchorStrategy  : public AnchorStrategy
{
public:
    InlineAnchorStrategy(KoTextAnchor *anchor, KoTextLayoutRootArea *rootArea);
    virtual ~InlineAnchorStrategy();

    virtual bool moveSubject();

private:

    bool countHorizontalPos(QPointF &newPosition, QTextBlock &block, QTextLayout *layout);
    bool countVerticalPos(QPointF &newPosition, KoTextShapeData *data, QTextBlock &block, QTextLayout *layout);
};

#endif /* INLINEANCHORSTRATEGY_H_ */
