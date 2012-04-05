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
#ifndef MUSIC_CORE_GLOBAL_H
#define MUSIC_CORE_GLOBAL_H

#include <QString>

namespace MusicCore {
    /**
     * This enum contains constants representing the lengths of various notes. The length of a 128th note is
     * 1*2*3*4*5*7 ticks to allow n-tuples with n from 3..10. The length of the other notes are mutliples of
     * this number. All durations and time-stamps are calculated in these units.
     */
    enum NoteLength {
        Note128Length = 1*2*3*4*5*7,
        Note64Length = Note128Length * 2,
        Note32Length = Note64Length * 2,
        Note16Length = Note32Length * 2,
        Note8Length = Note16Length * 2,
        QuarterLength = Note8Length * 2,
        HalfLength = QuarterLength * 2,
        WholeLength = HalfLength * 2,
        DoubleWholeLength = WholeLength * 2
    };
    
    /**
     * This enum represents the various supported durations for chords/rests.
     */
    enum Duration {
        HundredTwentyEighthNote,
        SixtyFourthNote,
        ThirtySecondNote,
        SixteenthNote,
        EighthNote,
        QuarterNote,
        HalfNote,
        WholeNote,
        BreveNote
    };
    
    enum StemDirection {
        StemUp,
        StemDown
    };
    
    enum BeamType {
        BeamStart,
        BeamContinue,
        BeamEnd,
        BeamFlag,
        BeamForwardHook,
        BeamBackwardHook
    };
    
    
    /**
     * Convert a duration to a number of ticks.
     *
     * @param duration the duration to convert to ticks
     */
    int durationToTicks(Duration duration);
    
    /**
     * Concert a duration to a string representation as it is expected when written to a MusicXML file.
     *
     * @param duration the duration to convert to a string
     */
    QString durationToString(Duration duration);
    
    
} // namespace MusicCore

#endif // MUSIC_CORE_GLOBAL_H
