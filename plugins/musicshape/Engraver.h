/* This file is part of the KDE project
 * Copyright (C) 2007 Marijn Kruisselbrink <m.kruisselbrink@student.tue.nl>
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
#ifndef MUSIC_ENGRAVER_H
#define MUSIC_ENGRAVER_H

#include <QtCore/QSizeF>

namespace MusicCore {
    class Bar;
    class VoiceBar;
    class Sheet;
    class Part;
}

/**
 * This class is responsible for layint out notes and other musical elements inside a bar, and for laying out bars
 * withing staff systems.
 */
class Engraver {
    public:
        Engraver();
        void engraveSheet(MusicCore::Sheet* sheet, int firstSystem, QSizeF size, bool engraveBars = true, int* lastSystem=0);
        void engraveBar(MusicCore::Bar* bar);
        void rebeamBar(MusicCore::Part* part, MusicCore::VoiceBar* bar);
};

#endif // MUSIC_ENGRAVER_H
