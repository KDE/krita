/* This file is part of the KDE project
 *
 * Copyright (C) 2007 Emanuele Tamponi <emanuele@valinor.it>
 * Copyright (C) 2009 Boudewijn Rempt <boud@valdyas.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */
#include "mixertool.h"

#include <QCursor>
#include <QRegion>
#include <QDomDocument>
#include <QDomElement>
#include <QString>

#include <KoPointerEvent.h>
#include <KoColor.h>
#include <KoColorSpace.h>
#include <KoCompositeOps.h>

#include <kis_image.h>
#include <kis_painter.h>
#include <kis_vec.h>
#include <kis_paintop.h>
#include <kis_paintop_registry.h>
#include <kis_debug.h>
#include <kis_painter.h>
#include <kis_paint_device.h>
#include <kis_paintop_registry.h>
#include <kis_canvas_resource_provider.h>
#include <kis_cursor.h>
#include <kis_paint_information.h>

#include "mixercanvas.h"


struct MixerTool::Private {

    Private()
    {
        cursor = KisCursor::load("tool_freehand_cursor.png", 5, 5);
    }

    KisPaintOpPresetSP mixingBrush;
    MixerCanvas *mixer;
    KoColor foregroundColor;
    KoColor backgroundColor;
    State state;
    qreal radius;
    QCursor cursor;
    QPoint currentMousePosition;
    KisPaintInformation previousPaintInformation;
    KisPainter *painter;
    double dragDist;
    const KoCompositeOp *compositeOp;
};

MixerTool::MixerTool(MixerCanvas* mixer)
    : KoToolBase(mixer)
    , m_d( new Private )
{
    m_d->mixer = mixer;
    m_d->state = MIXING;
    m_d->painter = 0;
    m_d->compositeOp = mixer->device()->colorSpace()->compositeOp(COMPOSITE_OVER);
    m_d->mixingBrush = KisPaintOpRegistry::instance()->defaultPreset(KoID("paintbrush"));
    QString preset = "<PresetResource>"
                     "<Preset name=\"my_preset\" paintopid=\"paintbrush\">"
                     "<param name=\"CurveDarken\"><![CDATA[0,0;1,1;]]></param>"
                         "<param name=\"CurveMix\"><![CDATA[0,0;1,1;]]></param>"
                         "<param name=\"CurveOpacity\"><![CDATA[0,0;1,1;]]></param>"
                         "<param name=\"CurveRotation\"><![CDATA[0,0;1,1;]]></param>"
                         "<param name=\"CurveSize\"><![CDATA[0,0;1,1;]]></param>"
                         "<param name=\"CustomDarken\"><![CDATA[false]]></param>"
                         "<param name=\"CustomMix\"><![CDATA[false]]></param>"
                         "<param name=\"CustomOpacity\"><![CDATA[true]]></param>"
                         "<param name=\"CustomRotation\"><![CDATA[true]]></param>"
                         "<param name=\"CustomSize\"><![CDATA[true]]></param>"
                         "<param name=\"DarkenSensor\"><![CDATA[pressure]]></param>"
                         "<param name=\"MixSensor\"><![CDATA[pressure]]></param>"
                         "<param name=\"OpacitySensor\"><![CDATA[pressure]]></param>"
                         "<param name=\"PaintOpAction\"><![CDATA[2]]></param>"
                         "<param name=\"PressureDarken\"><![CDATA[false]]></param>"
                         "<param name=\"PressureMix\"><![CDATA[false]]></param>"
                         "<param name=\"PressureOpacity\"><![CDATA[false]]></param>"
                         "<param name=\"PressureRotation\"><![CDATA[false]]></param>"
                         "<param name=\"PressureSize\"><![CDATA[true]]></param>"
                         "<param name=\"RotationSensor\"><![CDATA[pressure]]></param>"
                         "<param name=\"SizeSensor\"><![CDATA[pressure]]></param>"
                         "<param name=\"brush_definition\"><![CDATA[<!DOCTYPE BrushSetting>"
                 "<brush_definition brush_spacing=\"0.2\" brush_angle=\"1.570796326794897\" brush_type=\"kis_auto_brush\" autobrush_ratio=\"1\" autobrush_type=\"circle\" autobrush_hfade=\"0.11\" autobrush_spikes=\"10\" autobrush_radius=\"15\" autobrush_vfade=\"0.11\"/>"
                 "]]></param>"
                 "        <param name=\"paintop\"><![CDATA[paintbrush]]></param>"
                 "    </Preset>"
                " </PresetResource>";
    QDomDocument dom;
    dom.setContent(preset);
    QDomElement element = dom.documentElement();
    m_d->mixingBrush->fromXML(element);
    activate(KoToolBase::DefaultActivation, QSet<KoShape*>());
}

MixerTool::~MixerTool()
{
    delete m_d;
}

void MixerTool::setState(State state)
{
    m_d->state = state;
}

void MixerTool::setRadius(qreal radius)
{
    m_d->radius = radius;
    m_d->mixingBrush->settings()->changePaintOpSize(radius, radius);
}

void MixerTool::activate(ToolActivation toolActivation, const QSet<KoShape*> & shapes)
{
    KisTool::activate(toolActivation, shapes);
    Q_UNUSED(toolActivation)
    m_d->state = HOVER;
    useCursor(m_d->cursor);
    m_d->foregroundColor = canvas()->resourceManager()->resource(KoCanvasResourceManager::ForegroundColor).value<KoColor>();
    m_d->backgroundColor = canvas()->resourceManager()->resource(KoCanvasResourceManager::BackgroundColor).value<KoColor>();
}

void MixerTool::resourceChanged(int key, const QVariant & res)
{
    switch(key) {
    case KoCanvasResourceManager::ForegroundColor:
        m_d->foregroundColor = res.value<KoColor>();
        break;
    case KoCanvasResourceManager::BackgroundColor:
        m_d->backgroundColor = res.value<KoColor>();
        break;
    default:
        ;
    }
}

void MixerTool::paint(QPainter &painter, const KoViewConverter &converter)
{
    Q_UNUSED(converter);

    if (m_d->mixer->hasFocus()) {
        /**
         * We don't use paintOutline anymore. Now the tool needs
         * to request QPainterPath from the paint op with brushOutline()
         * and paint it itself. Consult with the code in KisToolFreehand.
         */
        //m_d->mixingBrush->settings()->paintOutline(m_d->currentMousePosition,
        //                                           0,
        //                                           painter,
        //                                           KisPaintOpSettings::CursorIsOutline);
    }
}

void MixerTool::mousePressEvent(KoPointerEvent *event)
{
    m_d->currentMousePosition = event->pos();

    if (event->button() == Qt::LeftButton) {
        m_d->state = MIXING;
        m_d->dragDist = 0;

        // Create painter
        KisPaintDeviceSP device = m_d->mixer->device();

        if (m_d->painter) delete m_d->painter;

        m_d->painter = new KisPainter(device);
        m_d->painter->setCompositeOp(m_d->compositeOp);
        m_d->painter->setPaintColor(m_d->foregroundColor);
        m_d->painter->setBackgroundColor(m_d->backgroundColor);
        m_d->painter->setPaintOpPreset(m_d->mixingBrush, 0);
        m_d->previousPaintInformation =
                KisPaintInformation(event->point,
                                    event->pressure(), event->xTilt(), event->yTilt(),
                                    KisVector2D::Zero(),
                                    event->rotation(), event->tangentialPressure());
        m_d->painter->paintAt(m_d->previousPaintInformation);
        m_d->mixer->updateCanvas(m_d->painter->takeDirtyRegion());
    }
}

void MixerTool::mouseMoveEvent(KoPointerEvent *event)
{
    m_d->currentMousePosition = event->pos();
    switch(m_d->state) {
    case MIXING:
        {
            QPointF dragVec = event->point - m_d->previousPaintInformation.pos();

            KisPaintInformation info = KisPaintInformation(event->point,
                                                           event->pressure(), event->xTilt(), event->yTilt(),
                                                           toKisVector2D(dragVec),
                                                           event->rotation(), event->tangentialPressure());

            m_d->painter->paintLine(m_d->previousPaintInformation, info);
            m_d->previousPaintInformation = info;
            m_d->mixer->updateCanvas(m_d->painter->takeDirtyRegion());
        }
        break;
    case PANNING:
        break;
    case PICKING:
        break;
    default:
        ;
    };
}

void MixerTool::mouseReleaseEvent(KoPointerEvent *event)
{
    // XXX: We want to be able to set a color source for the other paintops that
    //      contains the impure blend under the current cursor.
    if (m_d->state == MIXING) {
        m_d->state = HOVER;
        m_d->mixer->updateCanvas(m_d->painter->takeDirtyRegion());
        delete m_d->painter;
        m_d->painter = 0;
        m_d->mixer->device()->convertToQImage(0).save("canvas.png");

    }
    m_d->mixer->resourceManager()->setResource(KoCanvasResourceManager::ForegroundColor, event->pos());
}

void MixerTool::setDirty(const QRegion& region)
{
    m_d->mixer->updateCanvas(region.rects());
}



#include "mixertool.moc"
