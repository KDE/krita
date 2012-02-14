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
#ifndef MUSIC_CORE_CHORD_H
#define MUSIC_CORE_CHORD_H

#include "VoiceElement.h"
#include "Global.h"

#include <QString>

namespace MusicCore {

class Note;

/**
 * A Chord is used to represent one or more notes that have the same duration and starting time and are in the same
 * voice. A Chord is also used to represent rests, when it has no notes. When a chord does not have any notes its
 * staff should be set to know on which staff to display the rest. When the chord contains notes the notes can have
 * their own staff, so you could have one chord spreading multiple staves (with all staves in the same part).
 */
class Chord : public VoiceElement
{
    Q_OBJECT
public:
    /**
     * Creates a new Chord instance, not specifying the staff on which the chord should be placed. Add this note to
     * a VoiceBar instance using the addElement method.
     *
     * @param duration the duration of the chord
     * @param dots the number of dots of the chord, each dot multiplies the length of the chord by 1.5
     */
    explicit Chord(Duration duration, int dots = 0);

    /**
     * This constructor is overloaded for convenience, to avoid having to call the setStaff method to set the staff
     * when creating rests.
     *
     * @param staff the staff on which the chord should be placed
     * @param duration the duration of the chord
     * @param dots the number of dots of the chord, each dot multiplies the length of the chord by 1.5
     */
    Chord(Staff* staff, Duration duration, int dots = 0);

    /**
     * Destructor.
     */
    virtual ~Chord();

    /**
     * Returns the duration of the chord.
     */
    Duration duration() const;

    /**
     * Returns the number of dots of this chord. Each dot multiplies the duration by a factor 1.5.
     */
    int dots() const;

    /**
     * Returns the number of notes in this chord.
     */
    int noteCount() const;

    /**
     * Returns the note at the given index in this chord.
     *
     * @param index the index of the note to return
     */
    Note* note(int index) const;

    /**
     * Adds a new note to this chord. The note will be drawn on the given staff and will have the given pitch and
     * accidentals.
     *
     * @param staff the staff the note should be drawn on
     * @param pitch the pitch of the new note
     * @param accidentals the number of accidentals of the note
     */
    Note* addNote(Staff* staff, int pitch, int accidentals = 0);

    /**
     * Adds an existing note to this chord. This will transfer ownership of the note to the chord. When the chord is
     * deleted, all notes in it are also deleted.
     *
     * @param note the note to add
     */
    void addNote(Note* note);

    /**
     * Removes a note from this chord. If deleteNote is true the note is not only removed, but also deleted.
     *
     * @param index the index of the note to remove
     * @param deleteNote should the note not only be removed, but also deleted
     */
    void removeNote(int index, bool deleteNote = true);

    /**
     * Removes a note from this chord. if deleteNote is true, the note is not only removed but also deleted.
     *
     * @param note the note to remove
     * @param deleteNote should the note also be deleted
     */
    void removeNote(Note* note, bool deleteNote = true);
    
    /**
     * This overrides the method in the VoiceElement class to return the correct y position based on pitch
     * of the notes this chord contains.
     */
    virtual qreal y() const;
    
    /**
     * This overrides the method in the VoiceElement class to return the correct height based on the pitch of
     * the notes in this chord.
     */
    virtual qreal height() const;
    virtual qreal width() const;
    virtual qreal beatline() const;
    
    qreal stemX() const;
    qreal centerX() const;
    qreal topNoteY() const;
    qreal bottomNoteY() const;
    qreal stemEndY(bool interpolateBeams = true) const;
    qreal beamDirection() const;

    StemDirection stemDirection() const;
    StemDirection desiredStemDirection() const;
    void setStemDirection(StemDirection direction);
    
    /**
     * Length of the stem as it extends beyond the top-most or bottom-most note, measured in number of lines.
     */
    qreal stemLength() const;
    void setStemLength(qreal stemLength);
    qreal desiredStemLength() const;
    
    int beamCount() const;
    const Chord* beamStart(int index) const;
    const Chord* beamEnd(int index) const;
    Chord* beamStart(int index);
    Chord* beamEnd(int index);
    BeamType beamType(int index) const;
    void setBeam(int index, Chord* beamStart, Chord* beamEnd, BeamType type = BeamFlag);
public slots:
    /**
     * Changes the duration of the chord.
     *
     * @param duration the new duration
     */
    void setDuration(Duration duration);

    /**
     * Changes the number of dots of the chord.
     *
     * @param dots the new number of dots
     */
    void setDots(int dots);
signals:    
    /**
     * This signal is emitted when the duration of this chord changes.
     */
    void durationChanged(Duration duration);
    
    /**
     * This signal is emitted when the number of dots of this chord changes.
     */
    void dotsChanged(int dots);
private:
    class Private;
    Private * const d;
};

} // namespace MusicCore

#endif // MUSIC_CORE_CHORD_H

