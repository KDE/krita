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

void KisGmicApplicator::apply(KisImageWSP image, KisNodeSP node, const QString &actionName, KisNodeListSP kritaNodes, const QString &gmicCommand)
{
    KisImageSignalVector emitSignals;
    emitSignals << ModifiedSignal;

    KisProcessingApplicator applicator(image, node,
                                   KisProcessingApplicator::RECURSIVE,
                                   emitSignals, actionName);


    QSharedPointer< gmic_list<float> > gmicLayers(new gmic_list<float>);
    gmicLayers->assign(kritaNodes->size());

    QRect layerSize(0,0,image->width(), image->height());
    KisProcessingVisitorSP visitor;

    // convert krita layers to gmic layers
    visitor = new KisExportGmicProcessingVisitor(kritaNodes, gmicLayers, layerSize);
    applicator.applyVisitor(visitor, KisStrokeJobData::CONCURRENT);

    // apply gmic filters to provided layers
    applicator.applyCommand(new KisGmicCommand(gmicCommand, gmicLayers));

    // synchronize layer count
    applicator.applyCommand(new KisGmicSynchronizeLayersCommand(kritaNodes, gmicLayers, image), KisStrokeJobData::SEQUENTIAL, KisStrokeJobData::EXCLUSIVE);

    // would sleep(3) help here?
    visitor = new KisImportGmicProcessingVisitor(kritaNodes, gmicLayers);
    applicator.applyVisitor(visitor, KisStrokeJobData::CONCURRENT); // undo information is stored in this visitor
    applicator.end();
}
