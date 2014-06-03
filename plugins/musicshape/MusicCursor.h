/* This file is part of the KDE project
 * Copyright 2009 Marijn Kruisselbrink <mkruisselbrink@kde.org>
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
#ifndef MUSICCURSOR_H
#define MUSICCURSOR_H

#include <QObject>

namespace MusicCore {
    class Sheet;
    class Staff;
    class VoiceBar;
}

/**
 * Class that provides information for the current (keyboard) cursor position in a music shape.
 */
class MusicCursor : public QObject
{
    Q_OBJECT
public:
    explicit MusicCursor(MusicCore::Sheet *sheet, QObject *parent = 0);

    void moveRight();
    void moveLeft();
    void moveUp();
    void moveDown();
    void setVoice(int voice);

    MusicCore::Staff* staff() const { return m_staff; }
    int voice() const { return m_voice; }
    int bar() const { return m_bar; }
    int element() const { return m_element; }
    int line() const { return m_line; }
    MusicCore::VoiceBar* voiceBar() const;
private:
    MusicCore::Sheet *m_sheet;
    MusicCore::Staff *m_staff;
    int m_voice;
    int m_bar;
    int m_element;
    int m_line;
};

#endif // MUSICCURSOR_H
