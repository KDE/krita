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
#include <QDebug>

ArtisticTextRange::ArtisticTextRange(const QString &text, const QFont &font)
    : m_text(text)
    , m_font(font)
    , m_letterSpacing(0.0)
    , m_wordSpacing(0.0)
    , m_baselineShift(None)
    , m_baselineShiftValue(0.0)
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

void ArtisticTextRange::setFont(const QFont &font)
{
    if (m_font == font) {
        return;
    }

    m_font = font;
}

QFont ArtisticTextRange::font() const
{
    return m_font;
}

ArtisticTextRange ArtisticTextRange::extract(int from, int count)
{
    // copy text and font
    ArtisticTextRange extracted(m_text.mid(from, count), m_font);
    // copy corresponding character transformations
    if (from < m_xOffsets.count()) {
        extracted.setXOffsets(m_xOffsets.mid(from, count), m_xOffsetType);
    }
    if (from < m_yOffsets.count()) {
        extracted.setYOffsets(m_yOffsets.mid(from, count), m_yOffsetType);
    }
    if (from < m_rotations.count()) {
        extracted.setRotations(m_rotations.mid(from, count));
    }

    extracted.setLetterSpacing(m_letterSpacing);
    extracted.setWordSpacing(m_wordSpacing);
    extracted.setBaselineShift(m_baselineShift, m_baselineShiftValue);

    // remove text
    m_text.remove(from, count < 0 ? m_text.length() - from : count);
    // remove character transformations
    m_xOffsets = m_xOffsets.mid(0, from);
    m_yOffsets = m_yOffsets.mid(0, from);
    m_rotations = m_rotations.mid(0, from);

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

bool ArtisticTextRange::hasXOffsets() const
{
    return !m_xOffsets.isEmpty();
}

bool ArtisticTextRange::hasYOffsets() const
{
    return !m_yOffsets.isEmpty();
}

ArtisticTextRange::OffsetType ArtisticTextRange::xOffsetType() const
{
    return m_xOffsetType;
}

ArtisticTextRange::OffsetType ArtisticTextRange::yOffsetType() const
{
    return m_yOffsetType;
}

void ArtisticTextRange::setRotations(const QList<qreal> &rotations)
{
    m_rotations = rotations;
}

bool ArtisticTextRange::hasRotation(int charIndex) const
{
    return charIndex >= 0 && charIndex < m_rotations.count();
}

bool ArtisticTextRange::hasRotations() const
{
    return !m_rotations.isEmpty();
}

qreal ArtisticTextRange::rotation(int charIndex) const
{
    return m_rotations.value(charIndex);
}

void ArtisticTextRange::setLetterSpacing(qreal letterSpacing)
{
    m_letterSpacing = letterSpacing;
}

qreal ArtisticTextRange::letterSpacing() const
{
    return m_letterSpacing;
}

void ArtisticTextRange::setWordSpacing(qreal wordSpacing)
{
    m_wordSpacing = wordSpacing;
}

qreal ArtisticTextRange::wordSpacing() const
{
    return m_wordSpacing;
}

ArtisticTextRange::BaselineShift ArtisticTextRange::baselineShift() const
{
    return m_baselineShift;
}

qreal ArtisticTextRange::baselineShiftValue() const
{
    return m_baselineShiftValue;
}

void ArtisticTextRange::setBaselineShift(BaselineShift mode, qreal value)
{
    m_baselineShift = mode;
    m_baselineShiftValue = value;
}

qreal ArtisticTextRange::subAndSuperScriptSizeFactor()
{
    return 0.58; // taken from wikipedia
}

void ArtisticTextRange::printDebug() const
{
    qDebug() << "text:" << m_text;
    qDebug() << "font:" << m_font;
    switch (m_xOffsetType) {
    case AbsoluteOffset:
        qDebug() << "x:" << m_xOffsets;
        break;
    case RelativeOffset:
        qDebug() << "dx:" << m_xOffsets;
        break;
    }
    switch (m_yOffsetType) {
    case AbsoluteOffset:
        qDebug() << "y:" << m_yOffsets;
        break;
    case RelativeOffset:
        qDebug() << "dy:" << m_yOffsets;
        break;
    }
    qDebug() << "rotate:" << m_rotations;
}
