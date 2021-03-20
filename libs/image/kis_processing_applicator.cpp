/*
 *  SPDX-FileCopyrightText: 2011 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_processing_applicator.h"

#include "kis_image.h"
#include "kis_paint_layer.h"
#include "kis_node.h"
#include "kis_clone_layer.h"
#include "kis_processing_visitor.h"
#include "commands_new/kis_processing_command.h"
#include "kis_stroke_strategy_undo_command_based.h"
#include "kis_layer_utils.h"
#include "kis_command_utils.h"
#include "kis_time_span.h"
#include "kis_node.h"
#include "kis_image_signal_router.h"
#include "KisAsynchronouslyMergeableCommandInterface.h"
#include "kis_command_ids.h"

class DisableUIUpdatesCommand : public KisCommandUtils::FlipFlopCommand, public KisAsynchronouslyMergeableCommandInterface
{
public:
    DisableUIUpdatesCommand(KisImageWSP image,
                            bool finalUpdate)
        : FlipFlopCommand(finalUpdate),
          m_image(image)
    {
    }

    void partA() override {
        m_image->disableUIUpdates();
    }

    void partB() override {
        m_image->enableUIUpdates();
    }

    int id() const override {
        return KisCommandUtils::DisableUIUpdatesCommandId;
    }

    bool mergeWith(const KUndo2Command *command) override {
        return canMergeWith(command);
    }

    bool canMergeWith(const KUndo2Command *command) const override {
        const DisableUIUpdatesCommand *other =
            dynamic_cast<const DisableUIUpdatesCommand*>(command);

        return other && other->m_image == m_image;
    }

private:
    KisImageWSP m_image;
};


class UpdateCommand : public KisCommandUtils::FlipFlopCommand, public KisAsynchronouslyMergeableCommandInterface
{
public:
    UpdateCommand(KisImageWSP image, KisNodeSP node,
                  KisProcessingApplicator::ProcessingFlags flags,
                  State initialState, QSharedPointer<bool> sharedAllFramesToken)
        : FlipFlopCommand(initialState),
          m_image(image),
          m_node(node),
          m_flags(flags),
          m_sharedAllFramesToken(sharedAllFramesToken)
    {
    }

private:
    void partA() override {
        /**
         * We disable all non-centralized updates here. Everything
         * should be done by this command's explicit updates.
         *
         * If you still need third-party updates work, please add a
         * flag to the applicator.
         */
        m_image->disableDirtyRequests();
    }

    void partB() override {
        m_image->enableDirtyRequests();

        if (*m_sharedAllFramesToken) {
            KisLayerUtils::recursiveApplyNodes(m_image->root(), [](KisNodeSP node){
                KisPaintLayer* paintLayer = qobject_cast<KisPaintLayer*>(node.data());
                if (paintLayer && paintLayer->onionSkinEnabled()) {
                    paintLayer->flushOnionSkinCache();
                }
            });
        }

        m_image->root()->graphListener()->invalidateFrames(KisTimeSpan::infinite(0), m_node->exactBounds());

        if (!m_flags.testFlag(KisProcessingApplicator::NO_IMAGE_UPDATES)) {
            if(m_flags.testFlag(KisProcessingApplicator::RECURSIVE)) {
                m_image->refreshGraphAsync(m_node);
            }

            m_node->setDirty(m_image->bounds());

            updateClones(m_node);
        }
    }

    void updateClones(KisNodeSP node) {
        // simple tail-recursive iteration

        KisNodeSP prevNode = node->lastChild();
        while(prevNode) {
            updateClones(prevNode);
            prevNode = prevNode->prevSibling();
        }

        KisLayer *layer = qobject_cast<KisLayer*>(m_node.data());
        if(layer && layer->hasClones()) {
            Q_FOREACH (KisCloneLayerSP clone, layer->registeredClones()) {
                if(!clone) continue;

                QPoint offset(clone->x(), clone->y());
                QRegion dirtyRegion(m_image->bounds());
                dirtyRegion -= m_image->bounds().translated(offset);

                clone->setDirty(KisRegion::fromQRegion(dirtyRegion));
            }
        }
    }

    int id() const override {
        return KisCommandUtils::UpdateCommandId;
    }

    bool mergeWith(const KUndo2Command *command) override {
        return canMergeWith(command);
    }

    bool canMergeWith(const KUndo2Command *command) const override {
        const UpdateCommand *other =
            dynamic_cast<const UpdateCommand*>(command);

        return other &&
            other->m_image == m_image &&
            other->m_node == m_node &&
            other->m_flags == m_flags &&
            bool(other->m_sharedAllFramesToken) == bool(m_sharedAllFramesToken) &&
            (!m_sharedAllFramesToken || *m_sharedAllFramesToken == *other->m_sharedAllFramesToken);
    }

private:
    KisImageWSP m_image;
    KisNodeSP m_node;
    KisProcessingApplicator::ProcessingFlags m_flags;
    QSharedPointer<bool> m_sharedAllFramesToken;
};

class EmitImageSignalsCommand : public KisCommandUtils::FlipFlopCommand, public KisAsynchronouslyMergeableCommandInterface
{
public:
    EmitImageSignalsCommand(KisImageWSP image,
                            KisImageSignalVector emitSignals,
                            bool finalUpdate)
        : FlipFlopCommand(finalUpdate),
          m_image(image),
          m_emitSignals(emitSignals)
    {
    }

    void partB() override {
        if (getState() == State::FINALIZING) {
            doUpdate(m_emitSignals);
        } else {
            KisImageSignalVector reverseSignals;

            KisImageSignalVector::iterator i = m_emitSignals.end();
            while (i != m_emitSignals.begin()) {
                --i;
                reverseSignals.append(i->inverted());
            }

            doUpdate(reverseSignals);
        }
    }

    int id() const override {
        return KisCommandUtils::EmitImageSignalsCommandId;
    }

    bool mergeWith(const KUndo2Command *command) override {
        return canMergeWith(command);
    }

    bool canMergeWith(const KUndo2Command *command) const override {
        const EmitImageSignalsCommand *other =
            dynamic_cast<const EmitImageSignalsCommand*>(command);

        return other &&
            other->m_image == m_image;

            // TODO: implement proper comparison for emitted signals
            // other->m_emitSignals == m_emitSignals;
    }

private:
    void doUpdate(KisImageSignalVector emitSignals) {
        Q_FOREACH (KisImageSignalType type, emitSignals) {
            m_image->signalRouter()->emitNotification(type);
        }
    }

private:
    KisImageWSP m_image;
    KisImageSignalVector m_emitSignals;
};


KisProcessingApplicator::KisProcessingApplicator(KisImageWSP image,
                                                 KisNodeSP node,
                                                 ProcessingFlags flags,
                                                 KisImageSignalVector emitSignals,
                                                 const KUndo2MagicString &name,
                                                 KUndo2CommandExtraData *extraData,
                                                 int macroId)
    : m_image(image),
      m_node(node),
      m_flags(flags),
      m_emitSignals(emitSignals),
      m_finalSignalsEmitted(false),
      m_sharedAllFramesToken(new bool(false))
{
    KisStrokeStrategyUndoCommandBased *strategy =
            new KisStrokeStrategyUndoCommandBased(name, false, m_image.data());

    if (m_flags.testFlag(SUPPORTS_WRAPAROUND_MODE)) {
        strategy->setSupportsWrapAroundMode(true);
    }

    if (extraData) {
        strategy->setCommandExtraData(extraData);
    }

    strategy->setMacroId(macroId);

    m_strokeId = m_image->startStroke(strategy);
    if(!m_emitSignals.isEmpty()) {
        applyCommand(new EmitImageSignalsCommand(m_image, m_emitSignals, false), KisStrokeJobData::BARRIER);
    }

    if(m_flags.testFlag(NO_UI_UPDATES)) {
        applyCommand(new DisableUIUpdatesCommand(m_image, false), KisStrokeJobData::BARRIER);
    }

    if (m_node) {
        applyCommand(new UpdateCommand(m_image, m_node, m_flags,
                                       UpdateCommand::INITIALIZING,
                                       m_sharedAllFramesToken));
    }
}

KisProcessingApplicator::~KisProcessingApplicator()
{
}

const KisStrokeId KisProcessingApplicator::getStroke() const
{
    return m_strokeId;
}

void KisProcessingApplicator::applyVisitor(KisProcessingVisitorSP visitor,
                                           KisStrokeJobData::Sequentiality sequentiality,
                                           KisStrokeJobData::Exclusivity exclusivity)
{
    KUndo2Command *initCommand = visitor->createInitCommand();
    if (initCommand) {
        applyCommand(initCommand,
                     KisStrokeJobData::SEQUENTIAL, KisStrokeJobData::NORMAL);
    }

    if(!m_flags.testFlag(RECURSIVE)) {
        applyCommand(new KisProcessingCommand(visitor, m_node),
                     sequentiality, exclusivity);
    }
    else {
        visitRecursively(m_node, visitor, sequentiality, exclusivity);
    }
}

void KisProcessingApplicator::applyVisitorAllFrames(KisProcessingVisitorSP visitor,
                                                    KisStrokeJobData::Sequentiality sequentiality,
                                                    KisStrokeJobData::Exclusivity exclusivity)
{
    *m_sharedAllFramesToken = true;

    KUndo2Command *initCommand = visitor->createInitCommand();
    if (initCommand) {
        applyCommand(initCommand,
                     KisStrokeJobData::SEQUENTIAL, KisStrokeJobData::NORMAL);
    }

    KisLayerUtils::FrameJobs jobs;

    // TODO: implement a nonrecursive case when !m_flags.testFlag(RECURSIVE)
    //       (such case is not yet used anywhere)
    KIS_SAFE_ASSERT_RECOVER_NOOP(m_flags.testFlag(RECURSIVE));

    KisLayerUtils::updateFrameJobsRecursive(&jobs, m_node);

    if (jobs.isEmpty()) {
        applyVisitor(visitor, sequentiality, exclusivity);
        return;
    }

    KisLayerUtils::FrameJobs::const_iterator it = jobs.constBegin();
    KisLayerUtils::FrameJobs::const_iterator end = jobs.constEnd();

    KisLayerUtils::SwitchFrameCommand::SharedStorageSP switchFrameStorage(
        new KisLayerUtils::SwitchFrameCommand::SharedStorage());

    for (; it != end; ++it) {
        const int frame = it.key();
        const QSet<KisNodeSP> &nodes = it.value();

        applyCommand(new KisLayerUtils::SwitchFrameCommand(m_image, frame, false, switchFrameStorage), KisStrokeJobData::BARRIER, KisStrokeJobData::EXCLUSIVE);

        foreach (KisNodeSP node, nodes) {
            applyCommand(new KisProcessingCommand(visitor, node),
                         sequentiality, exclusivity);
        }

        applyCommand(new KisLayerUtils::SwitchFrameCommand(m_image, frame, true, switchFrameStorage), KisStrokeJobData::BARRIER, KisStrokeJobData::EXCLUSIVE);
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
    /*
     * One should not add commands after the final signals have been
     * emitted, only end or cancel the stroke
     */
    KIS_ASSERT_RECOVER_RETURN(!m_finalSignalsEmitted);

    m_image->addJob(m_strokeId,
                    new KisStrokeStrategyUndoCommandBased::Data(KUndo2CommandSP(command),
                                                                false,
                                                                sequentiality,
                                                                exclusivity));
}

void KisProcessingApplicator::explicitlyEmitFinalSignals()
{
    KIS_ASSERT_RECOVER_RETURN(!m_finalSignalsEmitted);

    if (m_node) {
        applyCommand(new UpdateCommand(m_image, m_node, m_flags,
                                       UpdateCommand::FINALIZING,
                                       m_sharedAllFramesToken));
    }

    if(m_flags.testFlag(NO_UI_UPDATES)) {
        applyCommand(new DisableUIUpdatesCommand(m_image, true), KisStrokeJobData::BARRIER);
    }

    if(!m_emitSignals.isEmpty()) {
        applyCommand(new EmitImageSignalsCommand(m_image, m_emitSignals, true), KisStrokeJobData::BARRIER);
    }

    // simple consistency check
    m_finalSignalsEmitted = true;
}

void KisProcessingApplicator::end()
{
    if (!m_finalSignalsEmitted) {
        explicitlyEmitFinalSignals();
    }

    m_image->endStroke(m_strokeId);
}

void KisProcessingApplicator::cancel()
{
    m_image->cancelStroke(m_strokeId);
}

void KisProcessingApplicator::runSingleCommandStroke(KisImageSP image, KUndo2Command *cmd, KisStrokeJobData::Sequentiality sequentiality, KisStrokeJobData::Exclusivity exclusivity)
{
    KisProcessingApplicator applicator(image, 0,
                                       KisProcessingApplicator::NONE,
                                       KisImageSignalVector(),
                                       cmd->text());
    applicator.applyCommand(cmd, sequentiality, exclusivity);
    applicator.end();
}
