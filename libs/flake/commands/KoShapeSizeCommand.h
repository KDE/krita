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

#ifndef KOSHAPESIZECOMMAND_H
#define KOSHAPESIZECOMMAND_H

#include "KoSelection.h"

#include <koffice_export.h>

#include <kcommand.h>
#include <QList>

class KoShape;

/// The undo / redo command for shape sizing.
class FLAKE_EXPORT KoShapeSizeCommand : public KCommand {
public:
    /**
     * The undo / redo command for shape sizing.
     * @param shapes all the shapes that will be rezised at the same time
     * @param previousSizes the old sizes; in a list with a member for each shape
     * @param newSizes the new sizes; in a list with a member for each shape
     */
    KoShapeSizeCommand(const KoSelectionSet &shapes, QList<QSizeF> &previousSizes, QList<QSizeF> &newSizes);
    /**
     * The undo / redo command for shape sizing.
     * @param shapes all the shapes that will be rezised at the same time
     * @param previousSizes the old sizes; in a list with a member for each shape
     * @param newSizes the new sizes; in a list with a member for each shape
     */
    KoShapeSizeCommand(const QList<KoShape*> &shapes, QList<QSizeF> &previousSizes, QList<QSizeF> &newSizes);
    /// execute the command
    void execute ();
    /// revert the actions done in execute
    void unexecute ();
    /// return the name of this command
    QString name () const;
private:
    QList<KoShape*> m_shapes;
    QList<QSizeF> m_previousSizes, m_newSizes;
};

#endif
