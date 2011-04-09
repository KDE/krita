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

#include <QtCore/QString>
#include <QtGui/QFont>

/// Represents a range of characters with the same text properties
class ArtisticTextRange
{
public:
    ArtisticTextRange(const QString &text, const QFont &font);
    ~ArtisticTextRange();

    /// Returns the text content
    QString text() const;

    /// Sets the text to display
    void setText(const QString &text);

    /// Inserts new text at the given position
    void insertText(int index, const QString &text);

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

    /// Appends another text range to this range
    void append(const ArtisticTextRange &range);

    /// Extracts specified part of the text range
    ArtisticTextRange extract(int from, int count);

    /// Checks if specified text range has the same style as this text range
    bool hasEqualStyle(const ArtisticTextRange &other) const;

private:
    QString m_text; ///< the text of the text range
    QFont m_font; ///< the font of the text range
};

#endif // ARTISITICTEXTRANGE_H
