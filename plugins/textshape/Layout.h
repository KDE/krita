/* This file is part of the KDE project
 * Copyright (C) 2006-2007, 2010 Thomas Zander <zander@kde.org>
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

#ifndef LAYOUT_H
#define LAYOUT_H

#include "TableLayout.h"

#include <KoTextDocumentLayout.h>
#include <KoTextBlockData.h>
#include <KoInsets.h>

#include <QTextLine>
#include <QTextBlock>
#include <QTextTableCell>
#include <QHash>
#include <QWeakPointer>

class KoStyleManager;
class KoTextBlockData;
class KoTextShapeData;
class TextShape;
class KoChangeTracker;
class KoImageCollection;
class ToCGenerator;

/**
 * The document layouter for KoText style docs.
 */
class Layout : public KoTextDocumentLayout::LayoutState
{
public:
    explicit Layout(KoTextDocumentLayout *parent);
    /// start layouting, return false when there is nothing to do
    virtual bool start();
    /// end layouting
    virtual void end();
    virtual void reset();
    /// returns true if reset has been called.
    virtual bool isInterrupted() const;
    virtual qreal width();
    virtual qreal x();
    virtual qreal y();
    virtual int cursorPosition() const;
    /// return the y offset of the document at start of shape.
    virtual qreal docOffsetInShape() const;
    /// when a line is added, update internal vars.  Return true if line does not fit in shape
    virtual bool addLine(QTextLine &line, bool processingLine = false);
    /// prepare for next paragraph; return false if there is no next parag.
    virtual bool nextParag();
    virtual bool previousParag();
    virtual qreal documentOffsetInShape();
    // get the text indent accounting for auto-text-indent
    qreal resolveTextIndent();
    virtual bool setFollowupShape(KoShape *shape);
    /// called by the KoTextDocumentLayout to notify the LayoutState of a successfully resized inline object
    virtual void registerInlineObject(const QTextInlineObject &inlineObject);
    virtual QTextTableCell hitTestTable(QTextTable *table, const QPointF &point);

    /// paint the document
    virtual void draw(QPainter *painter, const KoTextDocumentLayout::PaintContext & context);

    /// reimplemented from superclass
    virtual void clearTillEnd();

    /// reimplemented from superclass
    int numColumns() {
        return m_dropCapsNChars;
    }

    /// set default tab size for this document
    virtual void setTabSpacing(qreal spacing);
    /// Inner shapes possibly intersect and split line into more parts. This returns max part height.
    virtual qreal maxLineHeight() const {
        return m_maxLineHeight;
    }
private:
    friend class TestTableLayout; // to allow direct testing.

    void updateBorders();
    qreal topMargin();
    qreal listIndent();
    void cleanupShapes();
    void cleanupShape(KoShape *daShape);
    void nextShape();
    void drawFrame(QTextFrame *frame, QPainter *painter, const KoTextDocumentLayout::PaintContext & context, int inTable);
    void drawListItem(QPainter *painter, const QTextBlock &block, KoImageCollection *imageCollection);
    void drawTrackedChangeItem(QPainter *painter, QTextBlock &block, int selectionStart, int selectionEnd, const KoViewConverter *converter);
    void decorateParagraph(QPainter *painter, const QTextBlock &block, int selectionStart, int selectionEnd, const KoViewConverter *converter);
    void decorateTabs(QPainter *painter, const QVariantList& tabList, const QTextLine &line, const QTextFragment& currentFragment, int startOfBlock);

    qreal inlineCharHeight(const QTextFragment &fragment);

    /**
     * Find a footnote and if there exists one reserve space for it at the bottom of the shape.
     * @param line the line that we are currently doing layout for. Any finding of footnotes
     *      will be limited to content in this line.
     * @param oldLength this pointer will be filled with the text position in the footnote document
     *      before any footnotes have been inserted.  This is useful to undo the insertion of the
     *      footnote.
     * @return the height in document points of space used for all footnotes in this frame.
     */
    qreal findFootnote(const QTextLine &line, int *oldLength);
    void resetPrivate();

    /**
     * Handle any table layout work that needs to be done for the current block.
     *
     * This function is called from nextParag().
     */
    void handleTable();

    /**
     * Handle table breaking both forced and if cell is too high.
     *
     * This function is called from handleTable().
     */
    void handleTableBreak(QTextTableCell &previousCell, QTextTable *table);

    void drawStrikeOuts(QPainter *painter, const QTextFragment &currentFragment,
            const QTextLine &line, qreal x1, qreal x2, const int startOfFragmentInBlock,
            const int fragmentToLineOffset) const;
    void drawUnderlines(QPainter *painter, const QTextFragment &currentFragment,
            const QTextLine &line, qreal x1, qreal x2, const int startOfFragmentInBlock,
            const int fragmentToLineOffset) const;

    void updateFrameStack();

private:
    KoStyleManager *m_styleManager;

    KoChangeTracker *m_changeTracker;

    qreal m_y;
    QTextBlock m_block;
    KoTextBlockData *m_blockData;

    QTextBlockFormat m_format;
    QTextBlock::Iterator m_fragmentIterator;
    KoTextShapeData *m_data;
    bool m_newShape, m_newParag, m_reset, m_isRtl, m_inTable;
    KoInsets m_borderInsets;
    KoInsets m_shapeBorder;
    KoTextDocumentLayout *m_parent;
    QHash<int, qreal> m_inlineObjectHeights; // maps text-position to whole-line-height of an inline object
    TextShape *m_textShape;
    QVector<QTextFrame *> m_frameStack;
    QList<QWeakPointer<ToCGenerator> > m_tocGenerators;

    // demoText feature
    bool m_demoText, m_endOfDemoText;

    // tab setting
    qreal m_defaultTabSizing;
    int m_currentTabStop; // = n, where we should be looking from the nth tab stop onwards when
    // we decorate the tab for the text of a fragment
    int m_dropCapsNChars, m_dropCapsAffectsNMoreLines;
    qreal m_dropCapsAffectedLineWidthAdjust, m_y_justBelowDropCaps, m_dropCapsPositionAdjust;

    QString m_currentMasterPage;

    TableLayout m_tableLayout;   /**< Table layout. */
    QTextTableCell m_tableCell;  /**< Current table cell. */
    bool m_restartingAfterTableBreak; /** We are in a re-layout that was a result of a break in a table. */
    bool m_restartingFirstCellAfterTableBreak; /** We are in a re-layout that was a result of a break in a table. only true first cell after */
    qreal m_maxLineHeight;
};

#endif
