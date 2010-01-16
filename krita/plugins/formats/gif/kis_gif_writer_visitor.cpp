/*
 *  Copyright (c) 2010 Boudewijn Rempt
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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "kis_gif_writer_visitor.h"

#include <QImage>

#include <kis_paint_device.h>
#include <kis_group_layer.h>
#include <kis_image.h>
#include <kis_paint_layer.h>
#include <kis_types.h>
#include <generator/kis_generator_layer.h>

bool KisGifWriterVisitor::visit(KisPaintLayer *layer)
{
    return saveLayerProjection(layer);
}

bool KisGifWriterVisitor::visit(KisGroupLayer *layer)
{
    return visitAll(layer, true);
}

bool KisGifWriterVisitor::visit(KisGeneratorLayer* layer)
{
    // a generator layer has a nice paint device we can save.
    return saveLayerProjection(layer);
}

bool KisGifWriterVisitor::saveLayerProjection(KisLayer* layer)
{
    dbgFile << "converting layer" << layer->name();
    QRect bounds = layer->exactBounds();
    QImage layerImage = layer->projection()->convertToQImage(0);
    IndexedLayer indexedLayer;
    indexedLayer.topLeft = QPoint(bounds.x(), bounds.y());
    indexedLayer.image = layerImage.convertToFormat(QImage::Format_Indexed8);
    m_layers << indexedLayer;

    return true;
}
