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
#ifndef MUSIC_CORE_VOICE_H
#define MUSIC_CORE_VOICE_H

#include <QObject>
#include <QString>

namespace MusicCore {

class Part;
class VoiceBar;
class Bar;

/**
 * A voice contains the actual musical elements in a piece of music.
 */
class Voice : public QObject
{
    Q_OBJECT
public:
    /**
     * Creates a new voice for the given part. This does not actually add the voice to the part, for that call the
     * addVoice method of the part.
     *
     * @param part the part this voice belongs to
     */
    explicit Voice(Part* part);

    /**
     * Destructor.
     */
    ~Voice();

    /**
     * Returns the part this voice is part of.
     */
    Part* part();

    /**
     * Sets the part this voice belongs to. Do not call this method when the voice already is added to a part.
     *
     * @param part the new part this voice belongs to
     */
    void setPart(Part* part);

    /**
     * Returns the bar in this voice that contains the elements in the given bar in this piece of music.
     *
     * @param bar the bar for which to return the VoiceBar instance.
     */
    VoiceBar* bar(Bar* bar);

    VoiceBar* bar(int barIdx);
private:
    class Private;
    Private * const d;
};

} // namespace MusicCore

#endif // MUSIC_CORE_VOICE_H
