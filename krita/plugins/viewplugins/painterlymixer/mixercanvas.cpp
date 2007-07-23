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
#include <QStringList>

#include <kdebug.h>

#include <KoCanvasBase.h>
#include <KoColor.h>
#include <KoColorSpace.h>
#include <KoColorProfile.h>
#include <KoID.h>
#include <KoPointerEvent.h>
#include <KoShapeManager.h>
#include <KoToolProxy.h>

#include "kis_paint_device.h"
#include "kis_painterly_overlay.h"

// #include "kis_illuminant_profile.h"
// #include "kis_ks_colorspace.h"

#include "colorspot.h"
#include "mixertool.h"

#include "mixercanvas.h"

MixerCanvas::MixerCanvas(QWidget *parent)
    : QFrame(parent), KoCanvasBase(0), m_toolProxy(0)
{

}

MixerCanvas::~MixerCanvas()
{
    if (m_toolProxy)
        delete m_toolProxy;
}

void MixerCanvas::setDevice(KoColorSpace *cs)
{
    m_device = new KisPaintDevice(cs);
	m_overlay = new KisPainterlyOverlay;
}

void MixerCanvas::mouseDoubleClickEvent(QMouseEvent */*event*/)
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
    p.drawImage(r, m_device->convertToQImage(0, r.x(), r.y(), r.width(), r.height()), r);
    p.end();
}

void MixerCanvas::updateCanvas(const QRectF& rc)
{
    update(rc.toRect());
}


#include "mixercanvas.moc"
