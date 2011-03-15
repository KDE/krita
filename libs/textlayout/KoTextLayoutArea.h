/* This file is part of the KDE project
 * Copyright (C) 2011 Casper Boemann <cbo@kogmbh.com>
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

#ifndef KOTEXTLAYOUTAREA_H
#define KOTEXTLAYOUTAREA_H

#include "kotext_export.h"

#include "HierarchicalCursor.h"

#include <KoText.h>

#include <QRectF>

class KoStyleManager;
class KoTextDocumentLayout;
class KoTextBlockData;
class QTextList;

/**
 * When layout'ing text it is chopped into physical area of space.
 * Example of such areas are:
 *  RootArea (corresponds to a text shape, or a spreadsheet cell)
 *  TableArea (the kind of table that appears in text documents)
 *  SectionArea, that splits text into columns
 * Each of these are implemeted through subclasses, and this is just the interface
 *
 * Layout happens until maximalAllowedY() is reached. That maximum may be set by
 * The RootArea, but it may also be set by for example a row in a table with fixed height.
 */
class KOTEXT_EXPORT KoTextLayoutArea
{
public:
    /// constructor
    explicit KoTextLayoutArea(KoTextLayoutArea *parent);
    virtual ~KoTextLayoutArea();

    /// Layouts as much as it can
    virtual void layout(HierarchicalCursor *cursor);

    /// Returns the bounding rectangle in textdocument coordinates.
//    virtual QRectF boundingRect() const = 0;

    virtual qreal maximalAllowedY() const;
/*
    /// Sets the position in textdocument coordinates.
    virtual void setPosition(QPointF position) = 0;

    /// Sets the size in textdocument coordinates.
    virtual void setWidth(qreal width) = 0;
*/
    virtual KoText::Direction parentTextDirection() const;

    virtual qreal left() const;

    virtual qreal right() const;

    /// A pointer to the parent
    KoTextLayoutArea *parent; // you should treat it as read only

    qreal listIndent() const;
    qreal textIndent(QTextBlock block) const;
    qreal x() const;
    qreal width() const;
    
private:
    void layoutBlock(HierarchicalCursor *cursor);
    QTextLine createLine(HierarchicalCursor *cursor);

    KoTextDocumentLayout *m_documentLayout;
    KoStyleManager *m_styleManager;

    qreal m_x;
    qreal m_y;
    qreal m_width;
    qreal m_listIndent;
    qreal m_defaultTabSizing;
};

#endif
