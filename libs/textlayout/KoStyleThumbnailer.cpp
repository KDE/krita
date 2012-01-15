/* This file is part of the KDE project
 * Copyright (C) 2006, 2009 Thomas Zander <zander@kde.org>
 * Copyright (C) 2007 Sebastian Sauer <mail@dipe.org>
 * Copyright (C) 2007 Pierre Ducroquet <pinaraf@gmail.com>
 * Copyright (C) 2008 Girish Ramakrishnan <girish@forwardbias.in>
 * Copyright (C) 2009,2011 KO GmbH <cbo@kogmbh.com>
 * Copyright (C) 2011-2012 Pierre Stirnweiss <pstirnweiss@googlemail.com>
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

#include <QCache>
#include <QFont>
#include <QImage>
#include <QPainter>
#include <QRect>
#include <QTextTable>
#include <QTextTableFormat>
#include <QTextBlock>
#include <QTextCursor>
#include <QTextLayout>
#include <QTextLength>

#include <kdebug.h>

class KoStyleThumbnailer::Private
{
public:
    Private() :
        thumbnailHelperDocument(new QTextDocument),
        documentLayout(new KoTextDocumentLayout(thumbnailHelperDocument)),
        defaultSize(QSize(250, 48))
    {
        thumbnailHelperDocument->setDocumentLayout(documentLayout);
    }

    ~Private()
    {
        delete documentLayout;
        delete thumbnailHelperDocument;
    }

    QTextDocument *thumbnailHelperDocument;
    KoTextDocumentLayout *documentLayout;
    QCache<QString, QImage> thumbnailCache; // cache of QImage representations of the styles
    QSize defaultSize;
};

KoStyleThumbnailer::KoStyleThumbnailer()
        : d(new Private())
{
}

KoStyleThumbnailer::~KoStyleThumbnailer()
{
    delete d;
}

QImage KoStyleThumbnailer::thumbnail(KoParagraphStyle *style, QSize size, bool recreateThumbnail)
{
    if (!style || style->name().isNull()) {
        return QImage();
    }
    if (!size.isValid() || size.isNull()) {
        size = d->defaultSize;
    }
    QString imageKey = "p_" + QString::number(style->styleId()) + "_" + QString::number(size.width()) + "_" + QString::number(size.height());

    if (!recreateThumbnail && d->thumbnailCache.object(imageKey)) {
        return QImage(*(d->thumbnailCache.object(imageKey)));
    }

    QImage *im = new QImage(size.width(), size.height(), QImage::Format_ARGB32_Premultiplied);
    im->fill(QColor(Qt::transparent).rgba());

    KoParagraphStyle *clone = style->clone();
    //TODO: make the following real options
    //we ignore these properties when the thumbnail would not be sufficient to preview properly the whole paragraph with margins.
    clone->setMargin(QTextLength(QTextLength::FixedLength, 0));
    clone->setPadding(0);
    //
    QTextCursor cursor(d->thumbnailHelperDocument);
    cursor.select(QTextCursor::Document);
    cursor.setBlockFormat(QTextBlockFormat());
    cursor.setBlockCharFormat(QTextCharFormat());
    cursor.setCharFormat(QTextCharFormat());
    QTextBlock block = cursor.block();
    clone->applyStyle(block, true);

    QTextCharFormat format;
    clone->KoCharacterStyle::applyStyle(format);
    cursor.insertText(clone->name(), format);

    layoutThumbnail(size, im);

    d->thumbnailCache.insert(imageKey, im);
    delete clone;
    return QImage(*im);
}

QImage KoStyleThumbnailer::thumbnail(KoCharacterStyle *style, QSize size, bool recreateThumbnail)
{
    if (!style || style->name().isNull()) {
        return QImage();
    }
    if (!size.isValid() || size.isNull()) {
        size = d->defaultSize;
    }
    QString imageKey = "c_" + QString::number(style->styleId()) + "_" + QString::number(size.width()) + "_" + QString::number(size.height());

    if (!recreateThumbnail && d->thumbnailCache.object(imageKey)) {
        return QImage(*(d->thumbnailCache.object(imageKey)));
    }

    QImage *im = new QImage(size.width(), size.height(), QImage::Format_ARGB32_Premultiplied);
    im->fill(QColor(Qt::transparent).rgba());

    KoCharacterStyle *clone = style->clone();
    QTextCursor cursor(d->thumbnailHelperDocument);
    QTextCharFormat format;
    clone->applyStyle(format);
    cursor.select(QTextCursor::Document);
    cursor.setBlockFormat(QTextBlockFormat());
    cursor.setBlockCharFormat(QTextCharFormat());
    cursor.setCharFormat(QTextCharFormat());
    cursor.insertText(clone->name(), format);

    layoutThumbnail(size, im);

    d->thumbnailCache.insert(imageKey, im);
    delete clone;
    return QImage(*im);
}

void KoStyleThumbnailer::setThumbnailSize(QSize size)
{
    d->defaultSize = size;
}

void KoStyleThumbnailer::layoutThumbnail(QSize size, QImage *im)
{
    QPainter p(im);
    d->documentLayout->removeRootArea();
    KoTextLayoutRootArea rootArea(d->documentLayout);
    rootArea.setReferenceRect(0, size.width(), 0, 1E6);
    rootArea.setNoWrap(1E6);

    FrameIterator frameCursor(d->thumbnailHelperDocument->rootFrame());
    rootArea.layoutRoot(&frameCursor);

    QSizeF documentSize = rootArea.boundingRect().size();
    if (documentSize.width() > size.width() || documentSize.height() > size.height()) {
        //calculate the space needed for the font size indicator (should the preview big too big with the style's font size
        QTextCursor cursor(d->thumbnailHelperDocument);
        cursor.select(QTextCursor::Document);
        QString sizeHint = "\t" + QString::number(cursor.charFormat().fontPointSize()) + "pt";
        p.save();
        QFont sizeHintFont = p.font();
        sizeHintFont.setPointSize(8);
        p.setFont(sizeHintFont);
        QRectF sizeHintRect(p.boundingRect(0, 0, 1, 1, Qt::AlignCenter, sizeHint));
        p.restore();
        //calculate the font reduction factor so that the text + the sizeHint fits
        qreal reductionFactor = qMin((size.width()-sizeHintRect.width())/documentSize.width(), size.height()/documentSize.height());
        QTextCharFormat fmt = cursor.charFormat();
        fmt.setFontPointSize((int)(fmt.fontPointSize()*reductionFactor));
        cursor.mergeCharFormat(fmt);

        frameCursor = FrameIterator(d->thumbnailHelperDocument->rootFrame());
        rootArea.setReferenceRect(0, size.width()-sizeHintRect.width(), 0, 1E6);
        rootArea.setNoWrap(1E6);
        rootArea.layoutRoot(&frameCursor);
        documentSize = rootArea.boundingRect().size();
        //center the preview in the pixmap
        qreal yOffset = (size.height()-documentSize.height())/2;
        if (yOffset) {
            p.translate(0, yOffset);
        }
        KoTextDocumentLayout::PaintContext pc;
        rootArea.paint(&p, pc);
        if (yOffset) {
            p.translate(0, -yOffset);
        }
        p.save();
        p.setFont(sizeHintFont);
        p.drawText(QRectF(size.width()-sizeHintRect.width(), 0, sizeHintRect.width(),
                          size.height() /*because we want to be vertically centered in the pixmap, like the style name*/),Qt::AlignCenter, sizeHint);
        p.restore();
    }
    else {
        //center the preview in the pixmap
        qreal yOffset = (size.height()-documentSize.height())/2;
        if (yOffset) {
            p.translate(0, yOffset);
        }

        KoTextDocumentLayout::PaintContext pc;
        rootArea.paint(&p, pc);
    }
}
