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
#ifndef MUSIC_CORE_CLEF_H
#define MUSIC_CORE_CLEF_H

#include "StaffElement.h"

namespace MusicCore {

class Staff;

/**
 * This class represents a clef music element. This same class is used both for clef changes at the start of bars as for
 * mid-bar clef changes.
 */
class Clef : public StaffElement {
    Q_OBJECT
public:
    /**
     * The various supported shapes for a clef. Currently ownly G, F and C clefs are supported.
     */
    enum ClefShape {
        GClef,
        FClef,
        CClef,
        Trebble = GClef,    ///< Convenience value if you can't remember what clef a trebble clef is
        Bass = FClef,       ///< Convenience value if you can't remember what clef a bass clef is
        Alto = CClef,       ///< Convenience value if you can't remember what clef an alto clef is
        Tenor = CClef,      ///< Convenience value if you can't remember what clef a tenor clef is
        Soprano = CClef     ///< Convenience value if you can't remember what clef a soprano clef is
    };

    /**
     * Convenience constructor that removes the need to call setStaff after creating the clef to set the staff.
     *
     * @param staff the staff on which the clef should be shown
     * @param shape the shape of the new clef
     * @param line the line at which to draw the clef
     * @param octaveChange the octaveChange to apply to notes following this clef
     */
    Clef(Staff* staff, int startTime, ClefShape shape, int line, int octaveChange = 0);
    
    /**
     * Destructor.
     */
    virtual ~Clef();

    /**
     * Returns the shape of the clef.
     */
    ClefShape shape() const;
    
    /**
     * Returns the line the clef is displayed on.
     */
    int line() const;
    
    /**
     * Returns the octave change of this clef.
     */
    int octaveChange() const;
    
    /**
     * Returns the pitch for a given line.
     */
    int lineToPitch(int line) const;
    
    /**
     * Returns the line for a given pitch.
     */
    int pitchToLine(int pitch) const;
    
    /**
     * Returns the priority of this staff element with regard to order in which it should be sorted.
     */
    virtual int priority() const;
public slots:
    /**
     * Changes the shape of the clef.
     *
     * @param shape the new shape of the clef
     */
    void setShape(ClefShape shape);

    /**
     * Changes the line the clef is displayed on. The bottom line is 1, each higher line +1.
     *
     * @param line the new line this clef is displayed on
     */
    void setLine(int line);
    
    /**
     * Changes the octave change of this clef.
     *
     * @param octaveChange the new octave change for this clef
     */
    void setOctaveChange(int octaveChange);
signals:
    /**
     * This signal is emitted whenever the shape of the clef changes.
     */
    void shapeChanged(ClefShape shape);
    
    /**
     * This signal is emitted whenever the line of the clef changes.
     */
    void lineChanged(int line);
    
    /**
     * This signal is emitted whenever the octaveChange of the clef changes.
     */
    void octaveChangeChanged(int octaveChange);
private:
    class Private;
    Private * const d;
};

} // namespace MusicCore

#endif // MUSIC_CLEF_PART_H
