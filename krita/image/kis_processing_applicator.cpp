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

#include "kis_processing_applicator.h"

#include "kis_image.h"
#include "kis_image_signal_router.h"
#include "kis_node.h"
#include "kis_clone_layer.h"
#include "kis_processing_visitor.h"
#include "commands_new/kis_processing_command.h"
#include "kis_stroke_strategy_undo_command_based.h"


class DisableUIUpdatesCommand : public KUndo2Command
{
public:
    DisableUIUpdatesCommand(KisImageWSP image,
                          bool finalUpdate)
        : m_image(image),
          m_finalUpdate(finalUpdate)
    {
    }

    void redo() {
        if(m_finalUpdate) m_image->enableUIUpdates();
        else m_image->disableUIUpdates();
    }

    void undo() {
        if(!m_finalUpdate) m_image->enableUIUpdates();
        else m_image->disableUIUpdates();
    }

private:
    KisImageWSP m_image;
    bool m_finalUpdate;
};


class UpdateCommand : public KUndo2Command
{
public:
    UpdateCommand(KisImageWSP image, KisNodeSP node,
                  KisProcessingApplicator::ProcessingFlags flags,
                  bool finalUpdate)
        : m_image(image),
          m_node(node),
          m_flags(flags),
          m_finalUpdate(finalUpdate)
    {
    }

    void redo() {
        if(!m_finalUpdate) initialize();
        else finalize();
    }

    void undo() {
        if(m_finalUpdate) initialize();
        else finalize();
    }

private:
    void initialize() {
        /**
         * We disable all non-centralized updates here. Everything
         * should be done by this command's explicit updates.
         *
         * If you still need third-party updates work, please add a
         * flag to the applicator.
         */

        m_image->disableDirtyRequests();
    }

    void finalize() {
        m_image->enableDirtyRequests();

        if(m_flags.testFlag(KisProcessingApplicator::RECURSIVE)) {
            m_image->refreshGraphAsync(m_node);
        }

        m_node->setDirty(m_image->bounds());

        updateClones(m_node);
    }

    void updateClones(KisNodeSP node) {
        // simple tail-recursive iteration

        KisNodeSP prevNode = node->lastChild();
        while(prevNode) {
            updateClones(prevNode);
            prevNode = prevNode->prevSibling();
        }

        KisLayer *layer = dynamic_cast<KisLayer*>(m_node.data());
        if(layer && layer->hasClones()) {
            foreach(KisCloneLayerSP clone, layer->registeredClones()) {
                if(!clone) continue;

                QPoint offset(clone->x(), clone->y());
                QRegion dirtyRegion(m_image->bounds());
                dirtyRegion -= m_image->bounds().translated(offset);

                clone->setDirty(dirtyRegion);
            }
        }
    }

private:
    KisImageWSP m_image;
    KisNodeSP m_node;
    KisProcessingApplicator::ProcessingFlags m_flags;
    bool m_finalUpdate;
};

class EmitImageSignalsCommand : public KUndo2Command
{
public:
    EmitImageSignalsCommand(KisImageWSP image,
                            KisImageSignalVector emitSignals,
                            bool finalUpdate)
        : m_image(image),
          m_emitSignals(emitSignals),
          m_finalUpdate(finalUpdate)
    {
    }

    void redo() {
        if(m_finalUpdate) doUpdate(m_emitSignals);
    }

    void undo() {
        if(!m_finalUpdate) {

            KisImageSignalVector reverseSignals;

            KisImageSignalVector::iterator i = m_emitSignals.end();
            while (i != m_emitSignals.begin()) {
                --i;
                reverseSignals.append(i->inverted());
            }

            doUpdate(reverseSignals);
        }
    }

private:
    void doUpdate(KisImageSignalVector emitSignals) {
        foreach(KisImageSignalType type, emitSignals) {
            m_image->signalRouter()->emitNotification(type);
        }
    }

private:
    KisImageWSP m_image;
    KisImageSignalVector m_emitSignals;
    bool m_finalUpdate;
};


KisProcessingApplicator::KisProcessingApplicator(KisImageWSP image,
                                                 KisNodeSP node,
                                                 ProcessingFlags flags,
                                                 KisImageSignalVector emitSignals,
                                                 const QString &name)
    : m_image(image),
      m_node(node),
      m_flags(flags),
      m_emitSignals(emitSignals)
{
    KisStrokeStrategyUndoCommandBased *strategy =
        new KisStrokeStrategyUndoCommandBased(name, false,
                                              m_image->postExecutionUndoAdapter());

    m_strokeId = m_image->startStroke(strategy);
    if(!m_emitSignals.isEmpty()) {
        applyCommand(new EmitImageSignalsCommand(m_image, m_emitSignals, false), KisStrokeJobData::BARRIER);
    }

    if(m_flags.testFlag(NO_UI_UPDATES)) {
        applyCommand(new DisableUIUpdatesCommand(m_image, false), KisStrokeJobData::BARRIER);
    }

    if (m_node) {
        applyCommand(new UpdateCommand(m_image, m_node, m_flags, false));
    }
}

KisProcessingApplicator::~KisProcessingApplicator()
{
}


void KisProcessingApplicator::applyVisitor(KisProcessingVisitorSP visitor,
                                           KisStrokeJobData::Sequentiality sequentiality,
                                           KisStrokeJobData::Exclusivity exclusivity)
{
    if(!m_flags.testFlag(RECURSIVE)) {
        applyCommand(new KisProcessingCommand(visitor, m_node),
                     sequentiality, exclusivity);
    }
    else {
        visitRecursively(m_node, visitor, sequentiality, exclusivity);
    }
}

void KisProcessingApplicator::visitRecursively(KisNodeSP node,
                                               KisProcessingVisitorSP visitor,
                                               KisStrokeJobData::Sequentiality sequentiality,
                                               KisStrokeJobData::Exclusivity exclusivity)
{
    // simple tail-recursive iteration

    KisNodeSP prevNode = node->lastChild();
    while(prevNode) {
        visitRecursively(prevNode, visitor, sequentiality, exclusivity);
        prevNode = prevNode->prevSibling();
    }

    applyCommand(new KisProcessingCommand(visitor, node),
                 sequentiality, exclusivity);
}

void KisProcessingApplicator::applyCommand(KUndo2Command *command,
                                           KisStrokeJobData::Sequentiality sequentiality,
                                           KisStrokeJobData::Exclusivity exclusivity)
{
    m_image->addJob(m_strokeId,
                    new KisStrokeStrategyUndoCommandBased::
                    Data(KUndo2CommandSP(command),
                         false,
                         sequentiality,
                         exclusivity));
}

void KisProcessingApplicator::end()
{
    if (m_node) {
        applyCommand(new UpdateCommand(m_image, m_node, m_flags, true));
    }

    if(m_flags.testFlag(NO_UI_UPDATES)) {
        applyCommand(new DisableUIUpdatesCommand(m_image, true), KisStrokeJobData::BARRIER);
    }

    if(!m_emitSignals.isEmpty()) {
        applyCommand(new EmitImageSignalsCommand(m_image, m_emitSignals, true), KisStrokeJobData::BARRIER);
    }
    m_image->endStroke(m_strokeId);
}

