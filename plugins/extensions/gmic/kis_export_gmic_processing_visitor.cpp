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

#include "kis_export_gmic_processing_visitor.h"

#include <kis_gmic_simple_convertor.h>
#include <kis_node.h>
#include <kis_debug.h>

KisExportGmicProcessingVisitor::KisExportGmicProcessingVisitor(const KisNodeListSP nodes, QSharedPointer<gmic_list<float> > images, QRect rc)
    : m_nodes(nodes),
      m_images(images),
      m_rc(rc)
{
}


void KisExportGmicProcessingVisitor::visitNodeWithPaintDevice(KisNode *node, KisUndoAdapter *undoAdapter)
{
    Q_UNUSED(undoAdapter);

    int index = m_nodes->indexOf(node);
    if (index >= 0)
    {
        /* fill the image with data here */
        KisPaintDeviceSP device = node->paintDevice();
        gmic_image<float> &gimg = m_images->_data[index];

        quint32 x = m_rc.width();
        quint32 y = m_rc.height();
        quint32 z = 1;
        quint32 colorChannelCount = 4; // RGBA
        gimg.assign(x,y,z,colorChannelCount);
        KisGmicSimpleConvertor::convertToGmicImageFast(device, gimg, m_rc);
    }
}

