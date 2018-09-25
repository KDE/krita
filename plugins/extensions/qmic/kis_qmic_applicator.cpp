/*
 * Copyright (c) 2013 Lukáš Tvrdý <lukast.dev@gmail.com
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
#include "kis_qmic_applicator.h"

#include <kis_image_signal_router.h>
#include <kis_processing_applicator.h>

#include <kis_image.h>
#include <kis_selection.h>

#include "kis_import_qmic_processing_visitor.h"
#include "kis_qmic_synchronize_layers_command.h"
#include "kis_qmic_synchronize_image_size_command.h"

KisQmicApplicator::KisQmicApplicator():m_applicator(0),m_applicatorStrokeEnded(false)
{
}

KisQmicApplicator::~KisQmicApplicator()
{
    delete m_applicator;
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
    emitSignals << ComplexSizeChangedSignal() << ModifiedSignal;

    m_applicator = new KisProcessingApplicator(m_image, m_node,
            KisProcessingApplicator::RECURSIVE | KisProcessingApplicator::NO_UI_UPDATES,
            emitSignals, m_actionName);
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

   if (!selection) {
        // synchronize Krita image size with biggest gmic layer size
        m_applicator->applyCommand(new KisQmicSynchronizeImageSizeCommand(m_images, m_image));
    }

    // synchronize layer count
    m_applicator->applyCommand(new KisQmicSynchronizeLayersCommand(m_kritaNodes, m_images, m_image, layerSize, selection), KisStrokeJobData::SEQUENTIAL, KisStrokeJobData::EXCLUSIVE);

    KisProcessingVisitorSP  importVisitor = new KisImportQmicProcessingVisitor(m_kritaNodes, m_images, layerSize, selection);
    m_applicator->applyVisitor(importVisitor, KisStrokeJobData::SEQUENTIAL); // undo information is stored in this visitor
    m_applicator->explicitlyEmitFinalSignals();
    emit gmicFinished(true, 0, "done!");
}

void KisQmicApplicator::cancel()
{
    dbgPlugins << "KisQmicApplicator::cancel";
    if (m_applicator) {

        if (!m_applicatorStrokeEnded) {
            dbgPlugins << "Cancelling applicator: Yes!";
            m_applicator->cancel();
        }
        else {
            dbgPlugins << "Cancelling applicator: No! Reason: Already finished!";
        }

        dbgPlugins << "deleting applicator: " << m_applicator;
        delete m_applicator;
        m_applicator = 0;

        m_applicatorStrokeEnded = false;
        dbgPlugins << ppVar(m_applicatorStrokeEnded);
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
        m_applicatorStrokeEnded = true;
    }
    dbgPlugins << ppVar(m_applicatorStrokeEnded);
}

float KisQmicApplicator::getProgress() const
{
    dbgPlugins << "KisQmicApplicator::getProgress";

    if (m_gmicData) {
        m_gmicData->progress();
    }
    return KisQmicData::INVALID_PROGRESS_VALUE;
}
