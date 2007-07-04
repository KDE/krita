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

#include <QCursor>
#include <QFrame>

#include <kdebug.h>

#include <KoCanvasBase.h>
#include <KoCanvasResourceProvider.h>
#include <KoColor.h>
#include <KoColorConversions.h>
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
#include "kis_mixability_mask.h"
#include "kis_pigment_concentration_mask.h"
#include "kis_reflectivity_mask.h"
#include "kis_volume_mask.h"
#include "kis_viscosity_mask.h"
#include "kis_wetness_mask.h"

#include "mixercanvas.h"
#include "kis_painterly_information.h"

/*
Just add all the painterly overlays that we need in only one call
*/
void addPainterlyOverlays(KisPaintDevice* dev)
{
    dev->addPainterlyChannel(new KisAdsorbencyMask(dev));
    dev->addPainterlyChannel(new KisMixabilityMask(dev));
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
    addPainterlyOverlays(m_canvasDev.data());

    m_tool = new MixerTool(this, m_canvasDev.data(), rp);
    m_toolProxy = new KoToolProxy(this);
    m_toolProxy->setActiveTool(m_tool);
}

void MixerCanvas::initSpots(QFrame *sf)
{
    // TODO Initialize spots
}

void MixerCanvas::mouseDoubleClickEvent(QMouseEvent *event)
{
//     m_toolProxy->mouseDoubleClickEvent(event, event->pos());
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


#include "mixercore.moc"
