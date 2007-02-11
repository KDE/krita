/* This file is part of the KDE project
 * Copyright (C) 2006 Jan Hambrecht <jaham@gmx.net>
 * Copyright (C) 2006,2007 Thorsten Zachmann <zachmann@kde.org>
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

#ifndef KOPARAMETERTOPATHCOMMAND_H
#define KOPARAMETERTOPATHCOMMAND_H

#include <QUndoCommand>
#include <QList>
#include "KoPathShape.h"

class KoParameterShape;

/// The undo / redo command for changing a KoParameterShape into a KoPathShape
class KoParameterToPathCommand : public QUndoCommand
{
public:
    KoParameterToPathCommand( KoParameterShape *shape, QUndoCommand *parent = 0 );
    KoParameterToPathCommand( const QList<KoParameterShape*> &shapes, QUndoCommand *parent = 0 );
    virtual ~KoParameterToPathCommand();

    /// redo the command
    void redo();
    /// revert the actions done in redo
    void undo();
private:
    QList<KoParameterShape*> m_shapes;
    QList<KoSubpathList> m_oldSubpaths;
    QList<KoSubpathList> m_newSubpaths;
    bool m_newPointsActive;
};

#endif // KOPARAMETERTOPATHCOMMAND_H
