/* This file is part of the KDE project
 * Copyright (C) 2006 Thomas Zander <zander@kde.org>
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

#ifndef KOSHAPECREATECOMMAND_H
#define KOSHAPECREATECOMMAND_H

#include <koffice_export.h>
#include <kcommand.h>

class KoShape;
class KoShapeControllerBase;

/// The undo / redo command for creating shapes
class FLAKE_EXPORT KoShapeCreateCommand : public KCommand {
public:
    /**
     * Command used on creation of new shapes
     * @param controller the controller used to add/remove the shape from
     * @param shape the shape thats just been created.
     */
    KoShapeCreateCommand( KoShapeControllerBase *controller, KoShape *shape );
    virtual ~KoShapeCreateCommand();
    /// execute the command
    void execute ();
    /// revert the actions done in execute
    void unexecute ();
    /// return the name of this command
    virtual QString name () const;
private:
    enum AddRemove { Add, Remove };
    void recurse(KoShape *shape, const AddRemove ar);

    KoShapeControllerBase *m_controller;
    KoShape *m_shape;
    bool m_deleteShape;
};

#endif
