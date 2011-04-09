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

void ArtisticTextRange::insertText(int index, const QString &text)
{
    m_text.insert(index, text);
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
}

ArtisticTextRange ArtisticTextRange::extract(int from, int count)
{
    ArtisticTextRange extracted(m_text.mid(from, count), m_font);

    m_text.remove(from, count);

    return extracted;
}

bool ArtisticTextRange::hasEqualStyle(const ArtisticTextRange &other) const
{
    return m_font == other.m_font;
}
