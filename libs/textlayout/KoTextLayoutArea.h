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

#include "FrameIterator.h"

#include <KoText.h>
#include <KoTextDocumentLayout.h>

#include <QRectF>
#include <QList>

class KoStyleManager;
class KoTextDocumentLayout;
class KoTextBlockData;
class KoImageCollection;
class KoInlineNote;
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
 * the RootArea, but it may also be set by for example a row in a table with fixed height.
 */
class KOTEXT_EXPORT KoTextLayoutArea
{
public:
    /// constructor
    explicit KoTextLayoutArea(KoTextLayoutArea *parent, KoTextDocumentLayout *documentLayout);
    virtual ~KoTextLayoutArea();

    /// Layouts as much as we can
    bool layout(FrameIterator *cursor);

    /// Returns the bounding rectangle in textdocument coordinates.
    QRectF boundingRect() const;

    virtual KoText::Direction parentTextDirection() const;

    /// Sets the left,right and top coordinate of the reference rect we place ourselves within
    /// The content may be smaller or bigger than that depending on our margins
    void setReferenceRect(qreal left, qreal right, qreal top, qreal maximumAllowedBottom);

    /// The left coordinate of the reference rect we place ourselves within
    /// The content may be smaller or bigger than that depending on our margins
    qreal left() const;

    /// The right coordinate of the reference rect we place ourselves within
    /// The content may be smaller or bigger than that depending on our margins
    qreal right() const;

    /// The top coordinate of the reference rect we place ourselves within
    /// The content may be smaller or bigger than that depending on our margins
    qreal top() const;

    /// The bottom coordinate of the reference rect we place ourselves within
    /// The content may be smaller or bigger than that depending on our margins
    /// bottom() can be used to place contents following this area
    qreal bottom() const;

    /// The maximum allowed bottom coordinate of the reference rect we place ourselves within
    /// The real bottom will be determined during layout
    qreal maximumAllowedBottom() const;

    /// A pointer to the parent
    KoTextLayoutArea *parent; // you should treat it as read only

    qreal listIndent() const;
    qreal textIndent(QTextBlock block) const;
    qreal x() const;
    qreal width() const;

    void paint(QPainter *painter, const KoTextDocumentLayout::PaintContext &context);

    int hitTest(const QPointF &point, Qt::HitTestAccuracy accuracy) const;

    /// Calc a bounding box rect of the selection
    QRectF selectionBoundingBox(QTextCursor &cursor) const;

protected:
    void setBottom(qreal bottom);
    KoTextDocumentLayout *documentLayout() {return m_documentLayout;}

    /// If this area has the responsibility to show footnotes then store
    /// it so it can later bein the m_pregisteredFootnotes
    virtual void preregisterFootNote(KoInlineNote *note);

    /// Takes all preregistered footnotes and create Areas out of them
    void confirmFootNotes();

private:
    bool layoutBlock(FrameIterator *cursor);

    /// Returns vertical height of line
    qreal addLine(FrameIterator *cursor, KoTextBlockData *blockData);

    /// looks for footnotes and preregisters them
    void findFootNotes(QTextBlock block, const QTextLine &line);

    void clearPreregisteredFootNotes();

    void drawListItem(QPainter *painter, const QTextBlock &block, KoImageCollection *imageCollection);

    void decorateParagraph(QPainter *painter, const QTextBlock &block);

    void drawStrikeOuts(QPainter *painter, const QTextFragment &currentFragment, const QTextLine &line, qreal x1, qreal x2, const int startOfFragmentInBlock, const int fragmentToLineOffset) const;

    void drawUnderlines(QPainter *painter, const QTextFragment &currentFragment, const QTextLine &line, qreal x1, qreal x2, const int startOfFragmentInBlock, const int fragmentToLineOffset) const;

    int decorateTabs(QPainter *painter, const QVariantList& tabList, const QTextLine &line, const QTextFragment& currentFragment, int startOfBlock, int currentTabStop);

    KoTextDocumentLayout *m_documentLayout;

    qreal m_left; // reference area left
    qreal m_right; // reference area right
    qreal m_top; // reference area top
    qreal m_bottom; // reference area top
    qreal m_maximalAllowedBottom;
    QRectF m_boundingRect;

    qreal m_x; // text area starts here as defined by margins (so not == m_left)
    qreal m_y;
    qreal m_width; // of text area as defined by margins (so not == m_right - m_left)
    qreal m_listIndent;
    qreal m_indent;
    qreal m_dropCapsWidth;
    int m_dropCapsNChars;
    qreal m_defaultTabSizing;
    bool m_isRtl;
    qreal m_bottomSpacing;
    QList<KoTextLayoutTableArea *> m_tableAreas;
    FrameIterator *m_startOfArea;
    FrameIterator *m_endOfArea;

    qreal m_preregisteredFootNotesHeight;
    qreal m_footNotesHeight;
    QList<KoTextLayoutArea *> m_preregisteredFootNoteAreas;
    QList<KoTextLayoutArea *> m_footNoteAreas;
};

#endif
