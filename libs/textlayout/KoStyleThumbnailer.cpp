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
#include "FrameIterator.h"

#include <klocale.h>

#include <QFont>
#include <QPixmap>
#include <QPixmapCache>
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
    QPixmapCache pixmapCache; // cache of pixmap representations of the styles
};

KoStyleThumbnailer::KoStyleThumbnailer()
        : d(new Private())
{
    d->pixmapHelperDocument = new QTextDocument;
    d->documentLayout = new KoTextDocumentLayout(d->pixmapHelperDocument);
    d->pixmapHelperDocument->setDocumentLayout(d->documentLayout);
}

KoStyleThumbnailer::~KoStyleThumbnailer()
{
    delete d->documentLayout;
    delete d->pixmapHelperDocument;
    delete d;
}

QPixmap KoStyleThumbnailer::thumbnail(KoParagraphStyle *style)
{
    return thumbnail(style, QSize(250, 48));
}

QPixmap KoStyleThumbnailer::thumbnail(KoParagraphStyle *style, QSize size)
{
    QString pixmapKey = "p_" + QString::number(style->styleId()) + "_" + QString::number(size.width()) + "_" + QString::number(size.height());
    QPixmap pm(size.width(), size.height());

    if (d->pixmapCache.find(pixmapKey, &pm)) {
        return pm;
    }

    pm.fill(Qt::transparent);

    QTextCursor cursor(d->pixmapHelperDocument);
    cursor.select(QTextCursor::Document);
    cursor.setBlockFormat(QTextBlockFormat());
    cursor.setBlockCharFormat(QTextCharFormat());
    cursor.setCharFormat(QTextCharFormat());
    cursor.insertText(style->name());
    QTextBlock block = cursor.block();
    style->applyStyle(block, true);

    layoutThumbnail(size, pm);

    d->pixmapCache.insert(pixmapKey, pm);
    return pm;
}

QPixmap KoStyleThumbnailer::thumbnail(KoCharacterStyle *style)
{
    return thumbnail(style, QSize(250, 48));
}

QPixmap KoStyleThumbnailer::thumbnail(KoCharacterStyle *style, QSize size)
{
    QString pixmapKey = "p_" + QString::number(style->styleId()) + "_" + QString::number(size.width()) + "_" + QString::number(size.height());
    QPixmap pm(size.width(), size.height());

    if (d->pixmapCache.find(pixmapKey, &pm)) {
        return pm;
    }

    pm.fill(Qt::transparent);

    QTextCursor cursor(d->pixmapHelperDocument);
    cursor.select(QTextCursor::Document);
    cursor.setBlockFormat(QTextBlockFormat());
    cursor.setBlockCharFormat(QTextCharFormat());
    cursor.setCharFormat(QTextCharFormat());
    cursor.insertText(style->name());
    QTextBlock block = cursor.block();
    style->applyStyle(block);

    layoutThumbnail(size, pm);

    d->pixmapCache.insert(pixmapKey, pm);
    return pm;
}

void KoStyleThumbnailer::layoutThumbnail(QSize size, QPixmap &pm)
{
    QPainter p(&pm);
    p.setRenderHint(QPainter::Antialiasing);

    QTextCursor cursor (d->pixmapHelperDocument);

    d->documentLayout->removeRootArea();
    KoTextLayoutRootArea rootArea(d->documentLayout);

    FrameIterator frameCursor(d->pixmapHelperDocument->rootFrame());
    rootArea.setReferenceRect(0, size.width(), 0, 1E6);
    rootArea.setNoWrap(1E6);
    rootArea.layoutRoot(&frameCursor);

    QSizeF documentSize = rootArea.boundingRect().size();
    if (documentSize.width() > size.width() || documentSize.height() > size.height()) {
        QTextCharFormat fmt;
        cursor.select(QTextCursor::Document);
        fmt = cursor.charFormat();
        //calculate the font reduction factor so that the text fits and apply the new font size
        qreal reductionFactor = qMin(size.width()/documentSize.width(), size.height()/(documentSize.height()+2));
        fmt.setFontPointSize((int)(fmt.fontPointSize()*reductionFactor));
        cursor.mergeCharFormat(fmt);

        frameCursor = FrameIterator(d->pixmapHelperDocument->rootFrame());
        rootArea.setReferenceRect(0, size.width(), 0, 1E6);
        rootArea.setNoWrap(1E6);
        rootArea.layoutRoot(&frameCursor);

        //check that we now fit and eventually reduce the font size iteratively. this shouldn't be needed
        documentSize = rootArea.boundingRect().size();

        while ((documentSize.width() > size.width() || (documentSize.height()+2) > size.height()) && fmt.fontPointSize()>0) {
            fmt = cursor.blockCharFormat();
            fmt.setFontPointSize(fmt.fontPointSize() - 1);
            cursor.setBlockCharFormat(fmt);

            frameCursor = FrameIterator(d->pixmapHelperDocument->rootFrame());
            rootArea.setReferenceRect(0, size.width(), 0, 1E6);
            rootArea.setNoWrap(1E6);
            rootArea.layoutRoot(&frameCursor);
            documentSize = rootArea.boundingRect().size();
        }
    }

    KoTextDocumentLayout::PaintContext pc;
    p.translate(0, 1.5);
    rootArea.paint(&p, pc);
}
