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

#include "textlayout_export.h"

#include "KoPointedAt.h"

#include <KoText.h>
#include <KoTextDocumentLayout.h>
#include <KoInsets.h>

#include <QRectF>
#include <QList>

class KoStyleManager;
class KoTextDocumentLayout;
class KoTextBlockData;
class KoInlineNote;
class KoPointedAt;
class QTextList;
class KoTextBlockBorderData;
class KoTextLayoutEndNotesArea;
class KoTextLayoutTableArea;
class KoTextLayoutNoteArea;
class FrameIterator;

/**
 * When layouting text the text is chopped up into physical area of space.
 *
 * Examples of such areas are:
 * <ul>
 *  <li>RootArea (corresponds to a text shape, or a spreadsheet cell)
 *  <li>TableArea (the kind of table that appears in text documents)
 *  <li>SectionArea, that splits text into columns
 * </ul>
 *
 * Each of these are implemented through subclasses, and this is just the interface.
 *
 * Layout happens until maximalAllowedY() is reached. That maximum may be set by
 * the RootArea, but it may also be set by, for example, a row in a table with
 * fixed height.
 */
class TEXTLAYOUT_EXPORT KoTextLayoutArea
{
public:
    /// constructor
    explicit KoTextLayoutArea(KoTextLayoutArea *parent, KoTextDocumentLayout *documentLayout);
    virtual ~KoTextLayoutArea();

    /// Returns true if the area starts at the cursor position
    bool isStartingAt(FrameIterator *cursor) const;

    QTextFrame::iterator startTextFrameIterator() const;
    QTextFrame::iterator endTextFrameIterator() const;

    /// Layouts as much as we can
    bool layout(FrameIterator *cursor);

    /// Returns the bounding rectangle in textdocument coordinates.
    QRectF boundingRect() const;

    virtual KoText::Direction parentTextDirection() const;

    KoTextLayoutArea *parent() const;
    KoTextDocumentLayout *documentLayout() const;

    /// Sets the left,right and top coordinate of the reference rect we place ourselves within
    /// The content may be smaller or bigger than that depending on our margins
    void setReferenceRect(qreal left, qreal right, qreal top, qreal maximumAllowedBottom);

    /// Returns the left,right,top and bottom coordinate of the reference rect.
    QRectF referenceRect() const;

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

    /// Sets the maximum allowed width before wrapping text.
    /// Setting this also indicates that we don't want wrapping.
    /// 0 means wrapping is allowed
    /// This has to be set before each layout(), and the layout() will change the reference rect
    /// right value to fit snugly (minimum the old right value) and so that no wrapping occours up
    /// to maximumAllowedWidth
    void setNoWrap(qreal maximumAllowedWidth);

    qreal listIndent() const;
    qreal textIndent(QTextBlock block, QTextList *textList) const;
    void setExtraTextIndent(qreal extraTextIndent);
    qreal x() const;
    qreal width() const;

    /// Set if Area accepts page breaks, default is false;
    void setAcceptsPageBreak(bool accept);

    /// Areas that accept page breaks return true, default is false;
    bool acceptsPageBreak() const;

    /// Should be set to true when first starting layouting page
    /// Should be set to false when we add anything during layout
    void setVirginPage(bool virgin);

    /// returns true if we have not yet added anything to the page
    bool virginPage() const;

    /// Sets the amound the contenst should be vertically offset due to any outside induced
    /// vertical alignment
    void setVerticalAlignOffset(qreal offset);
    qreal verticalAlignOffset() const;

    void paint(QPainter *painter, const KoTextDocumentLayout::PaintContext &context);

    KoPointedAt hitTest(const QPointF &point, Qt::HitTestAccuracy accuracy) const;

    /// Calc a bounding box rect of the selection
    /// or invalid if not
    QRectF selectionBoundingBox(QTextCursor &cursor) const;

    static const int MaximumTabPos = 10000;

protected:
    void setBottom(qreal bottom);

    /// If this area has the responsibility to show footnotes then store
    /// it so it can later bein the m_pregisteredFootnotes
    /// returns the height of the foot note
    virtual qreal preregisterFootNote(KoInlineNote *note);

    /// Takes all preregistered footnotes and create Areas out of them
    void confirmFootNotes();

    /// Set the Left of the boundingRect to the min of what it was and x
    void expandBoundingLeft(qreal x);

    /// Set the Right of the boundingRect to the max of what it was and x
    void expandBoundingRight(qreal x);

private:
    /// utility method to restartlayout of a block
    QTextLine restartLayout(QTextLayout *layout, int lineTextStartOfLastKeep);

    bool layoutBlock(FrameIterator *cursor);

    /// Returns vertical height of line
    qreal addLine(QTextLine &line, FrameIterator *cursor, KoTextBlockData *blockData);

    /// looks for footnotes and preregisters them
    void findFootNotes(QTextBlock block, const QTextLine &line);

    void clearPreregisteredFootNotes();

    void drawListItem(QPainter *painter, const QTextBlock &block);

    void decorateParagraph(QPainter *painter, const QTextBlock &block);

    void drawStrikeOuts(QPainter *painter, const QTextFragment &currentFragment, const QTextLine &line, qreal x1, qreal x2, const int startOfFragmentInBlock, const int fragmentToLineOffset) const;

    void drawOverlines(QPainter *painter, const QTextFragment &currentFragment, const QTextLine &line, qreal x1, qreal x2, const int startOfFragmentInBlock, const int fragmentToLineOffset) const;

    void drawUnderlines(QPainter *painter, const QTextFragment &currentFragment, const QTextLine &line, qreal x1, qreal x2, const int startOfFragmentInBlock, const int fragmentToLineOffset) const;

    int decorateTabs(QPainter *painter, const QVariantList& tabList, const QTextLine &line, const QTextFragment& currentFragment, int startOfBlock, int currentTabStop);

    void handleBordersAndSpacing(KoTextBlockData *blockData, QTextBlock *block);

    KoTextLayoutArea *m_parent; //  A pointer to the parent

    KoTextDocumentLayout *m_documentLayout;

    qreal m_left; // reference area left
    qreal m_right; // reference area right
    qreal m_top; // reference area top
    qreal m_bottom; // reference area top
    qreal m_maximalAllowedBottom;
    qreal m_maximumAllowedWidth; // 0 indicates wrapping is allowed
    QRectF m_boundingRect;
    KoTextBlockBorderData *m_prevBorder;
    qreal m_prevBorderPadding;

    qreal m_x; // text area starts here as defined by margins (so not == m_left)
    qreal m_y;
    qreal m_width; // of text area as defined by margins (so not == m_right - m_left)
    qreal m_listIndent;
    qreal m_indent;
    qreal m_dropCapsWidth;
    int m_dropCapsNChars;
    bool m_isRtl;
    qreal m_bottomSpacing;
    QList<KoTextLayoutTableArea *> m_tableAreas;
    FrameIterator *m_startOfArea;
    FrameIterator *m_endOfArea;
    FrameIterator *m_footNotesStartOfArea;

    bool m_acceptsPageBreak;
    bool m_virginPage;
    qreal m_verticalAlignOffset;
    QList<QRectF> m_blockRects;

    qreal m_preregisteredFootNotesHeight;
    qreal m_footNotesHeight;
    int m_footNoteAutoCount;
    qreal m_extraTextIndent;
    QList<KoTextLayoutNoteArea *> m_preregisteredFootNoteAreas;
    QList<KoTextLayoutNoteArea *> m_footNoteAreas;
    KoTextLayoutEndNotesArea *m_endNotesArea;
    QList<KoTextLayoutArea *> m_tableOfContentsAreas;
    QList<KoTextLayoutArea *> m_bibliographyAreas;
};

#endif
