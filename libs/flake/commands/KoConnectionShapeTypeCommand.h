/* This file is part of the KDE project
 * Copyright (C) 2007 Jan Hambrecht <jaham@gmx.net>
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

#ifndef KoConnectionShapeTypeCommand_H
#define KoConnectionShapeTypeCommand_H

#include "KoConnectionShape.h"
#include <QtGui/QUndoCommand>

/// The undo / redo command for configuring an KoConnection shape
class KoConnectionShapeTypeCommand : public QUndoCommand
{
public:
    /**
     * Changes the tyoe of a connection shape
     * @param connection the connection shape to change type of
     * @param type the connection type
     * @param parent the optional parent command
     */
    KoConnectionShapeTypeCommand(KoConnectionShape *connection, KoConnectionShape::Type type, QUndoCommand *parent = 0);
    /// redo the command
    virtual void redo();
    /// revert the actions done in redo
    virtual void undo();
private:
    KoConnectionShape *m_connection;
    KoConnectionShape::Type m_oldType;
    KoConnectionShape::Type m_newType;
};

#endif // KoConnectionShapeTypeCommand_H

