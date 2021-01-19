/*
 * SPDX-FileCopyrightText: 2013 Lukáš Tvrdý <lukast.dev@gmail.com
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <kis_qmic_synchronize_layers_command.h>
#include "kis_import_qmic_processing_visitor.h"
#include <kis_paint_layer.h>
#include <kis_image.h>
#include <kis_selection.h>
#include <kis_types.h>
#include <KoColorSpaceConstants.h>

#include <commands/kis_image_layer_add_command.h>

KisQmicSynchronizeLayersCommand::KisQmicSynchronizeLayersCommand(KisNodeListSP nodes, QVector<gmic_image<float> *> images, KisImageWSP image, const QRect &dstRect, KisSelectionSP selection)
    :   KUndo2Command(),
        m_nodes(nodes),
        m_images(images),
        m_image(image),
        m_dstRect(dstRect),
        m_selection(selection),
        m_firstRedo(true)
{
    dbgPlugins << "KisQmicSynchronizeLayersCommand";
}

KisQmicSynchronizeLayersCommand::~KisQmicSynchronizeLayersCommand()
{
    qDeleteAll(m_imageCommands);
    m_imageCommands.clear();
}

void KisQmicSynchronizeLayersCommand::redo()
{
    dbgPlugins << "KisQmicSynchronizeLayersCommand::Redo" << m_firstRedo;

    if (m_firstRedo) {
        // if gmic produces more layers
        if (m_nodes->size() < m_images.size()) {

            if (m_image) {

                int nodesCount = m_nodes->size();
                for (int i = nodesCount; i < m_images.size(); i++) {

                    KisPaintDevice * device = new KisPaintDevice(m_image->colorSpace());
                    KisLayerSP paintLayer = new KisPaintLayer(m_image, QString("New layer %1 from gmic filter").arg(i), OPACITY_OPAQUE_U8, device);

                    KisImportQmicProcessingVisitor::gmicImageToPaintDevice(*m_images[i], device);

                    // This node is a copy made by GMic of an existing node;
                    // give it its name back (the existing node will be reused
                    // by KisImportQmicProcessingVisitor)
                    paintLayer->setName(m_nodes->at(i - nodesCount)->name());

                    KisNodeSP aboveThis = m_nodes->last()->prevSibling();
                    KisNodeSP parent = m_nodes->at(0)->parent();

                    dbgPlugins << "Adding paint layer" << (i - nodesCount + 1)
                               << paintLayer << "to parent" << parent->name()
                               << "above" << aboveThis;
                    auto *addLayerCmd = new KisImageLayerAddCommand(
                        m_image, paintLayer, parent, aboveThis, false, true);
                    addLayerCmd->redo();
                    m_imageCommands.append(addLayerCmd);
                    m_nodes->append(paintLayer);

                }
            }
            else // small preview
            {
                Q_ASSERT(m_nodes->size() > 0);
                for (int i = m_nodes->size(); i < m_images.size(); i++) {
                    KisPaintDevice * device = new KisPaintDevice(m_nodes->at(0)->colorSpace());
                    KisLayerSP paintLayer = new KisPaintLayer(0, "New layer from gmic filter", OPACITY_OPAQUE_U8, device);
                    m_nodes->append(paintLayer);
                }
            }
        } // if gmic produces less layers, we are going to drop some
        else if (m_nodes->size() > int(m_images.size()))
        {
            dbgPlugins << "no support for removing layers yet!!";
        }
    }
    else
    {
        dbgPlugins << "Redo again needed?";
    }
}

void KisQmicSynchronizeLayersCommand::undo()
{
    KisImageCommand * cmd;
    Q_FOREACH (cmd, m_imageCommands)
    {
        cmd->undo();
    }
}
