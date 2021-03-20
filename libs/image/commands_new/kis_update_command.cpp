/*
 *  SPDX-FileCopyrightText: 2011 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_update_command.h"

#include "kis_image_interfaces.h"
#include "kis_node.h"


KisUpdateCommand::KisUpdateCommand(KisNodeSP node, QRect dirtyRect,
                                   KisUpdatesFacade *updatesFacade,
                                   bool needsFullRefresh)
    : KUndo2Command(kundo2_noi18n("UPDATE_COMMAND")),
      m_node(node),
      m_dirtyRect(dirtyRect),
      m_updatesFacade(updatesFacade),
      m_needsFullRefresh(needsFullRefresh)
{
}

KisUpdateCommand::~KisUpdateCommand()
{
}

void KisUpdateCommand::undo()
{
    KUndo2Command::undo();
    update();
}

void KisUpdateCommand::redo()
{
    KUndo2Command::redo();
    update();
}

void KisUpdateCommand::update()
{
    if(m_needsFullRefresh) {
        m_updatesFacade->refreshGraphAsync(m_node, m_dirtyRect);
    }
    else {
        m_node->setDirty(m_dirtyRect);
    }
}
