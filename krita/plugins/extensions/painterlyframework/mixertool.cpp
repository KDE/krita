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

#include <kis_debug.h>
#include <kis_painter.h>
#include <kis_paintop_registry.h>
#include <kis_canvas_resource_provider.h>

#include "mixercanvas.h"

MixerTool::MixerTool(MixerCanvas* mixer)
    : KoTool(mixer)
    , m_mixer(mixer)
{
    activate();
}

MixerTool::~MixerTool()
{

}

void MixerTool::activate(bool temporary)
{
}

void MixerTool::deactivate()
{
}

void MixerTool::resourceChanged(int key, const QVariant & res)
{
}

void MixerTool::paint(QPainter &painter, const KoViewConverter &converter)
{
}

void MixerTool::mousePressEvent(KoPointerEvent *event)
{
}

void MixerTool::mouseMoveEvent(KoPointerEvent *event)
{
}

void MixerTool::mouseReleaseEvent(KoPointerEvent *event)
{
}

void MixerTool::setDirty(const QRegion& region)
{
    m_mixer->updateCanvas(region);
}

#include "mixertool.moc"
