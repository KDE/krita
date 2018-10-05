/*
 *  Copyright (c) 2013 Dmitry Kazakov <dimula73@gmail.com>
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

#include "kis_paintop_transformation_connector.h"

#include "kis_canvas_resource_provider.h"
#include "kis_canvas2.h"
#include "kis_coordinates_converter.h"
#include "brushengine/kis_paintop_preset.h"
#include "brushengine/kis_paintop_settings.h"


KisPaintopTransformationConnector::KisPaintopTransformationConnector(KisCanvas2 *canvas, QObject *parent)
    : QObject(parent),
      m_canvas(canvas)
{
    connect(m_canvas->resourceManager(),
            SIGNAL(canvasResourceChanged(int,QVariant)),
            SLOT(slotCanvasResourceChanged(int,QVariant)));
}

void KisPaintopTransformationConnector::notifyTransformationChanged()
{
    KisPaintOpPresetSP preset =
        m_canvas->resourceManager()->
        resource(KisCanvasResourceProvider::CurrentPaintOpPreset).value<KisPaintOpPresetSP>();

    if (preset) {
        const KisCoordinatesConverter *converter = m_canvas->coordinatesConverter();
        preset->settings()->setCanvasRotation(converter->rotationAngle());
        preset->settings()->setCanvasMirroring(converter->xAxisMirrored(),
                                               converter->yAxisMirrored());
    }
}

void KisPaintopTransformationConnector::slotCanvasResourceChanged(int key, const QVariant &resource)
{
    Q_UNUSED(resource);

    if (key == KisCanvasResourceProvider::CurrentPaintOpPreset) {
        notifyTransformationChanged();
    }
}
