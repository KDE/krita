/*
 * SPDX-FileCopyrightText: 2013 Lukáš Tvrdý <lukast.dev@gmail.com
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "kis_qmic_applicator.h"

#include <kis_image_signal_router.h>
#include <kis_processing_applicator.h>

#include <kis_image.h>
#include <kis_selection.h>

#include "kis_import_qmic_processing_visitor.h"
#include "kis_qmic_synchronize_layers_command.h"
#include "kis_qmic_synchronize_image_size_command.h"

KisQmicApplicator::KisQmicApplicator()
{
}

KisQmicApplicator::~KisQmicApplicator()
{
}

void KisQmicApplicator::setProperties(KisImageWSP image, KisNodeSP node, QVector<gmic_image<float> *> images, const KUndo2MagicString &actionName, KisNodeListSP kritaNodes)
{
    dbgPlugins << "KisQmicApplicator::setProperties();" << ppVar(image) << ppVar(node) << images.size() << actionName << kritaNodes->count();

    m_image = image;
    m_node = node;
    m_actionName = actionName;
    m_kritaNodes = kritaNodes;
    m_images = images;
}


void KisQmicApplicator::apply()
{
    dbgPlugins << "Request for applying the result";
    cancel();

    KisImageSignalVector emitSignals;
    emitSignals << ComplexSizeChangedSignal();

    m_applicator.reset(
        new KisProcessingApplicator(m_image, m_node,
                                    KisProcessingApplicator::RECURSIVE |
                                    KisProcessingApplicator::NO_UI_UPDATES,
                                    emitSignals, m_actionName));
    dbgPlugins << "Created applicator " << m_applicator;

    m_gmicData = KisQmicDataSP(new KisQmicData());

    QRect layerSize;
    KisSelectionSP selection = m_image->globalSelection();
    if (selection) {
        layerSize = selection->selectedExactRect();
    }
    else {
        layerSize = QRect(0, 0, m_image->width(), m_image->height());
    }

    // This is a three-stage process.

    if (!selection) {
        // 1. synchronize Krita image size with biggest gmic layer size
        m_applicator->applyCommand(new KisQmicSynchronizeImageSizeCommand(m_images, m_image));
    }

    // 2. synchronize layer count and convert excess GMic nodes to paint layers
    m_applicator->applyCommand(new KisQmicSynchronizeLayersCommand(m_kritaNodes, m_images, m_image, layerSize, selection), KisStrokeJobData::SEQUENTIAL, KisStrokeJobData::EXCLUSIVE);

    // 3. visit the existing nodes and reuse them to apply the remaining changes from GMic
    KisProcessingVisitorSP  importVisitor = new KisImportQmicProcessingVisitor(m_kritaNodes, m_images, layerSize, selection);
    m_applicator->applyVisitor(importVisitor, KisStrokeJobData::SEQUENTIAL); // undo information is stored in this visitor
    m_applicator->explicitlyEmitFinalSignals();
    emit gmicFinished(true, 0, "done!");
}

void KisQmicApplicator::cancel()
{
    dbgPlugins << "KisQmicApplicator::cancel";
    if (m_applicator) {

        dbgPlugins << "Cancelling applicator!";
        m_applicator->cancel();

        dbgPlugins << "deleting applicator: " << m_applicator;
        m_applicator.reset();
    }
    else  {
        dbgPlugins << "Cancelling applicator: No! Reason: Null applicator!";
    }
}

void KisQmicApplicator::finish()
{
    dbgPlugins << "Applicator " << m_applicator << " finished";
    if (m_applicator) {
        m_applicator->end();
        m_applicator.reset();
    }
}

float KisQmicApplicator::getProgress() const
{
    dbgPlugins << "KisQmicApplicator::getProgress";

    if (m_gmicData) {
        return m_gmicData->progress();
    }
    return KisQmicData::INVALID_PROGRESS_VALUE;
}
