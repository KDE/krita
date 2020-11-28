/*
 *  SPDX-FileCopyrightText: 2020 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
