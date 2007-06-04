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

#include <QtGui>

#include <kdebug.h>

#include <KoCanvasBase.h>
#include <KoColorSpace.h>
#include <KoPointerEvent.h>
#include <KoShapeManager.h>
#include <KoToolProxy.h>
#include <KoUnit.h>
#include <KoViewConverter.h>

#include "kis_paint_device.h"

#include "mixercore.h"

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

void MixerCanvas::initDevice(KoColorSpace *cs)
{
    m_canvasDev = new KisPaintDevice(cs);

    m_tool = new MixerTool(this, m_canvasDev.data());
    m_toolProxy = new KoToolProxy(this);
    m_toolProxy->setActiveTool(m_tool);
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

void MixerCanvas::updateCanvas(const QRectF& rc)
{
    // TODO Implement updateCanvas
}


/////////////////
// THE MIXER TOOL
/////////////////

MixerTool::MixerTool(KoCanvasBase *canvas, KisPaintDevice *device)
    : KoTool(canvas), m_canvasDev(device)
{
}

MixerTool::~MixerTool()
{
}

void MixerTool::mousePressEvent(KoPointerEvent *event)
{
    kDebug() << "MOUSE PRESSED!! " << event->pos() << endl;
}

void MixerTool::mouseMoveEvent(KoPointerEvent *event)
{
    kDebug() << "MOUSE MOVED!! " << event->pos() << endl;
}

void MixerTool::mouseReleaseEvent(KoPointerEvent *event)
{
    kDebug() << "MOUSE RELEASED!! " << event->pos() << endl;
}

#include "mixercore.moc"
