/* This file is part of the KDE project
 * Copyright (C) 2011 Brijesh Patel <brijesh3105@gmail.com>
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

#include "KoTextLayoutNoteArea.h"

#include "FrameIterator.h"
#include "KoStyleManager.h"
#include "KoParagraphStyle.h"
#include "KoTextLayoutObstruction.h"
#include "KoPointedAt.h"
#include <KoOdfNumberDefinition.h>
#include <KoInlineNote.h>
#include <KoTextDocument.h>

#include <QPainter>

#define OVERLAPPREVENTION 1000

class Q_DECL_HIDDEN KoTextLayoutNoteArea::Private
{
public:
    Private()
    {
    }
    KoInlineNote *note;
    QTextLayout *textLayout;
    QTextLayout *postLayout;
    qreal labelIndent;
    bool isContinuedArea;
    qreal labelWidth;
    qreal labelHeight;
    qreal labelYOffset;
};

KoTextLayoutNoteArea::KoTextLayoutNoteArea(KoInlineNote *note, KoTextLayoutArea *parent, KoTextDocumentLayout *documentLayout)
  : KoTextLayoutArea(parent, documentLayout)
  , d(new Private)
{
    Q_ASSERT(note);
    Q_ASSERT(parent);

    d->note = note;
    d->isContinuedArea = false;
    d->postLayout = 0;
}

KoTextLayoutNoteArea::~KoTextLayoutNoteArea()
{
    delete d;
}

void KoTextLayoutNoteArea::paint(QPainter *painter, const KoTextDocumentLayout::PaintContext &context)
{
    painter->save();
    if (d->isContinuedArea) {
        painter->translate(0, -OVERLAPPREVENTION);
    }

    KoTextLayoutArea::paint(painter, context);
    if (d->postLayout) {
        d->postLayout->draw(painter, QPointF(left() + d->labelIndent, top() + d->labelYOffset));
    }
    d->textLayout->draw(painter, QPointF(left() + d->labelIndent, top() + d->labelYOffset));
    painter->restore();
}

bool KoTextLayoutNoteArea::layout(FrameIterator *cursor)
{
    KoOdfNotesConfiguration *notesConfig = 0;
    if (d->note->type() == KoInlineNote::Footnote) {
        notesConfig = KoTextDocument(d->note->textFrame()->document()).styleManager()->notesConfiguration(KoOdfNotesConfiguration::Footnote);
    } else if (d->note->type() == KoInlineNote::Endnote) {
        notesConfig = KoTextDocument(d->note->textFrame()->document()).styleManager()->notesConfiguration(KoOdfNotesConfiguration::Endnote);
    }

    QString label;
    if (d->isContinuedArea) {
        if (! notesConfig->footnoteContinuationBackward().isEmpty()) {
            label = notesConfig->footnoteContinuationBackward() + " " + d->note->label();
        }
        setReferenceRect(left(), right(), top() + OVERLAPPREVENTION
                                    , maximumAllowedBottom() + OVERLAPPREVENTION);
    } else {
        label = d->note->label();
    }
    label.prepend(notesConfig->numberFormat().prefix());
    label.append(notesConfig->numberFormat().suffix());
    QPaintDevice *pd = documentLayout()->paintDevice();
    QTextBlock block = cursor->it.currentBlock();
    QTextCharFormat format = block.charFormat();
    KoCharacterStyle *style = static_cast<KoCharacterStyle *>(notesConfig->citationTextStyle());
    if (style) {
        style->applyStyle(format);
    }
    QFont font(format.font(), pd);
    d->textLayout = new QTextLayout(label, font, pd);
    QList<QTextLayout::FormatRange> layouts;
    QTextLayout::FormatRange range;
    range.start = 0;
    range.length = label.length();
    range.format = format;
    layouts.append(range);
    d->textLayout->setAdditionalFormats(layouts);

    QTextOption option(Qt::AlignLeft | Qt::AlignAbsolute);
    d->textLayout->setTextOption(option);
    d->textLayout->beginLayout();
    QTextLine line = d->textLayout->createLine();
    d->textLayout->endLayout();

    KoParagraphStyle pStyle(block.blockFormat(), QTextCharFormat());
    d->labelIndent = textIndent(d->note->textFrame()->begin().currentBlock(), 0, pStyle);
    if (line.naturalTextWidth() > -d->labelIndent) {
        KoTextLayoutArea::setExtraTextIndent(line.naturalTextWidth());
    } else {
        KoTextLayoutArea::setExtraTextIndent(-d->labelIndent);
    }
    d->labelIndent += pStyle.leftMargin();
    d->labelWidth = line.naturalTextWidth();
    d->labelHeight = line.naturalTextRect().bottom() - line.naturalTextRect().top();
    d->labelYOffset = -line.ascent();

    bool contNotNeeded = KoTextLayoutArea::layout(cursor);

    d->labelYOffset += block.layout()->lineAt(0).ascent();

    if (!contNotNeeded) {
        QString contNote = notesConfig->footnoteContinuationForward();
        font.setBold(true);
        d->postLayout = new QTextLayout(contNote, font, pd);
        QList<QTextLayout::FormatRange> contTextLayouts;
        QTextLayout::FormatRange contTextRange;
        contTextRange.start = 0;
        contTextRange.length = contNote.length();
        contTextRange.format = block.charFormat();;
        contTextLayouts.append(contTextRange);
        d->postLayout->setAdditionalFormats(contTextLayouts);

        QTextOption contTextOption(Qt::AlignLeft | Qt::AlignAbsolute);
        //option.setTextDirection();
        d->postLayout->setTextOption(contTextOption);
        d->postLayout->beginLayout();
        QTextLine contTextLine = d->postLayout->createLine();
        d->postLayout->endLayout();
        contTextLine.setPosition(QPointF(right() - contTextLine.naturalTextWidth(), bottom() - contTextLine.height()));

        documentLayout()->setContinuationObstruction(new
                                    KoTextLayoutObstruction(contTextLine.naturalTextRect(), false));
    }
    return contNotNeeded;
}

void KoTextLayoutNoteArea::setAsContinuedArea(bool isContinuedArea)
{
    d->isContinuedArea = isContinuedArea;
}

KoPointedAt KoTextLayoutNoteArea::hitTest(const QPointF &p, Qt::HitTestAccuracy accuracy) const
{
    KoPointedAt pointedAt;
    pointedAt.noteReference = -1;
    QPointF tmpP(p.x(), p.y() + (d->isContinuedArea ? OVERLAPPREVENTION : 0));

    pointedAt = KoTextLayoutArea::hitTest(tmpP, accuracy);

    if (tmpP.x() > left() && tmpP.x() < d->labelWidth && tmpP.y() < top() + d->labelYOffset + d->labelHeight)
    {
        pointedAt.noteReference = d->note->getPosInDocument();
        pointedAt.position = tmpP.x();
    }

    return pointedAt;
}

QRectF KoTextLayoutNoteArea::selectionBoundingBox(QTextCursor &cursor) const
{
    return KoTextLayoutArea::selectionBoundingBox(cursor).translated(0
                                        , d->isContinuedArea ? -OVERLAPPREVENTION : 0);

}
