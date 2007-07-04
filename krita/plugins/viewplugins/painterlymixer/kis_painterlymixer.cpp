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

#include <KoCanvasResourceProvider.h>
#include <KoToolProxy.h>

#include "kis_canvas2.h"
#include "kis_resource_provider.h"
#include "kis_view2.h"

#include "kis_painterlymixer.h"
#include "mixertool.h"

KisPainterlyMixer::KisPainterlyMixer(QWidget *parent, KisView2 *view)
    : QWidget(parent), m_view(view), m_resources(view->canvasBase()->resourceProvider())
{
    setupUi(this);

    m_canvas->setDevice(m_view->image()->colorSpace());
    initTool();
    initSpots();
}

KisPainterlyMixer::~KisPainterlyMixer()
{
    if (m_tool)
        delete m_tool;
}

void KisPainterlyMixer::initTool()
{
    m_tool = new MixerTool(m_canvas, m_canvas->device(), m_resources);

    m_canvas->setToolProxy(new KoToolProxy(m_canvas));
    m_canvas->toolProxy()->setActiveTool(m_tool);
}

void KisPainterlyMixer::initSpots()
{

}


#include "kis_painterlymixer.moc"
