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

#ifndef KOSHAPEUNGROUPCOMMAND_H
#define KOSHAPEUNGROUPCOMMAND_H

#include "kritaflake_export.h"
#include <kundo2command.h>
#include <QScopedPointer>

class KoShape;
class KoShapeGroup;
class KoShapeContainer;

/// The undo / redo command for ungrouping shapes
class KRITAFLAKE_EXPORT KoShapeUngroupCommand : public KUndo2Command
{
public:
    /**
     * Command to ungroup a set of shapes from one parent container.
     * @param container the group to ungroup the shapes from.
     * @param shapes a list of all the shapes that should be ungrouped.
     * @param topLevelShapes a list of top level shapes.
     * @param parent the parent command used for macro commands
     */
    KoShapeUngroupCommand(KoShapeContainer *container, const QList<KoShape *> &shapes,
                          const QList<KoShape *> &topLevelShapes = QList<KoShape*>(), KUndo2Command *parent = 0);
    ~KoShapeUngroupCommand();

    /// redo the command
    void redo() override;
    /// revert the actions done in redo
    void undo() override;

private:
    struct Private;
    const QScopedPointer<Private> m_d;

};

#endif
