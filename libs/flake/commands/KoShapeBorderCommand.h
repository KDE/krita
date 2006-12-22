/* This file is part of the KDE project
 * Copyright (C) 2006 Thomas Zander <zander@kde.org>
 * Copyright (C) 2006 Jan Hambrecht <jaham@gmx.net>
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

#ifndef KOSHAPEBORDERCOMMAND_H
#define KOSHAPEBORDERCOMMAND_H

#include "KoSelection.h"

#include <koffice_export.h>
#include <QUndoCommand>

/// The undo / redo command for setting the shape border
class FLAKE_EXPORT KoShapeBorderCommand : public QUndoCommand {
public:
    /**
     * Command to set a new shape background.
     * @param shapes a set of all the shapes that should get the new background.
     * @param border the new border
     * @param parent the parent command used for macro commands
     */
    KoShapeBorderCommand( const KoSelectionSet &shapes, KoShapeBorderModel *border, QUndoCommand *parent = 0 );
    virtual ~KoShapeBorderCommand();
    /// redo the command
    void redo ();
    /// revert the actions done in redo
    void undo ();
private:
    QList<KoShape*> m_shapes;                ///< the shapes to set border for
    QList<KoShapeBorderModel*> m_oldBorders; ///< the old borders, one for each shape
    KoShapeBorderModel * m_newBorder;        ///< the new border to set
};

#endif
