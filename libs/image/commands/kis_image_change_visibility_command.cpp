/*
 *  SPDX-FileCopyrightText: 2017 Nikita Smirnov <pakrentos@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */


#include "kis_image_commands.h"
#include "kis_image.h"
#include "kis_node.h"

#include <klocalizedstring.h>


KisImageChangeVisibilityCommand::KisImageChangeVisibilityCommand(bool visibility, KisNodeSP node)
    : KUndo2Command(kundo2_noi18n("change-visibility-command"), 0)
{
    m_node = node;
    m_visible = visibility;
}

void KisImageChangeVisibilityCommand::redo()
{
    m_node->setVisible(m_visible);
}

void KisImageChangeVisibilityCommand::undo()
{
    m_node->setVisible(!m_visible);
}
