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
#ifndef CHANGEPARTDETAILSCOMMAND_H
#define CHANGEPARTDETAILSCOMMAND_H

#include <kundo2command.h>
#include <QList>
#include <QPair>

namespace MusicCore {
    class Part;
    class Staff;
    class VoiceElement;
    class Note;
}
class MusicShape;

class ChangePartDetailsCommand : public KUndo2Command
{
public:
    ChangePartDetailsCommand(MusicShape* shape, MusicCore::Part* part, const QString& name, const QString& abbreviation, int staffCount);
    virtual void redo();
    virtual void undo();
private:
    MusicShape* m_shape;
    MusicCore::Part* m_part;
    QString m_oldName, m_newName;
    QString m_oldAbbr, m_newAbbr;
    int m_oldStaffCount, m_newStaffCount;
    QList<MusicCore::Staff*> m_staves;
    QList<QPair<MusicCore::VoiceElement*, MusicCore::Staff*> > m_elements;
    QList<QPair<MusicCore::Note*, MusicCore::Staff*> > m_notes;
};

#endif // CHANGEPARTDETAILSCOMMAND_H
