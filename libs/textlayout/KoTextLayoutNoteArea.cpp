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

#include <QPainter>

class KoTextLayoutNoteArea::Private
{
public:
    Private()
    {
    }
    KoInlineNote *note;
    QTextLayout *textLayout;
    qreal labelIndent;
};

KoTextLayoutNoteArea::KoTextLayoutNoteArea(KoInlineNote *note, KoTextLayoutArea *parent, KoTextDocumentLayout *documentLayout)
  : KoTextLayoutArea(parent, documentLayout)
  , d(new Private)
{
    Q_ASSERT(note);
    Q_ASSERT(parent);

    d->note = note;
}

KoTextLayoutNoteArea::~KoTextLayoutNoteArea()
{
    delete d;
}

void KoTextLayoutNoteArea::paint(QPainter *painter, const KoTextDocumentLayout::PaintContext &context)
{
    KoTextLayoutArea::paint(painter, context);
    d->textLayout->draw(painter, QPointF(left() + d->labelIndent, top()));
}

bool KoTextLayoutNoteArea::layout(FrameIterator *cursor)
{
    KoOdfNotesConfiguration *notesConfig = 0;
    if (d->note->type() == KoInlineNote::Footnote) {
        notesConfig = KoTextDocument(d->note->textFrame()->document()).styleManager()->notesConfiguration(KoOdfNotesConfiguration::Footnote);
    } else if (d->note->type() == KoInlineNote::Endnote) {
        notesConfig = KoTextDocument(d->note->textFrame()->document()).styleManager()->notesConfiguration(KoOdfNotesConfiguration::Endnote);
    }
    QString label = d->note->label();
    label.prepend(notesConfig->numberFormat().prefix());
    label.append(notesConfig->numberFormat().suffix());
    QPaintDevice *pd = documentLayout()->paintDevice();
    QTextCharFormat format = cursor->it.currentBlock().charFormat();
    QFont font(format.font(), pd);
    d->textLayout = new QTextLayout(label, font, pd);
    QList<QTextLayout::FormatRange> layouts;
    QTextLayout::FormatRange range;
    range.start = 0;
    range.length = label.length();
    range.format = format;
    range.format.setVerticalAlignment(QTextCharFormat::AlignSuperScript);
    layouts.append(range);
    d->textLayout->setAdditionalFormats(layouts);

    QTextOption option(Qt::AlignLeft | Qt::AlignAbsolute);
    //option.setTextDirection();
    d->textLayout->setTextOption(option);
    d->textLayout->beginLayout();
    QTextLine line = d->textLayout->createLine();
    d->textLayout->endLayout();
    KoTextLayoutArea::setExtraTextIndent(line.naturalTextWidth());

    KoParagraphStyle pStyle(d->note->textFrame()->begin().currentBlock().blockFormat(), QTextCharFormat());
    d->labelIndent = pStyle.leftMargin();

    return KoTextLayoutArea::layout(cursor);
}
