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

#include <kis_gmic_applicator.h>
#include <kis_image_signal_router.h>
#include <kis_processing_applicator.h>
#include <kis_gmic_data.h>

#include "kis_gmic_command.h"
#include "kis_import_gmic_processing_visitor.h"
#include "kis_image.h"
#include <kis_selection.h>
#include <KoUpdater.h>

#include <gmic.h>
#include "kis_gmic_synchronize_layers_command.h"
#include "kis_export_gmic_processing_visitor.h"
#include "kis_gmic_synchronize_image_size_command.h"

KisGmicApplicator::KisGmicApplicator():m_applicator(0),m_applicatorStrokeEnded(false)
{
}

KisGmicApplicator::~KisGmicApplicator()
{
    dbgPlugins << "Destructor: " << m_applicator;
    delete m_applicator;
}

void KisGmicApplicator::setProperties(KisImageWSP image, KisNodeSP node, const KUndo2MagicString &actionName, KisNodeListSP kritaNodes, const QString &gmicCommand, const QByteArray customCommands)
{
    m_image = image;
    m_node = node;
    m_actionName = actionName;
    m_kritaNodes = kritaNodes;
    m_gmicCommand = gmicCommand;
    m_customCommands = customCommands;
}


void KisGmicApplicator::preview()
{
    // cancel previous preview if there is one
    dbgPlugins << "Request for preview, cancelling any previous possible on-canvas preview";
    cancel();

    KisImageSignalVector emitSignals;
    emitSignals << ComplexSizeChangedSignal() << ModifiedSignal;


    m_applicator = new KisProcessingApplicator(m_image, m_node,
            KisProcessingApplicator::RECURSIVE | KisProcessingApplicator::NO_UI_UPDATES,
            emitSignals, m_actionName);
    dbgPlugins << "Created applicator " << m_applicator;

    QSharedPointer< gmic_list<float> > gmicLayers(new gmic_list<float>);
    gmicLayers->assign(m_kritaNodes->size());

    m_gmicData = KisGmicDataSP(new KisGmicData());

    QRect layerSize;
    KisSelectionSP selection = m_image->globalSelection();
    if (selection)
    {
        layerSize = selection->selectedExactRect();
    }
    else
    {
        layerSize = QRect(0,0,m_image->width(), m_image->height());
    }

    // convert krita layers to gmic layers
    KisProcessingVisitorSP exportVisitor = new KisExportGmicProcessingVisitor(m_kritaNodes, gmicLayers, layerSize);
    m_applicator->applyVisitor(exportVisitor, KisStrokeJobData::CONCURRENT);

    // apply gmic filters to provided layers
    KisGmicCommand * gmicCommand = new KisGmicCommand(m_gmicCommand, gmicLayers, m_gmicData, m_customCommands);
    connect(gmicCommand, SIGNAL(gmicFinished(bool, int, QString)), this, SIGNAL(gmicFinished(bool,int,QString)));

    m_applicator->applyCommand(gmicCommand);

    // synchronize Krita image size with biggest gmic layer size
    m_applicator->applyCommand(new KisGmicSynchronizeImageSizeCommand(gmicLayers, m_image));

    // synchronize layer count
    m_applicator->applyCommand(new KisGmicSynchronizeLayersCommand(m_kritaNodes, gmicLayers, m_image, layerSize, selection), KisStrokeJobData::SEQUENTIAL, KisStrokeJobData::EXCLUSIVE);

    KisProcessingVisitorSP  importVisitor = new KisImportGmicProcessingVisitor(m_kritaNodes, gmicLayers, layerSize, selection);
    m_applicator->applyVisitor(importVisitor, KisStrokeJobData::SEQUENTIAL); // undo information is stored in this visitor
    m_applicator->explicitlyEmitFinalSignals();
}

void KisGmicApplicator::cancel()
{
    if (m_gmicData)
    {
        dbgPlugins << "Cancel gmic script";
        m_gmicData->setCancel(true);
    }

    if (m_applicator)
    {

        if (!m_applicatorStrokeEnded)
        {
            dbgPlugins << "Cancelling applicator: Yes!";
            m_applicator->cancel();
        }
        else
        {
            dbgPlugins << "Cancelling applicator: No! Reason: Already finished!";
        }


        dbgPlugins << "deleting applicator: " << m_applicator;
        delete m_applicator;
        m_applicator = 0;


        m_applicatorStrokeEnded = false;
        dbgPlugins << ppVar(m_applicatorStrokeEnded);
    }
    else
    {
        dbgPlugins << "Cancelling applicator: No! Reason: Null applicator!";
    }
}

void KisGmicApplicator::finish()
{
    dbgPlugins << "aplicator " << m_applicator << " finished";
    if (m_applicator)
    {
        m_applicator->end();
        m_applicatorStrokeEnded = true;
    }
    dbgPlugins << ppVar(m_applicatorStrokeEnded);
}

float KisGmicApplicator::getProgress() const
{
    if (m_gmicData)
    {
        m_gmicData->progress();
    }
    return KisGmicData::INVALID_PROGRESS_VALUE;
}
