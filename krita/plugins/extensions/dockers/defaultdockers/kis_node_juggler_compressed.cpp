/*
 *  Copyright (c) 2015 Dmitry Kazakov <dimula73@gmail.com>
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

#include "kis_node_juggler_compressed.h"

#include <QHash>
#include <QSharedPointer>
#include <QPointer>

#include "kis_global.h"
#include "kis_image.h"
#include "kis_processing_applicator.h"
#include "commands/kis_image_layer_move_command.h"
#include "kis_signal_compressor.h"
#include "kis_command_utils.h"


struct MoveNodeStruct {
    MoveNodeStruct(KisImageSP _image, KisNodeSP _node, KisNodeSP _parent, KisNodeSP _above)
        : image(_image),
          node(_node),
          newParent(_parent),
          newAbove(_above),
          oldParent(_node->parent()),
          oldAbove(_node->prevSibling())
    {
    }

    bool tryMerge(const MoveNodeStruct &rhs) {
        if (rhs.node != node) return false;

        bool result = true;

        // qDebug() << "Merging";
        // qDebug() << ppVar(node);
        // qDebug() << ppVar(oldParent) << ppVar(newParent);
        // qDebug() << ppVar(oldAbove) << ppVar(newAbove);
        // qDebug() << ppVar(rhs.oldParent) << ppVar(rhs.newParent);
        // qDebug() << ppVar(rhs.oldAbove) << ppVar(rhs.newAbove);

        if (newParent == rhs.oldParent) {
            // 'rhs' is newer
            newParent = rhs.newParent;
            newAbove = rhs.newAbove;
        } else if (oldParent == rhs.newParent) {
            // 'this' is newer
            oldParent = rhs.oldParent;
            oldAbove = rhs.oldAbove;
        } else {
            warnKrita << "MoveNodeStruct: Trying to merge unsequential moves!";
            result = false;
        }

        return result;
    }

    void doRedoUpdates() {
        image->refreshGraphAsync(oldParent);
        if (oldParent != newParent)
            node->setDirty(image->bounds());
    }

    void doUndoUpdates() {
        image->refreshGraphAsync(newParent);
        if (oldParent != newParent)
            node->setDirty(image->bounds());
    }

    KisImageSP image;
    KisNodeSP node;
    KisNodeSP newParent;
    KisNodeSP newAbove;

    KisNodeSP oldParent;
    KisNodeSP oldAbove;
};

typedef QSharedPointer<MoveNodeStruct> MoveNodeStructSP;
typedef QHash<KisNodeSP, MoveNodeStructSP> MovedNodesHash;

struct BatchMoveUpdateData {
    MovedNodesHash m_movedNodesInitial;
    MovedNodesHash m_movedNodesUpdated;

    bool m_jugglerAlive;
    QMutex m_mutex;

    static void addToHashLazy(MovedNodesHash *hash, MoveNodeStructSP moveStruct) {
        if (hash->contains(moveStruct->node)) {
            bool result = hash->value(moveStruct->node)->tryMerge(*moveStruct);
            KIS_ASSERT_RECOVER_NOOP(result);
        } else {
            hash->insert(moveStruct->node, moveStruct);
        }
    }

    void processUnhandledUpdates() {
        QMutexLocker l(&m_mutex);

        if (m_movedNodesInitial.isEmpty()) return;

        MovedNodesHash::const_iterator it = m_movedNodesInitial.constBegin();
        MovedNodesHash::const_iterator end = m_movedNodesInitial.constEnd();

        for (; it != end; ++it) {
            it.value()->doRedoUpdates();
            addToHashLazy(&m_movedNodesUpdated, it.value());
        }

        m_movedNodesInitial.clear();
    }

    void addInitialUpdate(MoveNodeStructSP moveStruct) {
        QMutexLocker l(&m_mutex);
        addToHashLazy(&m_movedNodesInitial, moveStruct);
    }

    void emitFinalUpdates(bool undo) {
        if (m_movedNodesUpdated.isEmpty()) return;

        MovedNodesHash::const_iterator it = m_movedNodesUpdated.constBegin();
        MovedNodesHash::const_iterator end = m_movedNodesUpdated.constEnd();

        for (; it != end; ++it) {
            if (!undo) {
                it.value()->doRedoUpdates();
            } else {
                it.value()->doUndoUpdates();
            }
        }
    }
};

typedef QSharedPointer<BatchMoveUpdateData> BatchMoveUpdateDataSP;

class UpdateMovedNodesCommand : public KisCommandUtils::FlipFlopCommand
{
public:
    UpdateMovedNodesCommand(BatchMoveUpdateDataSP updateData, bool finalize, KUndo2Command *parent = 0)
        : FlipFlopCommand(finalize, parent),
          m_updateData(updateData)
    {
    }

    void end() {
        if (isFirstRedo()) {
            m_updateData->processUnhandledUpdates();
        } else {
            m_updateData->emitFinalUpdates(isFinalizing());
        }
    }
private:
    BatchMoveUpdateDataSP m_updateData;
    bool m_firstRun;
};

struct LowerRaiseLayer : public KisCommandUtils::AggregateCommand {
    LowerRaiseLayer(BatchMoveUpdateDataSP updateData,
               KisImageSP image,
               KisNodeSP node,
               bool lower)
        : m_updateData(updateData),
          m_image(image),
          m_node(node),
          m_lower (lower) {}

    void populateChildCommands() {
        KisNodeSP parent = m_node->parent();
        KisNodeSP grandParent = parent ? parent->parent() : 0;

        KisNodeSP newAbove;
        KisNodeSP newParent;

        if (m_lower) {
            KisNodeSP prevNode = m_node->prevSibling();

            if (prevNode) {
                if (prevNode->inherits("KisGroupLayer") &&
                    !prevNode->collapsed()) {

                    newAbove = prevNode->lastChild();
                    newParent = prevNode;
                } else {
                    newAbove = prevNode->prevSibling();
                    newParent = parent;
                }
            } else if (grandParent &&
                       !m_node->inherits("KisMask")) {

                newAbove = parent->prevSibling();
                newParent = grandParent;
            }
        } else {
            KisNodeSP nextNode = m_node->nextSibling();

            if (nextNode) {
                if (nextNode->inherits("KisGroupLayer") &&
                    !nextNode->collapsed()) {

                    newAbove = 0;
                    newParent = nextNode;
                } else {
                    newAbove = nextNode;
                    newParent = parent;
                }
            } else if (grandParent &&
                       !m_node->inherits("KisMask")) {

                newAbove = parent;
                newParent = grandParent;
            }
        }

        if (!newParent) return;

        MoveNodeStructSP moveStruct = toQShared(new MoveNodeStruct(m_image, m_node, newParent, newAbove));
        addCommand(new KisImageLayerMoveCommand(m_image, m_node, newParent, newAbove, false));
        m_updateData->addInitialUpdate(moveStruct);
    }

private:
    BatchMoveUpdateDataSP m_updateData;

    KisImageSP m_image;
    KisNodeSP m_node;
    bool m_lower;
};

struct KisNodeJugglerCompressed::Private
{
    Private(KisImageSP _image, int _timeout)
        : image(_image),
          compressor(_timeout, KisSignalCompressor::POSTPONE),
          selfDestructionCompressor(3 * _timeout, KisSignalCompressor::POSTPONE),
          updateData(new BatchMoveUpdateData),
          autoDelete(false),
          isStarted(false)
    {}

    KisImageSP image;
    QScopedPointer<KisProcessingApplicator> applicator;

    KisSignalCompressor compressor;
    KisSignalCompressor selfDestructionCompressor;

    BatchMoveUpdateDataSP updateData;

    bool autoDelete;
    bool isStarted;
};

KisNodeJugglerCompressed::KisNodeJugglerCompressed(const KUndo2MagicString &actionName, KisImageSP image, int timeout)
    : m_d(new Private(image, timeout))
{
    connect(m_d->image, SIGNAL(sigStrokeCancellationRequested()), SLOT(slotEndStrokeRequested()));
    connect(m_d->image, SIGNAL(sigStrokeEndRequestedActiveNodeFiltered()), SLOT(slotEndStrokeRequested()));

    KisImageSignalVector emitSignals;
    emitSignals << ModifiedSignal;

    m_d->applicator.reset(
        new KisProcessingApplicator(m_d->image, 0,
                                    KisProcessingApplicator::NONE,
                                    emitSignals,
                                    actionName));
    connect(&m_d->compressor, SIGNAL(timeout()), SLOT(slotUpdateTimeout()));

    m_d->applicator->applyCommand(
            new UpdateMovedNodesCommand(m_d->updateData, false));
    m_d->isStarted = true;
}

KisNodeJugglerCompressed::~KisNodeJugglerCompressed()
{
}

void KisNodeJugglerCompressed::lowerNode(KisNodeSP node)
{
    m_d->applicator->applyCommand(
        new LowerRaiseLayer(m_d->updateData, m_d->image, node, true));

    startTimers();
}

void KisNodeJugglerCompressed::raiseNode(KisNodeSP node)
{
    m_d->applicator->applyCommand(
        new LowerRaiseLayer(m_d->updateData, m_d->image, node, false));
    startTimers();
}

void KisNodeJugglerCompressed::moveNode(KisNodeSP node, KisNodeSP parent, KisNodeSP above)
{
    m_d->applicator->applyCommand(new KisImageLayerMoveCommand(m_d->image, node, parent, above, false));

    MoveNodeStructSP moveStruct = toQShared(new MoveNodeStruct(m_d->image, node, parent, above));

    m_d->updateData->addInitialUpdate(moveStruct);
    startTimers();
}

void KisNodeJugglerCompressed::startTimers()
{
    m_d->compressor.start();

    if (m_d->autoDelete) {
        m_d->selfDestructionCompressor.start();
    }
}

void KisNodeJugglerCompressed::slotUpdateTimeout()
{
    m_d->updateData->processUnhandledUpdates();
}

void KisNodeJugglerCompressed::end()
{
    if (!m_d->isStarted) return;

    m_d->applicator->applyCommand(
            new UpdateMovedNodesCommand(m_d->updateData, true));

    m_d->applicator->end();
    m_d->applicator.reset();
    m_d->compressor.stop();
    m_d->isStarted = false;

    if (m_d->autoDelete) {
        m_d->selfDestructionCompressor.stop();
        this->deleteLater();
    }
}

void KisNodeJugglerCompressed::setAutoDelete(bool value)
{
    m_d->autoDelete = value;
    connect(&m_d->selfDestructionCompressor, SIGNAL(timeout()), SLOT(end()));
}

void KisNodeJugglerCompressed::slotEndStrokeRequested()
{
    if (!m_d->isStarted) return;
    end();
}

bool KisNodeJugglerCompressed::isEnded() const
{
    return !m_d->isStarted;
}
