/*
 *  Copyright (c) 2011 Dmitry Kazakov <dimula73@gmail.com>
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
