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
#ifndef MUSIC_CORE_TIMESIGNATURE_H
#define MUSIC_CORE_TIMESIGNATURE_H

#include "StaffElement.h"

namespace MusicCore {

class Staff;

/**
 * This class represents a time signature.
 */
class TimeSignature : public StaffElement
{
    Q_OBJECT
public:
    enum TimeSignatureType {
        Classical,
        Number
    };

    /**
     * Create a new time signature instance and place it on a specified staff.
     */
    TimeSignature(Staff* staff, int startTime, int beats, int beat, TimeSignatureType type = Classical);

    /**
     * Destructor.
     */
    virtual ~TimeSignature();

    /**
     * Returns the number of beats in this time signature.
     */
    int beats() const;

    /**
     * Returns the beat type of the time signature.
     */
    int beat() const;

    /**
     * Returns the type of the time signature.
     */
    TimeSignatureType type() const;
    virtual int priority() const;
    
    QList<int> beatLengths() const;
public slots:
    /**
     * Changes the number of beats in this time signature.
     *
     * @param beats the new number of beats in the time signature
     */
    void setBeats(int beats);

    /**
     * Changes the beat type of the time signature.
     *
     * @param beat the new beat type
     */
    void setBeat(int beat);

    /**
     * Changes the type of the time signature.
     */
    void setType(TimeSignatureType type);
signals:
    void beatsChanged(int beats);
    void beatChanged(int beat);
    void typeChanged(TimeSignatureType type);
private:
    class Private;
    Private * const d;
};

} // namespace MusicCore

#endif // MUSIC_CORE_TIMESIGNATURE_H
