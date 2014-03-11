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

#include <gmic.h>

#include <kis_gmic_applicator.h>
#include <kis_image_signal_router.h>
#include <kis_processing_applicator.h>
#include "kis_export_gmic_processing_visitor.h"
#include "kis_gmic_synchronize_layers_command.h"
#include "kis_gmic_command.h"
#include "kis_import_gmic_processing_visitor.h"
#include "kis_image.h"

KisGmicApplicator::KisGmicApplicator()
{
}


KisGmicApplicator::~KisGmicApplicator()
{
}

void KisGmicApplicator::setProperties(KisImageWSP image, KisNodeSP node, const QString &actionName, KisNodeListSP kritaNodes, const QString &gmicCommand)
{
    m_image = image;
    m_node = node;
    m_actionName = actionName;
    m_kritaNodes = kritaNodes;
    m_gmicCommand = gmicCommand;
}

void KisGmicApplicator::run()
{
    KisImageSignalVector emitSignals;
    emitSignals << ModifiedSignal;

    KisProcessingApplicator applicator(m_image, m_node,
                                       KisProcessingApplicator::RECURSIVE,
                                       emitSignals, m_actionName);


    QSharedPointer< gmic_list<float> > gmicLayers(new gmic_list<float>);
    gmicLayers->assign(m_kritaNodes->size());

    QRect layerSize(0, 0, m_image->width(), m_image->height());
    KisProcessingVisitorSP visitor;

    // convert krita layers to gmic layers
    visitor = new KisExportGmicProcessingVisitor(m_kritaNodes, gmicLayers, layerSize);
    applicator.applyVisitor(visitor, KisStrokeJobData::CONCURRENT);

    // apply gmic filters to provided layers
    applicator.applyCommand(new KisGmicCommand(m_gmicCommand, gmicLayers));

    // synchronize layer count
    applicator.applyCommand(new KisGmicSynchronizeLayersCommand(m_kritaNodes, gmicLayers, m_image), KisStrokeJobData::SEQUENTIAL, KisStrokeJobData::EXCLUSIVE);

    // would sleep(3) help here?
    visitor = new KisImportGmicProcessingVisitor(m_kritaNodes, gmicLayers);
    applicator.applyVisitor(visitor, KisStrokeJobData::CONCURRENT); // undo information is stored in this visitor
    applicator.end();
}
