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
#include "MusicCursor.h"

#include "core/Part.h"
#include "core/Sheet.h"
#include "core/Staff.h"
#include "core/Voice.h"
#include "core/VoiceBar.h"

using namespace MusicCore;

MusicCursor::MusicCursor(Sheet* sheet, QObject* parent)
    : QObject(parent)
    , m_sheet(sheet)
    , m_staff(sheet->part(0)->staff(0))
    , m_voice(0)
    , m_bar(0)
    , m_element(0)
    , m_line(0)
{
}

void MusicCursor::moveRight()
{
    m_element++;
    if (m_element > m_staff->part()->voice(m_voice)->bar(m_sheet->bar(m_bar))->elementCount()) {
        if (m_bar < m_sheet->barCount()-1) {
            m_bar++;
            m_element = 0;
        }
    }
}

void MusicCursor::moveLeft()
{
    m_element--;
    if (m_element < 0) {
        if (m_bar == 0) {
            m_element = 0;
        } else {
            m_bar--;
            m_element = m_staff->part()->voice(m_voice)->bar(m_sheet->bar(m_bar))->elementCount();
        }
    }
}

void MusicCursor::moveUp()
{
    m_line++;
}

void MusicCursor::moveDown()
{
    m_line--;
}

void MusicCursor::setVoice(int voice)
{
    m_voice = voice;
}

VoiceBar* MusicCursor::voiceBar() const
{
    return m_staff->part()->voice(m_voice)->bar(m_sheet->bar(m_bar));
}
