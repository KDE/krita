/*
 *  Copyright (c) 2008 Cyrille Berger <cberger@cberger.net>
 *  Copyright (c) 2010 Adam Celarek <kdedev at xibo dot at>
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

#include "kis_color_selector_ng_dock.h"

#include <klocale.h>
#include "kis_canvas2.h"

#include "kis_color_selector_ng_docker_widget.h"


KisColorSelectorNgDock::KisColorSelectorNgDock()
        : QDockWidget()
{
    m_colorSelectorNgWidget = new KisColorSelectorNgDockerWidget(this);

    setWidget(m_colorSelectorNgWidget);
    m_colorSelectorNgWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    setWindowTitle(i18n("Advanced Color Selector"));
}

void KisColorSelectorNgDock::setCanvas(KoCanvasBase * canvas)
{
    m_colorSelectorNgWidget->setCanvas(dynamic_cast<KisCanvas2*>(canvas));
}

void KisColorSelectorNgDock::unsetCanvas()
{
    m_colorSelectorNgWidget->unsetCanvas();
}

#include "kis_color_selector_ng_dock.moc"
