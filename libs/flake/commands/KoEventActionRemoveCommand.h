/* This file is part of the KDE project
 * Copyright (C) 2008 Thorsten Zachmann <zachmann@kde.org>
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

#ifndef KOEVENTACTIONREMOVECOMMAND_H
#define KOEVENTACTIONREMOVECOMMAND_H

#include <QUndoCommand>
#include "flake_export.h"

class KoShape;
class KoEventAction;
class KoEventActionRemoveCommandPrivate;

/// API docs go here
class FLAKE_EXPORT KoEventActionRemoveCommand : public QUndoCommand
{
public:
    KoEventActionRemoveCommand(KoShape *shape, KoEventAction *eventAction, QUndoCommand *parent = 0);
    virtual ~KoEventActionRemoveCommand();

    /// redo the command
    void redo();
    /// revert the actions done in redo
    void undo();

private:
    KoEventActionRemoveCommandPrivate *d;
};

#endif /* KOEVENTACTIONREMOVECOMMAND_H */
