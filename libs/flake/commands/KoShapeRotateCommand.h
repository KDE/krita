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

#ifndef KOSHAPEROTATECOMMAND_H
#define KOSHAPEROTATECOMMAND_H

#include "KoSelection.h"
#include <koffice_export.h>

#include <kcommand.h>
#include <QList>

class KoShape;

/// The undo / redo command for shape rotating.
class FLAKE_EXPORT KoShapeRotateCommand : public KCommand {
public:
    /**
     * Comand to rotate a selection of shapes.  Note that it just alters the rotated
     * property of those shapes, and nothing more.
     * @param shapes all the shapes that should be rotated
     * @param previousAngles a list with the same amount of items as shapes with the
     *        old rotation angles
     * @param newAngles a list with the same amount of items as shapes with the new angles.
     */
    KoShapeRotateCommand(const KoSelectionSet &shapes, QList<double> &previousAngles, QList<double> &newAngles);
    /**
     * Comand to rotate a selection of shapes.  Note that it just alters the rotated
     * property of those shapes, and nothing more.
     * @param shapes all the shapes that should be rotated
     * @param previousAngles a list with the same amount of items as shapes with the
     *        old rotation angles
     * @param newAngles a list with the same amount of items as shapes with the new angles.
     */
    KoShapeRotateCommand(const QList<KoShape*> &shapes, QList<double> &previousAngles, QList<double> &newAngles);
    /// execute the command
    void execute ();
    /// revert the actions done in execute
    void unexecute ();
    /// return the name of this command
    QString name () const;
private:
    QList<KoShape*> m_shapes;
    QList<double> m_previousAngles, m_newAngles;
};

#endif
