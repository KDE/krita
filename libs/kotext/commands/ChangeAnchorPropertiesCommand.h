/*
 *  Copyright (c) 2012 C. Boemann <cbo@boemann.dk>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef CHANGEANCHORPROPERTIESCOMMAND_H
#define CHANGEANCHORPROPERTIESCOMMAND_H

#include <kundo2command.h>
#include "kritatext_export.h"
#include "KoShapeAnchor.h"

#include <QPointF>

class KoShapeContainer;


class KRITATEXT_EXPORT ChangeAnchorPropertiesCommand : public KUndo2Command
{
public:
    ChangeAnchorPropertiesCommand(KoShapeAnchor *anchor, const KoShapeAnchor &newAnchorData, KoShapeContainer *newParent, KUndo2Command *parent);
    virtual ~ChangeAnchorPropertiesCommand();

    /// redo the command
    void redo();
    /// revert the actions done in redo
    void undo();
private:
    void copyLayoutProperties(const KoShapeAnchor *from, KoShapeAnchor *to);

    KoShapeAnchor *m_anchor;
    KoShapeAnchor m_oldAnchor;
    KoShapeAnchor m_newAnchor;
    KoShapeContainer *m_oldParent;
    KoShapeContainer *m_newParent;
    QPointF m_oldAbsPos;
    QPointF m_newAbsPos;
    KoShapeAnchor::TextLocation *m_oldLocation;
    KoShapeAnchor::TextLocation *m_newLocation;
    bool m_first;
    bool m_undone;
};

#endif // CHANGEANCHORPROPERTIESCOMMAND_H
