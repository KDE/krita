/*
 *  Copyright (c) 2009 Cyrille Berger <cberger@cberger.net>
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

#include "./kis_node_commands_adapter.h"

#include <KoCompositeOp.h>
#include "kis_undo_adapter.h"
#include "kis_image.h"
#include "commands/kis_image_layer_add_command.h"
#include "commands/kis_image_layer_move_command.h"
#include "commands/kis_image_layer_remove_command.h"
#include "commands/KisNodeRenameCommand.h"
#include "commands/kis_node_commands.h"
#include "KisViewManager.h"
#include "kis_processing_applicator.h"

KisNodeCommandsAdapter::KisNodeCommandsAdapter(KisViewManager * view)
        : QObject(view)
        , m_view(view)
{

}

KisNodeCommandsAdapter::~KisNodeCommandsAdapter()
{
}

void KisNodeCommandsAdapter::applyOneCommandAsync(KUndo2Command *cmd, KisProcessingApplicator *applicator)
{
    if (applicator) {
        applicator->applyCommand(cmd, KisStrokeJobData::SEQUENTIAL, KisStrokeJobData::EXCLUSIVE);
    } else {
        QScopedPointer<KisProcessingApplicator> localApplicator(
            new KisProcessingApplicator(m_view->image(), 0, KisProcessingApplicator::NONE,
                                        KisImageSignalVector() << ModifiedSignal,
                                        cmd->text()));
        localApplicator->applyCommand(cmd);
        localApplicator->end();
    }
}

void KisNodeCommandsAdapter::addNodeAsync(KisNodeSP node, KisNodeSP parent, KisNodeSP aboveThis, bool doRedoUpdates, bool doUndoUpdates, KisProcessingApplicator *applicator)
{
    KUndo2Command *cmd = new KisImageLayerAddCommand(m_view->image(), node, parent, aboveThis, doRedoUpdates, doUndoUpdates);
    applyOneCommandAsync(cmd, applicator);
}

void KisNodeCommandsAdapter::addNodeAsync(KisNodeSP node, KisNodeSP parent, quint32 index, bool doRedoUpdates, bool doUndoUpdates, KisProcessingApplicator *applicator)
{
    KUndo2Command *cmd = new KisImageLayerAddCommand(m_view->image(), node, parent, index, doRedoUpdates, doUndoUpdates);
    applyOneCommandAsync(cmd, applicator);
}

void KisNodeCommandsAdapter::beginMacro(const KUndo2MagicString& macroName)
{
    Q_ASSERT(m_view->image()->undoAdapter());
    m_view->image()->undoAdapter()->beginMacro(macroName);
}

void KisNodeCommandsAdapter::addExtraCommand(KUndo2Command *command)
{
    Q_ASSERT(m_view->image()->undoAdapter());
    m_view->image()->undoAdapter()->addCommand(command);
}

void KisNodeCommandsAdapter::endMacro()
{
    Q_ASSERT(m_view->image()->undoAdapter());
    m_view->image()->undoAdapter()->endMacro();
}

void KisNodeCommandsAdapter::addNode(KisNodeSP node, KisNodeSP parent, KisNodeSP aboveThis, bool doRedoUpdates, bool doUndoUpdates)
{
    Q_ASSERT(m_view->image()->undoAdapter());
    m_view->image()->undoAdapter()->addCommand(new KisImageLayerAddCommand(m_view->image(), node, parent, aboveThis, doRedoUpdates, doUndoUpdates));
}

void KisNodeCommandsAdapter::addNode(KisNodeSP node, KisNodeSP parent, quint32 index, bool doRedoUpdates, bool doUndoUpdates)
{
    Q_ASSERT(m_view->image()->undoAdapter());
    m_view->image()->undoAdapter()->addCommand(new KisImageLayerAddCommand(m_view->image(), node, parent, index, doRedoUpdates, doUndoUpdates));
}

void KisNodeCommandsAdapter::moveNode(KisNodeSP node, KisNodeSP parent, KisNodeSP aboveThis)
{
    Q_ASSERT(m_view->image()->undoAdapter());
    m_view->image()->undoAdapter()->addCommand(new KisImageLayerMoveCommand(m_view->image(), node, parent, aboveThis));
}

void KisNodeCommandsAdapter::moveNode(KisNodeSP node, KisNodeSP parent, quint32 indexaboveThis)
{
    Q_ASSERT(m_view->image()->undoAdapter());
    m_view->image()->undoAdapter()->addCommand(new KisImageLayerMoveCommand(m_view->image(), node, parent, indexaboveThis));
}

void KisNodeCommandsAdapter::removeNode(KisNodeSP node)
{
    Q_ASSERT(m_view->image()->undoAdapter());
    m_view->image()->undoAdapter()->addCommand(new KisImageLayerRemoveCommand(m_view->image(), node));
}

void KisNodeCommandsAdapter::setOpacity(KisNodeSP node, qint32 opacity)
{
    KUndo2Command *cmd = new KisNodeOpacityCommand(node, node->opacity(), opacity);
    applyOneCommandAsync(cmd);
}

void KisNodeCommandsAdapter::setCompositeOp(KisNodeSP node,
                                            const KoCompositeOp* compositeOp)
{
    KUndo2Command *cmd = new KisNodeCompositeOpCommand(node, node->compositeOpId(),
                                                       compositeOp->id());
    applyOneCommandAsync(cmd);
}

void KisNodeCommandsAdapter::setNodeName(KisNodeSP node, const QString &name)
{
    KUndo2Command *cmd = new KisNodeRenameCommand(node, node->name(), name);
    applyOneCommandAsync(cmd);
}

void KisNodeCommandsAdapter::undoLastCommand()
{
    Q_ASSERT(m_view->image()->undoAdapter());
    m_view->image()->undoAdapter()->undoLastCommand();
}
