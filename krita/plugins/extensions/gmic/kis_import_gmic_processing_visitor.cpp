/*
 *  Copyright (c) 2013 Dmitry Kazakov <dimula73@gmail.com>
 *  Copyright (c) 2013 Lukáš Tvrdý <lukast.dev@gmail.com>
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



#include <kis_transaction.h>
#include <kis_paint_device.h>
#include <kis_debug.h>

#include "kis_gmic_simple_convertor.h"
#include <kis_node.h>
#include <kis_painter.h>
#include <commands/kis_image_layer_add_command.h>
#include <kis_paint_layer.h>
#include <KoCompositeOpRegistry.h>
#include <kis_selection.h>
#include <kis_types.h>



#include "kis_import_gmic_processing_visitor.h"
#include <gmic.h>

KisImportGmicProcessingVisitor::KisImportGmicProcessingVisitor(const KisNodeListSP nodes,QSharedPointer<gmic_list<float> > images, const QRect &dstRect, KisSelectionSP selection)
    : m_nodes(nodes),
      m_images(images),
      m_dstRect(dstRect),
      m_selection(selection)
{
}

void KisImportGmicProcessingVisitor::visitNodeWithPaintDevice(KisNode *node, KisUndoAdapter *undoAdapter)
{
    int index = m_nodes->indexOf(node);
    if (index >= 0)
    {
        gmic_image<float> &gimg = m_images->_data[index];
        dbgPlugins << "Importing layer index" << index << "Size: "<< gimg._width << "x" << gimg._height << "colorchannels: " << gimg._spectrum;

        KisPaintDeviceSP dst = node->paintDevice();
        KisTransaction transaction(dst);
        KisImportGmicProcessingVisitor::gmicImageToPaintDevice(gimg, dst, m_selection, m_dstRect);
        if (undoAdapter)
        {
            transaction.commit(undoAdapter);
            node->setDirty(m_dstRect);
        }
    }
}

void KisImportGmicProcessingVisitor::gmicImageToPaintDevice(cimg_library::CImg< float >& srcGmicImage,
                                                            KisPaintDeviceSP dst, KisSelectionSP selection, const QRect &dstRect)
{
        if (selection)
        {
            KisPaintDeviceSP src = new KisPaintDevice(dst->colorSpace());
            KisGmicSimpleConvertor::convertFromGmicFast(srcGmicImage, src, 255.0f);

            KisPainter painter(dst, selection);
            painter.bitBlt(dstRect.topLeft(), src, QRect(QPoint(0,0),dstRect.size()));
        }
        else
        {
            KisGmicSimpleConvertor::convertFromGmicFast(srcGmicImage, dst, 255.0f);
        }
}


void KisImportGmicProcessingVisitor::visitExternalLayer(KisExternalLayer *layer, KisUndoAdapter *undoAdapter)
{
    Q_UNUSED(layer);
    Q_UNUSED(undoAdapter);
}
