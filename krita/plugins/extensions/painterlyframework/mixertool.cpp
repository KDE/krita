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
    bool mouseDown;
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
    m_d->mixingBrush = KisPaintOpRegistry::instance()->defaultPreset(KoID("complex"));
    activate();
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

void MixerTool::activate(bool temporary)
{
    Q_UNUSED(temporary)
    m_d->mouseDown = false;

    useCursor(m_d->cursor);
    m_d->foregroundColor = canvas()->resourceManager()->resource(KoCanvasResource::ForegroundColor).value<KoColor>();
    m_d->backgroundColor = canvas()->resourceManager()->resource(KoCanvasResource::BackgroundColor).value<KoColor>();
}

void MixerTool::deactivate()
{
}

void MixerTool::resourceChanged(int key, const QVariant & res)
{
    switch(key) {
    case KoCanvasResource::ForegroundColor:
        m_d->foregroundColor = res.value<KoColor>();
        break;
    case KoCanvasResource::BackgroundColor:
        m_d->backgroundColor = res.value<KoColor>();
        break;
    default:
        ;
    }
}

void MixerTool::paint(QPainter &painter, const KoViewConverter &converter)
{
    if (m_d->mixer->hasFocus()) {
        m_d->mixingBrush->settings()->paintOutline(m_d->currentMousePosition,
                                                   0,
                                                   painter,
                                                   converter,
                                                   KisPaintOpSettings::CURSOR_IS_OUTLINE);
    }
}

void MixerTool::mousePressEvent(KoPointerEvent *event)
{
    m_d->currentMousePosition = event->pos();
    m_d->mouseDown = true;

    if (event->button() == Qt::LeftButton) {
        initPaint(event);
        m_d->previousPaintInformation =
                KisPaintInformation(event->point,
                                    event->pressure(), event->xTilt(), event->yTilt(),
                                    KisVector2D::Zero(),
                                    event->rotation(), event->tangentialPressure());
        paintAt(m_d->previousPaintInformation);
    }
}

void MixerTool::mouseMoveEvent(KoPointerEvent *event)
{
    m_d->currentMousePosition = event->pos();
    if (m_d->mouseDown) {
        switch(m_d->state) {
        case MIXING:
            {
                QPointF dragVec = event->point - m_d->previousPaintInformation.pos();

                KisPaintInformation info = KisPaintInformation(event->point,
                                                               event->pressure(), event->xTilt(), event->yTilt(),
                                                               toKisVector2D(dragVec),
                                                               event->rotation(), event->tangentialPressure());

                paintLine(m_d->previousPaintInformation, info);
                m_d->previousPaintInformation = info;
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
}

void MixerTool::mouseReleaseEvent(KoPointerEvent *event)
{
    // XXX: We want to be able to set a color source for the other paintops that
    //      contains the impure blend under the current cursor.
    m_d->mixer->resourceManager()->setResource(KoCanvasResource::ForegroundColor, event->pos());
    m_d->mouseDown = false;
}

void MixerTool::setDirty(const QRegion& region)
{
    m_d->mixer->updateCanvas(region);
}

void MixerTool::initPaint(KoPointerEvent *)
{
    m_d->state = MIXING;
    m_d->dragDist = 0;

    // Create painter
    KisPaintDeviceSP device = m_d->mixer->device();

    if (m_d->painter) delete m_d->painter;

    m_d->painter = new KisPainter(device);
    // XXX: setup the painter
    m_d->painter->setCompositeOp(m_d->compositeOp);
}

void MixerTool::endPaint()
{
    m_d->state = HOVER;
//    if (m_d->painter) {
//
//        KisPainter painter(m_source, currentSelection());
//        painter.setCompositeOp(m_compositeOp);
//        painter.setOpacity(m_opacity);
//
//        QRegion r = m_incrementalDirtyRegion;
//        foreach(const QRect& rc, r.rects()) {
//            painter.bitBlt(rc.topLeft(), m_target, rc);
//        }
//        m_source->setDirty(painter.dirtyRegion());
//        delete incrementalTransaction;
//    }
    delete m_d->painter;
    m_d->painter = 0;
}

void MixerTool::paintAt(const KisPaintInformation &pi)
{
}

void MixerTool::paintLine(const KisPaintInformation &pi1, const KisPaintInformation &pi2)
{
}


#include "mixertool.moc"
