/* This file is part of the KDE project
 * Copyright (C) 2007 Thomas Zander <zander@kde.org>
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
#include "KoInlineNote.h"

#include <QString>
#include <QTextInlineObject>
#include <QFontMetricsF>
#include <QTextOption>

class KoInlineNote::Private {
public:
    Private(KoInlineNote::Type t) : autoNumbering(false), type(t) {}
    QString text, label;
    bool autoNumbering;
    KoInlineNote::Type type;
};

KoInlineNote::KoInlineNote(Type type)
    : d(new Private(type))
{
}

KoInlineNote::~KoInlineNote() {
    delete d;
}

void KoInlineNote::setText(const QString &text) {
    d->text = text;
}

void KoInlineNote::setLabel(const QString &text) {
    d->label = text;
}

QString KoInlineNote::text() const {
    return d->text;
}

QString KoInlineNote::label() const {
    return d->label;
}

bool KoInlineNote::autoNumbering() const {
    return d->autoNumbering;
}

void KoInlineNote::setAutoNumbering(bool on) {
    d->autoNumbering = on;
}

KoInlineNote::Type KoInlineNote::type() const {
    return d->type;
}

void KoInlineNote::updatePosition(const QTextDocument *document, QTextInlineObject object, int posInDocument, const QTextCharFormat &format) {
    Q_UNUSED(document);
    Q_UNUSED(object);
    Q_UNUSED(posInDocument);
    Q_UNUSED(format);
}

void KoInlineNote::resize(const QTextDocument *document, QTextInlineObject object, int posInDocument, const QTextCharFormat &format, QPaintDevice *pd) {
    if(d->label.isEmpty())
        return;
    Q_ASSERT(format.isCharFormat());
    QFontMetricsF fm(format.font(), pd);
    object.setWidth( fm.width(d->label) );
    object.setAscent(fm.ascent());
    object.setDescent(fm.descent());
}

void KoInlineNote::paint (QPainter &painter, QPaintDevice *pd, const QTextDocument *document, const QRectF &rect, QTextInlineObject object, int posInDocument, const QTextCharFormat &format) {
    Q_UNUSED(document);
    Q_UNUSED(object);
    Q_UNUSED(posInDocument);

    if(d->label.isEmpty())
        return;

    QFont font(format.font(), pd);
    QTextLayout layout(d->label, font, pd);
    layout.setCacheEnabled(true);
    QList<QTextLayout::FormatRange> layouts;
    QTextLayout::FormatRange range;
    range.start=0;
    range.length=d->label.length();
    range.format = format;
    range.format.setVerticalAlignment(QTextCharFormat::AlignSuperScript);
    layouts.append(range);
    layout.setAdditionalFormats(layouts);

    QTextOption option(Qt::AlignLeft | Qt::AlignAbsolute);
    option.setTextDirection(object.textDirection());
    layout.setTextOption(option);
    layout.beginLayout();
    layout.createLine();
    layout.endLayout();
    layout.draw(&painter, rect.topLeft());
}

