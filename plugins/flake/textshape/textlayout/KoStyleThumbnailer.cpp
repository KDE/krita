/* This file is part of the KDE project
 * Copyright (C) 2006, 2009 Thomas Zander <zander@kde.org>
 * Copyright (C) 2007 Sebastian Sauer <mail@dipe.org>
 * Copyright (C) 2007 Pierre Ducroquet <pinaraf@gmail.com>
 * Copyright (C) 2008 Girish Ramakrishnan <girish@forwardbias.in>
 * Copyright (C) 2009,2011 KO GmbH <cbo@kogmbh.com>
 * Copyright (C) 2011-2012 Pierre Stirnweiss <pstirnweiss@googlemail.com>
 * Copyright (C) 2012 Gopalakrishna Bhat A <gopalakbhat@gmail.com>
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
#include "KoTextDocumentLayout.h"
#include "KoTextLayoutRootArea.h"
#include "FrameIterator.h"

#include <stdint.h>

#include <klocalizedstring.h>

#include <QCache>
#include <QFont>
#include <QImage>
#include <QPainter>
#include <QRect>
#include <QTextBlock>
#include <QTextCursor>
#include <QTextLength>

#include <TextLayoutDebug.h>

extern int qt_defaultDpiX();
extern int qt_defaultDpiY();

class Q_DECL_HIDDEN KoStyleThumbnailer::Private
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
    QString thumbnailText;
};

KoStyleThumbnailer::KoStyleThumbnailer()
        : d(new Private())
{
}

KoStyleThumbnailer::~KoStyleThumbnailer()
{
    delete d;
}

QImage KoStyleThumbnailer::thumbnail(KoParagraphStyle *style, const QSize &_size, bool recreateThumbnail, KoStyleThumbnailerFlags flags)
{
    if ((flags & UseStyleNameText)  && (!style || style->name().isNull())) {
        return QImage();
    } else if ((! (flags & UseStyleNameText)) && d->thumbnailText.isEmpty()) {
        return QImage();
    }

    const QSize &size = (!_size.isValid() || _size.isNull()) ? d->defaultSize : _size;

    QString imageKey = "p_" + QString::number(reinterpret_cast<uintptr_t>(style)) + "_" + QString::number(size.width()) + "_" + QString::number(size.height());

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
    // Default to black as text color, to match what KoTextLayoutArea::paint(...)
    // does, setting solid black if no brush is set. Otherwise the UI text color
    // would be used, which might be too bright with dark UI color schemes
    format.setForeground(QColor(Qt::black));
    clone->KoCharacterStyle::applyStyle(format);
    if (flags & UseStyleNameText) {
        cursor.insertText(clone->name(), format);
    } else {
        cursor.insertText(d->thumbnailText, format);
    }
    layoutThumbnail(size, im, flags);
    // Make a copy of the image before inserting in the cache
    QImage res = QImage(*im);
    // Because on inserting, QCache can decide to delete the object immediately
    d->thumbnailCache.insert(imageKey, im);

    delete clone;
    return res;
}

QImage KoStyleThumbnailer::thumbnail(KoCharacterStyle *characterStyle, KoParagraphStyle *paragraphStyle, const QSize &_size, bool recreateThumbnail, KoStyleThumbnailerFlags flags)
{
    if ((flags & UseStyleNameText)  && (!characterStyle || characterStyle->name().isNull())) {
        return QImage();
    } else if ((! (flags & UseStyleNameText)) && d->thumbnailText.isEmpty()) {
        return QImage();
    }
    else if (characterStyle == 0) {
        return QImage();
    }

    const QSize &size = (!_size.isValid() || _size.isNull()) ? d->defaultSize : _size;

    QString imageKey = "c_" + QString::number(reinterpret_cast<uintptr_t>(characterStyle)) + "_"
                     + "p_" + QString::number(reinterpret_cast<uintptr_t>(paragraphStyle)) + "_"
                     + QString::number(size.width()) + "_" + QString::number(size.height());

    if (!recreateThumbnail && d->thumbnailCache.object(imageKey)) {
        return QImage(*(d->thumbnailCache.object(imageKey)));
    }

    QImage *im = new QImage(size.width(), size.height(), QImage::Format_ARGB32_Premultiplied);
    im->fill(QColor(Qt::transparent).rgba());

    QTextCursor cursor(d->thumbnailHelperDocument);
    QTextCharFormat format;
    // Default to black as text color, to match what KoTextLayoutArea::paint(...)
    // does, setting solid black if no brush is set. Otherwise the UI text color
    // would be used, which might be too bright with dark UI color schemes
    format.setForeground(QColor(Qt::black));
    KoCharacterStyle *characterStyleClone = characterStyle->clone();
    characterStyleClone->applyStyle(format);
    cursor.select(QTextCursor::Document);
    cursor.setBlockFormat(QTextBlockFormat());
    cursor.setBlockCharFormat(QTextCharFormat());
    cursor.setCharFormat(QTextCharFormat());

    if (paragraphStyle) {
        KoParagraphStyle *paragraphStyleClone = paragraphStyle->clone();
       // paragraphStyleClone->KoCharacterStyle::applyStyle(format);
        QTextBlock block = cursor.block();
        paragraphStyleClone->applyStyle(block, true);
        delete paragraphStyleClone;
        paragraphStyleClone = 0;
    }

    if (flags & UseStyleNameText) {
        cursor.insertText(characterStyleClone->name(), format);
    } else {
        cursor.insertText(d->thumbnailText, format);
    }

    layoutThumbnail(size, im, flags);
    QImage res = QImage(*im);
    d->thumbnailCache.insert(imageKey, im);
    delete characterStyleClone;
    return res;
}

void KoStyleThumbnailer::setThumbnailSize(const QSize &size)
{
    d->defaultSize = size;
}

void KoStyleThumbnailer::layoutThumbnail(const QSize &size, QImage *im, KoStyleThumbnailerFlags flags)
{
    QPainter p(im);
    d->documentLayout->removeRootArea();
    KoTextLayoutRootArea rootArea(d->documentLayout);
    rootArea.setReferenceRect(0, size.width() * 72.0 / qt_defaultDpiX(), 0, 1E6);
    rootArea.setNoWrap(1E6);

    FrameIterator frameCursor(d->thumbnailHelperDocument->rootFrame());
    rootArea.layoutRoot(&frameCursor);

    QSizeF documentSize = rootArea.boundingRect().size();
    documentSize.setWidth(documentSize.width() * qt_defaultDpiX() / 72.0);
    documentSize.setHeight(documentSize.height() * qt_defaultDpiY() / 72.0);
    if (documentSize.width() > size.width() || documentSize.height() > size.height()) {
        //calculate the space needed for the font size indicator (should the preview be too big with the style's font size
        QTextCursor cursor(d->thumbnailHelperDocument);
        cursor.select(QTextCursor::Document);
        QString sizeHint = "\t" + QString::number(cursor.charFormat().fontPointSize()) + "pt";
        p.save();
        QFont sizeHintFont = p.font();
        sizeHintFont.setPointSize(8);
        p.setFont(sizeHintFont);
        QRectF sizeHintRect(p.boundingRect(0, 0, 1, 1, Qt::AlignCenter, sizeHint));
        p.restore();
        qreal width = qMax<qreal>(0., size.width()-sizeHintRect.width());

        QTextCharFormat fmt = cursor.charFormat();
        if (flags & ScaleThumbnailFont) {
            //calculate the font reduction factor so that the text + the sizeHint fits
            qreal reductionFactor = qMin(width/documentSize.width(), size.height()/documentSize.height());

            fmt.setFontPointSize((int)(fmt.fontPointSize()*reductionFactor));
        }

        cursor.mergeCharFormat(fmt);

        frameCursor = FrameIterator(d->thumbnailHelperDocument->rootFrame());
        rootArea.setReferenceRect(0, width * 72.0 / qt_defaultDpiX(), 0, 1E6);
        rootArea.setNoWrap(1E6);
        rootArea.layoutRoot(&frameCursor);
        documentSize = rootArea.boundingRect().size();
        documentSize.setWidth(documentSize.width() * qt_defaultDpiX() / 72.0);
        documentSize.setHeight(documentSize.height() * qt_defaultDpiY() / 72.0);
        //center the preview in the pixmap
        qreal yOffset = (size.height()-documentSize.height())/2;
        p.save();
        if ((flags & CenterAlignThumbnail) && yOffset) {
            p.translate(0, yOffset);
        }

        p.scale(qt_defaultDpiX() / 72.0, qt_defaultDpiY() / 72.0);

        KoTextDocumentLayout::PaintContext pc;
        rootArea.paint(&p, pc);

        p.restore();

        p.setFont(sizeHintFont);
        p.drawText(QRectF(size.width()-sizeHintRect.width(), 0, sizeHintRect.width(),
                          size.height() /*because we want to be vertically centered in the pixmap, like the style name*/),Qt::AlignCenter, sizeHint);
    }
    else {
        //center the preview in the pixmap
        qreal yOffset = (size.height()-documentSize.height())/2;
        if ((flags & CenterAlignThumbnail) && yOffset) {
            p.translate(0, yOffset);
        }

        p.scale(qt_defaultDpiX() / 72.0, qt_defaultDpiY() / 72.0);

        KoTextDocumentLayout::PaintContext pc;
        rootArea.paint(&p, pc);
    }
}

void KoStyleThumbnailer::removeFromCache(KoParagraphStyle *style)
{
    QString imageKey = "p_" + QString::number(reinterpret_cast<uintptr_t>(style)) + "_";
    removeFromCache(imageKey);
}

void KoStyleThumbnailer::removeFromCache(KoCharacterStyle *style)
{
    QString imageKey = "c_" + QString::number(reinterpret_cast<uintptr_t>(style)) + "_";
    removeFromCache(imageKey);
}

void KoStyleThumbnailer::setText(const QString &text)
{
    d->thumbnailText = text;
}

void KoStyleThumbnailer::removeFromCache(const QString &expr)
{
    QList<QString> keys = d->thumbnailCache.keys();
    foreach (const QString &key, keys) {
        if (key.contains(expr)) {
            d->thumbnailCache.remove(key);
        }
    }
}
