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

#ifndef KOSHAPEBACKGROUNDCOMMAND_H
#define KOSHAPEBACKGROUNDCOMMAND_H

#include "KoSelection.h"

#include <koffice_export.h>
#include <kcommand.h>


/// The undo / redo command for setting the shape background
class FLAKE_EXPORT KoShapeBackgroundCommand : public KCommand {
public:
    /**
     * Command to set a new shape background.
     * @param shapes a set of all the shapes that should get the new background.
     * @param brush the new background brush
     */
    KoShapeBackgroundCommand( const KoSelectionSet &shapes, const QBrush &brush );
    virtual ~KoShapeBackgroundCommand();
    /// execute the command
    void execute ();
    /// revert the actions done in execute
    void unexecute ();
    /// return the name of this command
    virtual QString name () const;
private:
    QList<KoShape*> m_shapes;    ///< the shapes to set background for
    QList<QBrush> m_oldBrushes; ///< the old background brushes, one for each shape
    QBrush m_newBrush;           ///< the new background brush to set
};

#endif
