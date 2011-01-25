/* This file is part of the KDE project
 * Copyright (C) 2009 Marijn Kruisselbrink <mkruisselbrink@kde.org>
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
#include <QApplication>
#include <QFontDatabase>
#include <QDebug>
#include "MusicWidget.h"
#include "../core/Sheet.h"
#include "../core/Bar.h"
#include "../core/Part.h"
#include "../core/Clef.h"
#include "../core/Chord.h"
#include "../core/VoiceBar.h"

int main(int argc, char** argv)
{
    QApplication app(argc, argv);

    // Load the font that is used by the music widget
    QString fontFile = KDESRCDIR "/../fonts/Emmentaler-14.ttf";
    if (QFontDatabase::addApplicationFont(fontFile) == -1) {
        qWarning() << "Could not load emmentaler font";
    }

    MusicWidget w;
    // Setup a simple sheet, with one part exising of one staff and one voice, and a single bar
    MusicCore::Sheet* sheet = new MusicCore::Sheet(&w);
    MusicCore::Bar* bar = sheet->addBar();
    MusicCore::Part* part = sheet->addPart("Part 1");
    MusicCore::Staff* staff = part->addStaff();
    MusicCore::Voice* voice = part->addVoice();

    // Add a clef to the staff
    MusicCore::Clef* clef = new MusicCore::Clef(staff, 0, MusicCore::Clef::Trebble, 2, 0);
    bar->addStaffElement(clef);

    // And add some notes (middle C up-to C one octave higher)
    for (int i = 0; i < 8; i++) {
        MusicCore::Chord* chord = new MusicCore::Chord(MusicCore::HalfNote);
        MusicCore::Note* note = chord->addNote(staff, i); // central C
        Q_ASSERT(note);
        Q_UNUSED(note);
        bar->voice(voice)->addElement(chord);
    }
    w.setSheet(sheet);
    w.setScale(1.5);
    w.show();
    return app.exec();
}
