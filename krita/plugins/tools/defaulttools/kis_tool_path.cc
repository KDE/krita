/*
 *  Copyright (c) 2007 Sven Langkamp <sven.langkamp@gmail.com>
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

#include "kis_tool_path.h"

#include <klocale.h>
#include <KoPathShape.h>
#include <KoCanvasBase.h>

#include "kis_image.h"
#include "kis_painter.h"
#include "kis_layer.h"
#include "kis_resource_provider.h"
#include "kis_paintop_registry.h"
#include "kis_brush.h"
#include "kis_selection.h"

KisToolPath::KisToolPath(KoCanvasBase * canvas)
    : KoCreatePathTool(canvas)
{
}

KisToolPath::~KisToolPath()
{
}

void KisToolPath::addPathShape()
{
    KisLayerSP currentLayer = m_canvas->resourceProvider()->resource( KisResourceProvider::CurrentKritaLayer ).value<KisLayerSP>();
    if(!currentLayer)
        return;

    KisImageSP image = currentLayer->image();
    KisPaintDeviceSP dev = currentLayer->paintDevice();

    KisBrush* brush = static_cast<KisBrush *>( m_canvas->resourceProvider()->resource( KisResourceProvider::CurrentBrush ).value<void *>());

    KisPainter painter(dev, currentLayer->selection());
    painter.beginTransaction(i18n("Path"));
    painter.setPaintColor(KoColor(Qt::black, dev->colorSpace()));
    painter.setFillStyle(KisPainter::FillStyleForegroundColor);
    painter.setStrokeStyle(KisPainter::StrokeStyleNone);
    painter.setOpacity(OPACITY_OPAQUE);
    painter.setBrush(brush);
    painter.setCompositeOp(dev->colorSpace()->compositeOp(COMPOSITE_OVER));
    KisPaintOp * op = KisPaintOpRegistry::instance()->paintOp("paintbrush", 0, &painter, image);
    painter.setPaintOp(op);

    QMatrix matrix;
    matrix.scale(image->xRes(), image->yRes());
    matrix.translate(m_shape->position().x(), m_shape->position().y());
    painter.paintPainterPath(matrix.map(m_shape->outline()));
    QRegion dirtyRegion = painter.dirtyRegion();
    dev->setDirty( dirtyRegion );
    image->setModified();

    m_canvas->addCommand(painter.endTransaction());
}

#include "kis_tool_path.moc"
