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

#ifndef KOPATHFILLRULECOMMAND_H
#define KOPATHFILLRULECOMMAND_H

#include <flake_export.h>

#include <QUndoCommand>
#include <QList>

class KoPathShape;

/// The undo / redo command for setting the fill rule of a path shape
class FLAKE_EXPORT KoPathFillRuleCommand : public QUndoCommand {
public:
    /**
     * Command to set a new shape background.
     * @param shapes a set of all the path shapes that should get the new fill rule
     * @param fillrule the new fill rule
     * @param parent the parent command used for macro commands
     */
    KoPathFillRuleCommand( const QList<KoPathShape*> &shapes, Qt::FillRule fillRule, QUndoCommand *parent = 0 );
    virtual ~KoPathFillRuleCommand();
    /// redo the command
    void redo ();
    /// revert the actions done in redo
    void undo ();
private:
    class Private;
    Private * const d;
};

#endif // KOPATHFILLRULECOMMAND_H
