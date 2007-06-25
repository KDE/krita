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

#include "kis_canvas2.h"
#include "kis_resource_provider.h"
#include "kis_view2.h"

#include "kis_painterlymixer.h"

KisPainterlyMixer::KisPainterlyMixer(QWidget *parent, KisView2 *view)
    : QWidget(parent), m_view(view)
{
    setupUi(this);

    m_canvas->initDevice(m_view->image()->colorSpace(), m_view->canvasBase()->resourceProvider());
    m_canvas->initSpots(m_spotsFrame);
}

KisPainterlyMixer::~KisPainterlyMixer()
{
}


#include "kis_painterlymixer.moc"
