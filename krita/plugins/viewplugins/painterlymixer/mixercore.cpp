/* This file is part of the KDE project
   Made by Emanuele Tamponi (emanuele@valinor.it)
   Copyright (C) 2007 Emanuele Tamponi

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#include <cmath>

#include <QCursor>
#include <QFrame>

#include <kdebug.h>

#include <KoCanvasBase.h>
#include <KoCanvasResourceProvider.h>
#include <KoColor.h>
#include <KoColorSpace.h>
#include <KoID.h>
#include <KoPointerEvent.h>
#include <KoShapeManager.h>
#include <KoToolProxy.h>
#include <KoUnit.h>
#include <KoViewConverter.h>

#include "kis_iterators_pixel.h"
#include "kis_paint_device.h"
#include "kis_painter.h"
#include "kis_paintop.h"
#include "kis_paintop_registry.h"
#include "kis_resource_provider.h"

#include "kis_adsorbency_mask.h"
#include "kis_pigment_concentration_mask.h"
#include "kis_reflectivity_mask.h"
#include "kis_volume_mask.h"
#include "kis_viscosity_mask.h"
#include "kis_wetness_mask.h"

#include "mixercore.h"
#include "kis_painterly_information.h"

/*
Just add all the painterly overlays that we need in only one call
*/
void addPainterlyOverlays(KisPaintDeviceSP dev)
{
    dev->addPainterlyChannel(new KisAdsorbencyMask(dev));
    dev->addPainterlyChannel(new KisPigmentConcentrationMask(dev));
    dev->addPainterlyChannel(new KisReflectivityMask(dev));
    dev->addPainterlyChannel(new KisVolumeMask(dev));
    dev->addPainterlyChannel(new KisViscosityMask(dev));
    dev->addPainterlyChannel(new KisWetnessMask(dev));
}

MixerCanvas::MixerCanvas(QWidget *parent)
    : QFrame(parent), KoCanvasBase(0), m_tool(0), m_toolProxy(0)
{

}

MixerCanvas::~MixerCanvas()
{
    if (m_toolProxy)
        delete m_toolProxy;
    if (m_tool)
        delete m_tool;
}

void MixerCanvas::initDevice(KoColorSpace *cs, KoCanvasResourceProvider *rp)
{
    m_canvasDev = new KisPaintDevice(cs);
    addPainterlyOverlays(m_canvasDev);

    m_tool = new MixerTool(this, m_canvasDev, rp);
    m_toolProxy = new KoToolProxy(this);
    m_toolProxy->setActiveTool(m_tool);
}

void MixerCanvas::initSpots(QFrame *sf)
{
    // TODO Initialize spots
}

void MixerCanvas::mouseDoubleClickEvent(QMouseEvent *event)
{
    m_toolProxy->mouseDoubleClickEvent(event, event->pos());
}

void MixerCanvas::mouseMoveEvent(QMouseEvent *event)
{
    m_toolProxy->mouseMoveEvent(event, event->pos());
}

void MixerCanvas::mousePressEvent(QMouseEvent *event)
{
    m_toolProxy->mousePressEvent(event, event->pos());
}

void MixerCanvas::mouseReleaseEvent(QMouseEvent *event)
{
    m_toolProxy->mouseReleaseEvent(event, event->pos());
}

void MixerCanvas::tabletEvent(QTabletEvent *event)
{
    m_toolProxy->tabletEvent(event, event->pos());
}

void MixerCanvas::paintEvent(QPaintEvent *event)
{
    QFrame::paintEvent(event);

    QRect r = rect();
    QPainter p(this);
    p.drawImage(r, m_canvasDev->convertToQImage(0, r.x(), r.y(), r.width(), r.height()), r);
    p.end();
}

void MixerCanvas::updateCanvas(const QRectF& rc)
{
    update(rc.toRect());
}

/////////////////
// THE MIXER TOOL
/////////////////

MixerTool::MixerTool(MixerCanvas *canvas, KisPaintDeviceSP device, KoCanvasResourceProvider *rp)
    : KoTool(canvas), m_canvasDev(device), m_resources(rp)
{
    //{{ - Just for testing!
    m_info.PaintType = "Test";
    m_info.Mixability = 0.4f;
    m_info.PigmentConcentration = 0.9f;
    m_info.PaintVolume = 200.0f;
    m_info.Reflectivity = 0.1f;
    m_info.Viscosity = 0.6f;
    m_info.Wetness = 0.5f;
    //}}
}

MixerTool::~MixerTool()
{
}

void MixerTool::mousePressEvent(KoPointerEvent */*e*/)
{
//     kDebug() << "MOUSE PRESSED!! " << event->pos() << endl;
}

void MixerTool::mouseReleaseEvent(KoPointerEvent */*e*/)
{
//     kDebug() << "MOUSE RELEASED!! " << event->pos() << endl;
}

void MixerTool::mouseMoveEvent(KoPointerEvent *e)
{
    KisPaintDeviceSP stroke = new KisPaintDevice(m_canvasDev->colorSpace());
    addPainterlyOverlays(stroke);

    //{{ KisPainter initialization - Put it in another function?
    KisPainter painter(stroke);
    KisPaintOp *current = KisPaintOpRegistry::instance()->paintOp(
                          m_resources->resource(KisResourceProvider::CurrentPaintop).value<KoID>(),
                          static_cast<KisPaintOpSettings*>(m_resources->resource(KisResourceProvider::CurrentPaintopSettings).value<void*>()),
                          &painter, 0);
    painter.setPaintOp(current); // The painter now has the paintop and will destroy it.
    painter.setPaintColor(m_resources->resource(KoCanvasResource::ForegroundColor).value<KoColor>());
    painter.setBackgroundColor(m_resources->resource(KoCanvasResource::ForegroundColor).value<KoColor>());
    painter.setBrush(static_cast<KisBrush*>(m_resources->resource(KisResourceProvider::CurrentBrush).value<void*>()));
    //}}

    if (current->painterly()) {
        QRect rc = m_canvasDev->exactBounds();
        painter.bitBlt(rc.topLeft(), m_canvasDev, rc);
        painter.end();
    }

    painter.paintAt(KisPaintInformation(e->pos(), e->pressure(), e->xTilt(), e->yTilt()));
    painter.end();

    if (!current->painterly()) {
        updatePainterlyOverlays(stroke, e);
        mixColors(stroke, e);
        updateResources(stroke);
    }

    QRect rc = stroke->exactBounds();
    painter.begin(m_canvasDev);
    painter.bitBlt(rc.topLeft(), stroke, rc);
    painter.copyMasks(rc.topLeft(), stroke, rc);
    painter.end();

    m_canvas->updateCanvas(rc);
}

#define RHO2(x,y) x*x + y*y

void MixerTool::updatePainterlyOverlays(KisPaintDeviceSP stroke, KoPointerEvent *e)
{
    float p, f; // Pressure and force
    float rho2; // Square of the distance from the event point
    int x, y; // Actual point
    float wetness; // Actual point wetness
    float p0 = e->pressure();
    int x0 = e->x(), y0 = e->y();
    QRect rc = stroke->exactBounds();
    QColor c; quint8 opacity;

    KisRectIteratorPixel             // Give a more or less clear name to each iterator.
        it_main = stroke->createRectIterator(rc.x(),rc.y(),rc.width(),rc.height()),
        it_adso = stroke->painterlyChannel("KisAdsorbencyMask")->createRectIterator(rc.x(),rc.y(),rc.width(),rc.height()),
        it_pigm = stroke->painterlyChannel("KisPigmentConcentrationMask")->createRectIterator(rc.x(),rc.y(),rc.width(),rc.height()),
        it_refl = stroke->painterlyChannel("KisReflectivityMask")->createRectIterator(rc.x(),rc.y(),rc.width(),rc.height()),
        it_volu = stroke->painterlyChannel("KisVolumeMask")->createRectIterator(rc.x(),rc.y(),rc.width(),rc.height()),
        it_visc = stroke->painterlyChannel("KisViscosityMask")->createRectIterator(rc.x(),rc.y(),rc.width(),rc.height()),
        it_wetn = stroke->painterlyChannel("KisWetnessMask")->createRectIterator(rc.x(),rc.y(),rc.width(),rc.height());
    KisRectIteratorPixel *iters[6] = // Used only to cleanly increase the iterators in the next cycle
        {&it_adso, &it_pigm, &it_refl, &it_volu, &it_visc, &it_wetn};

    while (!it_main.isDone()) {
        stroke->colorSpace()->toQColor(it_main.rawData(), &c, &opacity); // Is this too expensive?

        // Do something if there is actually some paint
        if (opacity > 0.0) {
            x = it_main.x();
            y = it_main.y();
            rho2 = RHO2(x-x0,y-y0);
            p = p0*exp(-rho2);
            // TODO Make the friction coefficient user configurable! (And find a good and complete meaning for it)
            f = p + 0.2f*pow(p,2);
            wetness = p*m_info.Wetness; // Wetness doesn't depend on friction.

            *it_wetn.rawData() = (quint8)(wetness*256.0f);
            *it_volu.rawData() = (quint8)(f*m_info.PaintVolume);
            *it_visc.rawData() = (quint8)(f*m_info.Viscosity/wetness*256.0f);
            *it_refl.rawData() = (quint8)(wetness*m_info.Reflectivity*256.0f);
            *it_pigm.rawData() = (quint8)(f*m_info.PigmentConcentration*((float)opacity));
        }

        for (int _i=0;_i<6;_i++) ++(*iters[_i]);
        ++it_main;
    }
}

void MixerTool::mixColors(KisPaintDeviceSP stroke, KoPointerEvent *e)
{

}

void MixerTool::updateResources(KisPaintDeviceSP stroke)
{
    // TODO Update the color and the tool's own KisPainterlyInformation structure.
}


#include "mixercore.moc"
