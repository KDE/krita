/* This file is part of the KDE project
 * Copyright (C) 2011 Thorsten Zachmann <zachmann@kde.org>
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

#include "KoTextSoftPageBreak.h"

#include <KoShapeSavingContext.h>
#include <KoXmlReader.h>
#include <KoXmlWriter.h>

#include <QTextInlineObject>
#include <QPainter>

// Include Q_UNSUSED classes, for building on Windows
#include <KoShapeLoadingContext.h>

KoTextSoftPageBreak::KoTextSoftPageBreak()
{
}

KoTextSoftPageBreak::~KoTextSoftPageBreak()
{
}

bool KoTextSoftPageBreak::loadOdf(const KoXmlElement &element, KoShapeLoadingContext &context)
{
    Q_UNUSED(element)
    Q_UNUSED(context)
    return true;
}

void KoTextSoftPageBreak::saveOdf(KoShapeSavingContext &context)
{
    KoXmlWriter &writer = context.xmlWriter();
    writer.startElement("text:soft-page-break");
    writer.endElement();
}

void KoTextSoftPageBreak::updatePosition(const QTextDocument *document,
                                         int posInDocument, const QTextCharFormat &format)
{
    Q_UNUSED(document)
    Q_UNUSED(posInDocument)
    Q_UNUSED(format)
}

void KoTextSoftPageBreak::resize(const QTextDocument *document, QTextInlineObject &object,
                                 int posInDocument, const QTextCharFormat &format, QPaintDevice *pd)
{
    Q_UNUSED(document)
    Q_UNUSED(object)
    Q_UNUSED(posInDocument)
    Q_UNUSED(format)
    Q_UNUSED(pd)
    object.setWidth(0); // set the width to 0 as otherwise it is negative which results in the text being moved to left
    object.setAscent(0);
    object.setDescent(0);
}

void KoTextSoftPageBreak::paint(QPainter &painter, QPaintDevice *pd, const QTextDocument *document,
                                const QRectF &rect, const QTextInlineObject &object, int posInDocument, const QTextCharFormat &format)
{
    Q_UNUSED(painter)
    Q_UNUSED(pd)
    Q_UNUSED(document)
    Q_UNUSED(rect)
    Q_UNUSED(object)
    Q_UNUSED(posInDocument)
    Q_UNUSED(format)
    // TODO have a way to display the soft page break
}
