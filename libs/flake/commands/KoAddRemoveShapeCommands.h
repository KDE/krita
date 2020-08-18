/*
 *  Copyright (c) 2020 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef KoAddRemoveShapeCommands_H
#define KoAddRemoveShapeCommands_H

#include "kritaflake_export.h"

#include "kis_command_utils.h"

class KoShape;
class KoShapeContainer;


struct KRITAFLAKE_EXPORT KoAddRemoveShapeCommandImpl : public KisCommandUtils::FlipFlopCommand
{
    KoAddRemoveShapeCommandImpl(KoShape *shape, KoShapeContainer *parent, State state, KUndo2Command *parentCommand);
    ~KoAddRemoveShapeCommandImpl();

    void partA() override;
    void partB() override;

private:
    bool m_ownShape = true;
    KoShape *m_shape = 0;
    KoShapeContainer *m_parent = 0;
};

struct KRITAFLAKE_EXPORT KoAddShapeCommand : public KoAddRemoveShapeCommandImpl
{
    KoAddShapeCommand(KoShape *shape, KoShapeContainer *parent, KUndo2Command *parentCommand = 0);
};

struct KRITAFLAKE_EXPORT KoRemoveShapeCommand : public KoAddRemoveShapeCommandImpl
{
    KoRemoveShapeCommand(KoShape *shape, KoShapeContainer *parent, KUndo2Command *parentCommand = 0);
};

#endif // KoAddRemoveShapeCommands_H
