/* This file is part of the KDE project
 * Copyright (C) 2006 Thomas Zander <zander@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; version 2.
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
#include "KoCharacterStyle.h"

#include "Styles_p.h"

#include <QTextBlock>
#include <QTextCursor>

KoCharacterStyle::KoCharacterStyle(QObject *parent)
    : QObject(parent)
{
    m_stylesPrivate = new StylePrivate();
    setFontPointSize(12.0);
    setFontWeight(QFont::Normal);
    setVerticalAlignment(QTextCharFormat::AlignNormal);
    setTextOutline(QPen(Qt::NoPen));
}

KoCharacterStyle::~KoCharacterStyle() {
}

void KoCharacterStyle::setProperty(int key, const QVariant &value) {
    m_stylesPrivate->add(key, value);
}

const QVariant *KoCharacterStyle::get(int key) const {
    return m_stylesPrivate->get(key);
}

double KoCharacterStyle::propertyDouble(int key) const {
    const QVariant *variant = get(key);
    if(variant == 0)
        return 0.0;
    return variant->toDouble();
}

QPen KoCharacterStyle::textOutline () const {
    const QVariant *variant = get(QTextFormat::TextOutline);
    if(variant == 0) {
        QPen pen(Qt::NoPen);
        return pen;
    }
    return qvariant_cast<QPen>(variant);
}

QColor KoCharacterStyle::underlineColor () const {
    const QVariant *variant = get(QTextFormat::TextUnderlineColor);
    if(variant == 0) {
        QColor color;
        return color;
    }
    return qvariant_cast<QColor>(variant);
}

QBrush KoCharacterStyle::background() const {
    const QVariant *variant = get(QTextFormat::BackgroundBrush);
    if(variant == 0) {
        QBrush brush;
        return brush;
    }
    return qvariant_cast<QBrush>(variant);
}

void KoCharacterStyle::clearBackground() {
    m_stylesPrivate->remove(QTextCharFormat::BackgroundBrush);
}

QBrush KoCharacterStyle::foreground() const {
    const QVariant *variant = get(QTextFormat::ForegroundBrush);
    if(variant == 0) {
        QBrush brush;
        return brush;
    }
    return qvariant_cast<QBrush>(variant);
}

void KoCharacterStyle::clearForeground() {
    m_stylesPrivate->remove(QTextCharFormat::ForegroundBrush);
}

int KoCharacterStyle::propertyInt(int key) const {
    const QVariant *variant = get(key);
    if(variant == 0)
        return 0;
    return variant->toInt();
}

bool KoCharacterStyle::propertyBoolean(int key) const {
    const QVariant *variant = get(key);
    if(variant == 0)
        return false;
    return variant->toBool();
}

void KoCharacterStyle::applyStyle(QTextCharFormat &format) const {
    // copy all relevant properties.
    static const int properties[] = {
        StyleId,
        QTextFormat::FontPointSize,
        -1
    };

    int i=0;
    while(properties[i] != -1) {
        QVariant const *variant = get(properties[i]);
        if(variant) format.setProperty(properties[i], *variant);
        i++;
    }
}

void KoCharacterStyle::applyStyle(QTextBlock &block) const {
    QTextCursor cursor(block);
    QTextCharFormat cf = cursor.charFormat();
/*
    TODO make replacement of the style be a lot smarter.
    QTextBlock::Iterator fragmentIter = block.begin();
 */
    cursor.setPosition(block.position() + block.length()-1, QTextCursor::KeepAnchor);
    applyStyle(cf);
    cursor.mergeCharFormat(cf);
    cursor.setBlockCharFormat(cf);
}

void KoCharacterStyle::applyStyle(QTextCursor *selection) const {
    QTextCharFormat cf = selection->charFormat();
    applyStyle(cf);
    selection->mergeCharFormat(cf);
}

QString KoCharacterStyle::propertyString(int key) const {
    const QVariant *variant = m_stylesPrivate->get(key);
    if(variant == 0)
        return QString();
    return qvariant_cast<QString>(variant);
}

#include "KoCharacterStyle.moc"
