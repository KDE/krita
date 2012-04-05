/*
 *  Copyright (c) 2012 C. Boemann <cbo@boemann.dk>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
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
#include "kotext_export.h"
#include "KoTextAnchor.h"

#include <QPointF>

class KoShapeContainer;


class KOTEXT_EXPORT ChangeAnchorPropertiesCommand : public KUndo2Command
{
public:
    ChangeAnchorPropertiesCommand(KoTextAnchor *anchor, const KoTextAnchor &newAnchorData, KoShapeContainer *newParent, KUndo2Command *parent);
    virtual ~ChangeAnchorPropertiesCommand();

    /// redo the command
    void redo();
    /// revert the actions done in redo
    void undo();
private:
    void copyLayoutProperties(const KoTextAnchor *from, KoTextAnchor *to);

    KoTextAnchor *m_anchor;
    KoTextAnchor m_oldAnchor;
    KoTextAnchor m_newAnchor;
    KoShapeContainer *m_oldParent;
    KoShapeContainer *m_newParent;
    QPointF m_oldAbsPos;
    bool m_first;
};

#endif // CHANGEANCHORPROPERTIESCOMMAND_H
