/* This file is part of the KDE project
 * Copyright 2007 Marijn Kruisselbrink <mkruisselbrink@kde.org>
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
#ifndef SETKEYSIGNATURECOMMAND_H
#define SETKEYSIGNATURECOMMAND_H

#include <kundo2command.h>
#include <QList>
#include <QPair>

class MusicShape;
namespace MusicCore {
    class Bar;
    class Staff;
    class KeySignature;
}

class SetKeySignatureCommand : public KUndo2Command
{
public:
    enum RegionType {
        EndOfPiece,
        NextChange
    };
    SetKeySignatureCommand(MusicShape* shape, int bar, RegionType type, MusicCore::Staff* staff, int accidentals);
    SetKeySignatureCommand(MusicShape* shape, int startBar, int endBar, MusicCore::Staff* staff, int accidentals);
    virtual void redo();
    virtual void undo();
private:
    MusicShape* m_shape;
    MusicCore::Staff* m_staff;
    QList<QPair<MusicCore::Bar*, MusicCore::KeySignature*> > m_newKeySignatures, m_oldKeySignatures;
};

#endif // SETKEYSIGNATURECOMMAND_H
