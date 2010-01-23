/* This file is part of the KDE project

   Copyright (C) 2007 Emanuele Tamponi <emanuele@valinor.it>

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
   Boston, MA 02110-1301, USA.
*/

#include "kis_painterlymixerdocker.h"

#include <KLocale>

#include <KoCanvasBase.h>
#include <KoResourceManager.h>
#include <KoColor.h>

#include "kis_painterlymixer.h"

KisPainterlyMixerDocker::KisPainterlyMixerDocker()
        : QDockWidget()
        , KoCanvasObserverBase()
        , m_currentCanvas(0)
{
    setWindowTitle(i18n("Painterly Color Mixer"));

    m_painterlyMixer = new KisPainterlyMixer(this);
    setWidget(m_painterlyMixer);

    connect(m_painterlyMixer, SIGNAL(colorChanged(KoColor)), this, SLOT(colorChanged(KoColor)));
}

KisPainterlyMixerDocker::~KisPainterlyMixerDocker()
{
}

QString KisPainterlyMixerDockerFactory::id() const
{
    return QString("KisPainterlyMixerDocker");
}

void KisPainterlyMixerDocker::setCanvas(KoCanvasBase* canvas)
{
    m_currentCanvas = canvas;
    connect(m_currentCanvas->resourceManager(), SIGNAL(resourceChanged(int,QVariant)), this, SLOT(resourceChanged(int,QVariant)));
}

void KisPainterlyMixerDocker::colorChanged(const KoColor& color)
{
    m_currentCanvas->resourceManager()->setForegroundColor(color);
}

void KisPainterlyMixerDocker::resourceChanged(int key, const QVariant& value)
{
    if (key == KoCanvasResource::ForegroundColor) {
        m_painterlyMixer->setColor(value.value<KoColor>());
    }
}


#include "kis_painterlymixerdocker.moc"
