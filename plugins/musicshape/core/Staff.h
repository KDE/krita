/* This file is part of the KDE project
 * Copyright (C) 2007 Marijn Kruisselbrink <mkruisselbrink@kde.org>
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
#ifndef MUSIC_CORE_STAFF_H
#define MUSIC_CORE_STAFF_H

#include <QObject>
#include <QString>

namespace MusicCore {

class Part;
class Clef;
class Bar;
class KeySignature;
class TimeSignature;

/**
 * A Staff is purely used for displaying/formatting. The staff class is used to indicate on what staff
 * music elements should be printed, but does only contain information regarding the formatting.
 */
class Staff : QObject
{
    Q_OBJECT
public:
    /**
     * Creates a new staff in the given part. This does not actually add the staff to the part, to do that call the
     * addStaff of the part.
     *
     * @param part the part in which to create a staff
     */
    explicit Staff(Part *part);

    /**
     * Destructor.
     */
    ~Staff();

    /**
     * Returns the part this staff is part of.
     */
    Part* part();

    /**
     * Sets the part this staff is part of. You should not call this method after adding the staff to a part using the
     * addStaff method in the Part class.
     *
     * @param part the part that this staff is part of
     */
    void setPart(Part* part);

    /**
     * Returns the spacing in points between this staff and the staff above it.
     */
    qreal spacing() const;

    /**
     * Returns the vertical position of this staff relative to the top of the staff system.
     */
    qreal top();
    qreal center();
    qreal bottom();

    /**
     * Returns the number of lines in this staff. Typical values are 5 for normal music, or 1 for a percussion part.
     * The default value for this attribute is 5.
     */
    int lineCount() const;

    /**
     * Returns the distance in points between two lines of this staff.
     */
    qreal lineSpacing() const;

    /**
     * Returns the closest line corresponding to the given y coordinate, where the coordinate should be relative
     * to the top of the staff. 0 Is the bottom line, 2 the line above it, and so on.
     *
     * @param y the coordinate for which to return the closest line.
     */
    int line(qreal y) const;

    /**
     * Returns the last Clef element in this staff that is at or before the given time in the given bar. If oldClef is
     * specified and no clef changes are found in the given bar, this method returns oldClef instead of searching for
     * older clef changes.
     */
    Clef* lastClefChange(int bar, int time = -1, Clef* oldClef = 0);
    Clef* lastClefChange(Bar* bar, int time = -1, Clef* oldClef = 0);
    KeySignature* lastKeySignatureChange(int bar);
    KeySignature* lastKeySignatureChange(Bar* bar);
    TimeSignature* lastTimeSignatureChange(int bar);
    TimeSignature* lastTimeSignatureChange(Bar* bar);

    void updateAccidentals(int fromBar = 0);
    void updateAccidentals(Bar* fromBar);
public slots:
    /**
     * Set the spacing between this staff and the staff above it.
     *
     * @param spacing the new spacing.
     */
    void setSpacing(qreal spacing);

    /**
     * Sets the number of lines of this staff.
     *
     * @param lineCount the new number of lines in this staff.
     */
    void setLineCount(int lineCount);

    /**
     * Sets the distance in points between two lines of this staff.
     *
     * @param lineSpacing the new distance between two lines.
     */
    void setLineSpacing(qreal lineSpacing);
signals:
    void spacingChanged(qreal spacing);
    void lineCountChanged(int lineCount);
    void lineSpacingChanged(qreal lineSpacing);
private:
    class Private;
    Private * const d;
};

} // namespace MusicCore

#endif // MUSIC_CORE_PART_H
