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

#include "kis_import_gmic_processing_visitor.h"

#include <kis_transaction.h>
#include <kis_paint_device.h>
#include <kis_debug.h>

#include <gmic.h>
#include "kis_gmic_simple_convertor.h"
#include <kis_node.h>
#include <kis_painter.h>
#include <KoCompositeOpRegistry.h>

KisImportGmicProcessingVisitor::KisImportGmicProcessingVisitor(const QList<KisNodeSP> &nodes,QSharedPointer<gmic_list<float> > images)
    : m_nodes(nodes),
      m_images(images)
{
}

void KisImportGmicProcessingVisitor::visitNodeWithPaintDevice(KisNode *node, KisUndoAdapter *undoAdapter)
{
    int index = m_nodes.indexOf(node);
    if (index >= 0)
    {
        gmic_image<float> &gimg = m_images->_data[index];

        dbgPlugins << "Importing: " << gimg._width << gimg._height << gimg._spectrum;


        KisPaintDeviceSP src = node->paintDevice();
        KisTransaction transaction("", src);

        bool preserveAlpha = false;

        KisGmicSimpleConvertor convertor;
        KisPaintDeviceSP dstDev = convertor.convertFromGmicImage(m_images->_data[index], preserveAlpha);


        // to actual layer colorspace
        dstDev->convertTo(src->colorSpace());
        // bitBlt back -- we don't write the pixel data back directly, but bitBlt so the
        // unselected pixels are not overwritten.
        KisPainter gc(src);
        gc.setCompositeOp(COMPOSITE_COPY);

        // preserve alpha if grayscale or RGB output
        QRect rc(0,0,m_images->_data[index]._width, m_images->_data[index]._height);
        if (preserveAlpha)
        {
            gc.setLockAlpha(true);
        }
        gc.bitBlt(rc.topLeft(), dstDev, rc);
        gc.end();

        transaction.commit(undoAdapter);
    }
}

void KisImportGmicProcessingVisitor::visitExternalLayer(KisExternalLayer *layer, KisUndoAdapter *undoAdapter)
{
    Q_UNUSED(layer);
    Q_UNUSED(undoAdapter);
}
