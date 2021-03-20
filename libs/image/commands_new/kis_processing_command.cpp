/*
 *  SPDX-FileCopyrightText: 2011 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_processing_command.h"
#include "kis_node.h"
#include "kis_processing_visitor.h"


KisProcessingCommand::KisProcessingCommand(KisProcessingVisitorSP visitor, KisNodeSP node, KUndo2Command *parent)
    : KUndo2Command(kundo2_noi18n("processing_command"), parent),
      m_visitor(visitor),
      m_node(node),
      m_visitorExecuted(false)
{
}

void KisProcessingCommand::redo()
{
    if(!m_visitorExecuted) {
        m_node->accept(*m_visitor, &m_undoAdapter);
        m_visitorExecuted = true;
        m_visitor = 0;
    }
    else {
        m_undoAdapter.redoAll();
    }
}

void KisProcessingCommand::undo()
{
    m_undoAdapter.undoAll();
}
