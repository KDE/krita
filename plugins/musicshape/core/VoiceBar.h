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
#ifndef MUSIC_CORE_VOICEBAR_H
#define MUSIC_CORE_VOICEBAR_H

#include <QObject>
#include <QRectF>

namespace MusicCore {

class Voice;
class Bar;
class VoiceElement;

/**
 * A VoiceBar contains the music elements in a specific voice in a specific bar. A VoiceBar is in many ways
 * simply a wrapper around a QList containging the actual music elements.
 */
class VoiceBar : public QObject
{
    Q_OBJECT
public:
    /**
     * Creates a new empty voice bar.
     */
    explicit VoiceBar(Bar* bar);

    /**
     * Destructor.
     */
    ~VoiceBar();

    Bar* bar();

    /**
     * Returns the number of elements in the bar.
     */
    int elementCount() const;

    /**
     * Returns the element at the given index in this bar.
     *
     * @param index the index of the element to return
     */
    VoiceElement* element(int index);

    int indexOfElement(VoiceElement* element);

    /**
     * Adds an element to this bar. You should not add an element to more than one bar, because when the bar is deleted
     * all elements in the bar are also deleted.
     *
     * @param element the element to add to this bar
     */
    void addElement(VoiceElement* element);

    /**
     * Inserts an element into this bar. You should not add an element to more than one bar, because when the bar is deleted
     * all elements in the bar are also deleted.
     *
     * @param element the element to insert into the bar
     * @param before the index of the element before which to insert the element
     */
    void insertElement(VoiceElement* element, int before);

    /**
     * Inserts an element into the bar. You should not add an element to more than one bar, because when the bar is deleted
     * all elements in the bar are also deleted.
     *
     * @param element the element to insert into the bar
     * @param before the element before which to insert the element
     */
    void insertElement(VoiceElement* element, VoiceElement* before);

    /**
     * Removes an element from this bar. If deleteElement is true, the element is not only removed but also deleted.
     *
     * @param index the index of the element to remove
     * @param deleteElement should the element not only be removed but also deleted
     */
    void removeElement(int index, bool deleteElement = true);

    /**
     * Removes an element from this bar. If deleteElement is true, the element is not only removed but also deleted.
     *
     * @param element the element to remove
     * @param deleteElement should the element not only be removed but also deleted
     */
    void removeElement(VoiceElement* element, bool deleteElement = true);
    
    void updateAccidentals();
private:
    class Private;
    Private * const d;
};

} // namespace MusicCore

#endif // MUSIC_CORE_VOICEBAR_H
