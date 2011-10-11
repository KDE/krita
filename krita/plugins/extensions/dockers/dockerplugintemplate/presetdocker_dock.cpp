/*
 *  Copyright (c) 2009 Cyrille Berger <cberger@cberger.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2 of the License.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "presetdocker_dock.h"

#include <QGridLayout>

#include <klocale.h>

#include <KoCanvasResourceManager.h>
#include <KoCanvasBase.h>


PresetDockerDock::PresetDockerDock( ) : QDockWidget(i18n("Preset docker")), m_canvas(0)
{
    setWidget(new QWidget(this));
}

void PresetDockerDock::setCanvas(KoCanvasBase * canvas)
{
    // "Every connection you make emits a signal, so duplicate connections emit two signals"
    if (m_canvas) {
        m_canvas->disconnectCanvasObserver(this);
    }

    m_canvas = canvas;
    
    connect(m_canvas->resourceManager(), SIGNAL(resourceChanged(int, const QVariant&)),
            this, SLOT(resourceChanged(int, const QVariant&)));
}

void PresetDockerDock::resourceChanged(int key, const QVariant& v)
{
}

#include "presetdocker_dock.moc"
