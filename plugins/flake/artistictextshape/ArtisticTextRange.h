/* This file is part of the KDE project
 * Copyright (C) 2007-2008,2011 Jan Hambrecht <jaham@gmx.net>
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

#ifndef ARTISITICTEXTRANGE_H
#define ARTISITICTEXTRANGE_H

#include <QString>
#include <QPointF>
#include <QFont>

/// Represents a range of characters with the same text properties
class ArtisticTextRange
{
public:
    enum OffsetType {
        AbsoluteOffset,
        RelativeOffset
    };

    enum BaselineShift {
        None,     ///< no baseline shift
        Sub,      ///< subscript baseline shift
        Super,    ///< superscript baseline shift
        Percent,  ///< percentage baseline shift
        Length    ///< absolute baseline shift
    };

    ArtisticTextRange(const QString &text, const QFont &font);
    ~ArtisticTextRange();

    /// Returns the text content
    QString text() const;

    /// Sets the text to display
    void setText(const QString &text);

    /// Inserts new text at the given position
    void insertText(int charIndex, const QString &text);

    /// Appends text to the text range
    void appendText(const QString &text);

    /**
     * Sets the font used for drawing
     * Note that it is expected that the font has its point size set
     * in postscript points.
     */
    void setFont(const QFont &font);

    /// Returns the font
    QFont font() const;

    /// Extracts specified part of the text range
    ArtisticTextRange extract(int from, int count = -1);

    /// Checks if specified text range has the same style as this text range
    bool hasEqualStyle(const ArtisticTextRange &other) const;

    /// Sets indivdual character x-offsets
    void setXOffsets(const QList<qreal> &offsets, OffsetType type);

    /// Sets indivdual character y-offsets
    void setYOffsets(const QList<qreal> &offsets, OffsetType type);

    /// Returns the character x-offset for the specified character
    qreal xOffset(int charIndex) const;

    /// Returns the character y-offset for the specified character
    qreal yOffset(int charIndex) const;

    /// Checks if specified character has an x-offset value
    bool hasXOffset(int charIndex) const;

    /// Checks if specified character has an y-offset value
    bool hasYOffset(int charIndex) const;

    /// Checks if range has x-offsets
    bool hasXOffsets() const;

    /// Checks if range has y-offsets
    bool hasYOffsets() const;

    /// Returns the type of the x-offsets
    OffsetType xOffsetType() const;

    /// Returns the type of the y-offsets
    OffsetType yOffsetType() const;

    /// Sets individual character rotations
    void setRotations(const QList<qreal> &rotations);

    /// Checks if specified character has a rotation
    bool hasRotation(int charIndex) const;

    /// Checks if range has character rotations
    bool hasRotations() const;

    /// Returns the character rotation for the specified character
    qreal rotation(int charIndex) const;

    /// Sets additional spacing between characters
    void setLetterSpacing(qreal letterSpacing);

    /// Returns the letter spacing
    qreal letterSpacing() const;

    /// Sets additional spacing between words
    void setWordSpacing(qreal wordSpacing);

    /// Returns the word spacing
    qreal wordSpacing() const;

    /// Returns baseline shift mode
    BaselineShift baselineShift() const;

    /// Returns the optional baseline shift value
    qreal baselineShiftValue() const;

    /// Returns the normalized baseline shift value in point
    qreal baselineShiftValueNormalized() const;

    /// Sets baseline shift mode and optional value
    void setBaselineShift(BaselineShift mode, qreal value = 0.0);

    /// Returns the factor to calculate sub and super script font size
    static qreal subAndSuperScriptSizeFactor();

    /// Prints debug output
    void printDebug() const;

private:
    QString m_text; ///< the text of the text range
    QFont m_font; ///< the font of the text range
    QList<qreal> m_xOffsets; ///< character x-offsets
    QList<qreal> m_yOffsets; ///< character y-offsets
    OffsetType m_xOffsetType; ///< character x-offset type
    OffsetType m_yOffsetType; ///< character y-offset type
    QList<qreal> m_rotations; ///< character rotations
    qreal m_letterSpacing; ///< additional inter character spacing
    qreal m_wordSpacing; ///< additional inter word spacing
    BaselineShift m_baselineShift; ///< baseline shift mode
    qreal m_baselineShiftValue; ///< optional baseline shift value
};

#endif // ARTISITICTEXTRANGE_H
