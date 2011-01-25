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
#include "Global.h"

namespace MusicCore {

int durationToTicks(Duration duration)
{
    switch (duration) {
        case HundredTwentyEighthNote: return Note128Length;
        case SixtyFourthNote:         return Note64Length;
        case ThirtySecondNote:        return Note32Length;
        case SixteenthNote:           return Note16Length;
        case EighthNote:              return Note8Length;
        case QuarterNote:             return QuarterLength;
        case HalfNote:                return HalfLength;
        case WholeNote:               return WholeLength;
        case BreveNote:               return DoubleWholeLength;
    }
    return 0;
}

QString durationToString(Duration duration)
{
    switch (duration) {
        case HundredTwentyEighthNote:   return "128th";
        case SixtyFourthNote:           return "64th";
        case ThirtySecondNote:          return "32nd";
        case SixteenthNote:             return "16th";
        case EighthNote:                return "eighth";
        case QuarterNote:               return "quarter";
        case HalfNote:                  return "half";
        case WholeNote:                 return "whole";
        case BreveNote:                 return "breve";
    }
    return "[unknown note length]";
}

} // namespace MusicCore
