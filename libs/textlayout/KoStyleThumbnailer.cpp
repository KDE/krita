/* This file is part of the KDE project
 * Copyright (C) 2006, 2009 Thomas Zander <zander@kde.org>
 * Copyright (C) 2007 Sebastian Sauer <mail@dipe.org>
 * Copyright (C) 2007 Pierre Ducroquet <pinaraf@gmail.com>
 * Copyright (C) 2008 Girish Ramakrishnan <girish@forwardbias.in>
 * Copyright (C) 2009,2011 KO GmbH <cbo@kogmbh.com>
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

#include "KoStyleThumbnailer.h"
#include "KoParagraphStyle.h"
#include "KoCharacterStyle.h"
#include "KoListStyle.h"
#include "KoListLevelProperties.h"
#include "KoTableStyle.h"
#include "KoTableColumnStyle.h"
#include "KoTableRowStyle.h"
#include "KoTableCellStyle.h"
#include "KoSectionStyle.h"
#include "KoTextDocument.h"
#include "KoTextDocumentLayout.h"
#include "KoTextLayoutRootArea.h"

#include <klocale.h>

#include <QFont>
#include <QMap>
#include <QPixmap>
#include <QRect>
#include <QTextTable>
#include <QTextTableFormat>
#include <QTextBlock>
#include <QTextCursor>
#include <QTextLayout>

#include <kdebug.h>

class KoStyleThumbnailer::Private
{
public:
    Private() : pixmapHelperDocument(0){ }

    QTextDocument *pixmapHelperDocument;
    KoTextDocumentLayout *documentLayout;
    QMap<int,QPixmap> pixmapMap; // map of pixmap representations of the styles
};

KoStyleThumbnailer::KoStyleThumbnailer()
        : d(new Private())
{
}

KoStyleThumbnailer::~KoStyleThumbnailer()
{
    delete d;
}

void KoStyleThumbnailer::setPixmapHelperDocument(QTextDocument *pixmapHelperDocument)
{
    if (d->pixmapHelperDocument)
        return;
    d->pixmapHelperDocument = pixmapHelperDocument;
/*    d->pixmapHelperDocument = new QTextDocument;
    d->documentLayout = new KoTextDocumentLayout(d->pixmapHelperDocument);
    d->pixmapHelperDocument->setDocumentLayout(d->documentLayout);
*/
}

QPixmap KoStyleThumbnailer::thumbnail(KoParagraphStyle *style)
{
    if (d->pixmapMap.contains(style->styleId())) {
        return d->pixmapMap[style->styleId()];
    }
    QTextCursor cursor (d->pixmapHelperDocument);
    QPixmap pm(250,48);

    pm.fill(Qt::transparent);
    QPainter p(&pm);

    p.translate(0, 1.5);
    p.setRenderHint(QPainter::Antialiasing);
    cursor.select(QTextCursor::Document);
    cursor.setBlockFormat(QTextBlockFormat());
    cursor.setBlockCharFormat(QTextCharFormat());
    cursor.setCharFormat(QTextCharFormat());
    cursor.insertText(style->name());
    QTextBlock block = cursor.block();
    style->applyStyle(block, true);
    dynamic_cast<KoTextDocumentLayout *> (d->pixmapHelperDocument->documentLayout())->layout();

    KoTextDocumentLayout::PaintContext pc;
    dynamic_cast<KoTextDocumentLayout*>(d->pixmapHelperDocument->documentLayout())->rootAreaForPosition(0)->paint(&p, pc);

    d->pixmapMap.insert(style->styleId(), pm);
    return pm;
}

QPixmap KoStyleThumbnailer::thumbnail(KoParagraphStyle *style, QSize size)
{
    QPixmap pm(size.width(), size.height());
    pm.fill(Qt::transparent);
    QPainter p(&pm);
    p.setRenderHint(QPainter::Antialiasing);

    //calculate the space needed for the font size indicator (should the preview big too big with the style's font size
    QString sizeHint = QString::number(style->characterStyle()->fontPointSize()) + "pt";
    p.save();
    QFont sizeHintFont = p.font();
    sizeHintFont.setPointSize(8);
    p.setFont(sizeHintFont);
    QRect sizeHintRect(p.boundingRect(0, 0, 1, 1, Qt::AlignCenter, sizeHint));

    QTextCursor cursor (d->pixmapHelperDocument);

    cursor.select(QTextCursor::Document);
    cursor.setBlockFormat(QTextBlockFormat());
    cursor.setBlockCharFormat(QTextCharFormat());
    cursor.setCharFormat(QTextCharFormat());
    cursor.insertText(style->name());
    QTextBlock block = cursor.block();
    style->applyStyle(block, true);
    dynamic_cast<KoTextDocumentLayout *> (d->pixmapHelperDocument->documentLayout())->layout();

    QSizeF documentSize = dynamic_cast<KoTextDocumentLayout *> (d->pixmapHelperDocument->documentLayout())->documentSize();
    if (documentSize.width() > size.width() || documentSize.height() > size.height()) {
        //calculate the font reduction factor so that the text + the sizeHint fits
        int reductionFactor = (int)((size.width()+2*sizeHintRect.width())/documentSize.width());

        cursor.select(QTextCursor::Document);
        QTextTableFormat tbFormat;
        tbFormat.setCellPadding(0);
        tbFormat.setCellSpacing(0);
        tbFormat.setBorderStyle(QTextFrameFormat::BorderStyle_None);
        QTextTable *table = cursor.insertTable(1, 2, tbFormat);
        style->characterStyle()->setFontPointSize(style->characterStyle()->fontPointSize()*reductionFactor);
        style->characterStyle()->applyStyle(&cursor);
        cursor.insertText(style->name());
        cursor.movePosition(QTextCursor::NextCell);
        cursor.setBlockFormat(QTextBlockFormat());
        cursor.setBlockCharFormat(QTextCharFormat());
        QTextCharFormat charFormat;
        charFormat.setFont(sizeHintFont);
        cursor.setCharFormat(charFormat);
        cursor.insertText(sizeHint);

        dynamic_cast<KoTextDocumentLayout *> (d->pixmapHelperDocument->documentLayout())->layout();
        documentSize = dynamic_cast<KoTextDocumentLayout *> (d->pixmapHelperDocument->documentLayout())->documentSize();
        while (documentSize.width() > size.width() || documentSize.height() > size.height()) {
            cursor.setPosition(table->cellAt(1, 1).firstCursorPosition().position());
            cursor.setPosition(table->cellAt(1, 1).lastCursorPosition().position(), QTextCursor::KeepAnchor);
            QTextCharFormat fmt = cursor.charFormat();
            fmt.setFontPointSize(fmt.fontPointSize() - 1);
            dynamic_cast<KoTextDocumentLayout *> (d->pixmapHelperDocument->documentLayout())->layout();
            documentSize = dynamic_cast<KoTextDocumentLayout *> (d->pixmapHelperDocument->documentLayout())->documentSize();
        }
    }

//    p.translate(0, 1.5);

    d->pixmapHelperDocument->drawContents(&p);

//    d->pixmapMap.insert(style->styleId(), pm);
    return pm;
}

QPixmap KoStyleThumbnailer::thumbnail(KoCharacterStyle *style)
{
    if (d->pixmapMap.contains(style->styleId())) {
        return d->pixmapMap[style->styleId()];
    }
    QTextCursor cursor (d->pixmapHelperDocument);
    QPixmap pm(250,48);

    pm.fill(Qt::transparent);
    QPainter p(&pm);

    p.translate(0, 1.5);
    p.setRenderHint(QPainter::Antialiasing);
    cursor.select(QTextCursor::Document);
    cursor.setBlockFormat(QTextBlockFormat());
    cursor.setBlockCharFormat(QTextCharFormat());
    cursor.setCharFormat(QTextCharFormat());
    cursor.insertText(style->name());
    QTextBlock block = cursor.block();
    style->applyStyle(block);
    dynamic_cast<KoTextDocumentLayout*> (d->pixmapHelperDocument->documentLayout())->layout();

    KoTextDocumentLayout::PaintContext pc;
    dynamic_cast<KoTextDocumentLayout*>(d->pixmapHelperDocument->documentLayout())->rootAreaForPosition(0)->paint(&p, pc);

    d->pixmapMap.insert(style->styleId(), pm);
    return pm;
}

QPixmap KoStyleThumbnailer::thumbnail(KoCharacterStyle *style, QSize size)
{
    return QPixmap();
}

