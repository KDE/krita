/* This file is part of the KDE project
 * Copyright (C) 2007-2009 Jan Hambrecht <jaham@gmx.net>
 * Copyright (C) 2008 Rob Buis <buis@kde.org>
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

#include "ArtisticTextRange.h"
#include <KDebug>

ArtisticTextRange::ArtisticTextRange(const QString &text, const QFont &font)
    : m_text(text), m_font(font)
{

}

ArtisticTextRange::~ArtisticTextRange()
{
}

void ArtisticTextRange::setText(const QString &text)
{
    m_text = text;
}

QString ArtisticTextRange::text() const
{
    return m_text;
}

void ArtisticTextRange::insertText(int charIndex, const QString &text)
{
    m_text.insert(charIndex, text);
}

void ArtisticTextRange::appendText(const QString &text)
{
    m_text += text;
}

void ArtisticTextRange::setFont( const QFont & font )
{
    if( m_font == font )
        return;

    m_font = font;
}

QFont ArtisticTextRange::font() const
{
    return m_font;
}

void ArtisticTextRange::append(const ArtisticTextRange &range)
{
    m_text += range.text();
    m_xOffsets += range.m_xOffsets;
    m_yOffsets += range.m_yOffsets;
}

ArtisticTextRange ArtisticTextRange::extract(int from, int count)
{
    ArtisticTextRange extracted(m_text.mid(from, count), m_font);
    extracted.setXOffsets(m_xOffsets.mid(from, count), m_xOffsetType);
    extracted.setYOffsets(m_yOffsets.mid(from, count), m_yOffsetType);

    m_text.remove(from, count);
    m_xOffsets = m_xOffsets.mid(0, from);
    m_yOffsets = m_yOffsets.mid(0, from);

    return extracted;
}

bool ArtisticTextRange::hasEqualStyle(const ArtisticTextRange &other) const
{
    return m_font == other.m_font;
}

void ArtisticTextRange::setXOffsets(const QList<qreal> &offsets, OffsetType type)
{
    m_xOffsets = offsets;
    m_xOffsetType = type;
}

void ArtisticTextRange::setYOffsets(const QList<qreal> &offsets, OffsetType type)
{
    m_yOffsets = offsets;
    m_yOffsetType = type;
}

qreal ArtisticTextRange::xOffset(int charIndex) const
{
    return m_xOffsets.value(charIndex);
}

qreal ArtisticTextRange::yOffset(int charIndex) const
{
    return m_yOffsets.value(charIndex);
}

bool ArtisticTextRange::hasXOffset(int charIndex) const
{
    return charIndex >= 0 && charIndex < m_xOffsets.count();
}

bool ArtisticTextRange::hasYOffset(int charIndex) const
{
    return charIndex >= 0 && charIndex < m_yOffsets.count();
}

ArtisticTextRange::OffsetType ArtisticTextRange::xOffsetType() const
{
    return m_xOffsetType;
}

ArtisticTextRange::OffsetType ArtisticTextRange::yOffsetType() const
{
    return m_yOffsetType;
}

void ArtisticTextRange::printDebug() const
{
    kDebug() << "text:" << m_text;
    kDebug() << "font:" << m_font;
    switch(m_xOffsetType) {
    case AbsoluteOffset:
        kDebug() << "x:" << m_xOffsets;
        break;
    case RelativeOffset:
        kDebug() << "dx:" << m_xOffsets;
        break;
    }
    switch(m_yOffsetType) {
    case AbsoluteOffset:
        kDebug() << "y:" << m_yOffsets;
        break;
    case RelativeOffset:
        kDebug() << "dy:" << m_yOffsets;
        break;
    }
}
