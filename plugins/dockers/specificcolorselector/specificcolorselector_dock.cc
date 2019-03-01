/*
 *  Copyright (c) 2008 Cyrille Berger <cberger@cberger.net>
 *
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2.1 of the License.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "specificcolorselector_dock.h"

#include <klocalizedstring.h>
#include <QLayout>

#include <kis_layer.h>
#include <KisViewManager.h>
#include <canvas/kis_canvas2.h>
#include <kis_canvas_resource_provider.h>
#include <kis_image.h>
#include <kis_display_color_converter.h>

#include "kis_specific_color_selector_widget.h"

SpecificColorSelectorDock::SpecificColorSelectorDock()
    : QDockWidget(i18n("Specific Color Selector"))
    , m_canvas(0)
    , m_view(0)
    , m_colorSelector(new KisSpecificColorSelectorWidget(this))
{
    setWidget(m_colorSelector);
    widget()->setContentsMargins(4,4,4,0);
}

void SpecificColorSelectorDock::setCanvas(KoCanvasBase * canvas)
{
    setEnabled(canvas != 0);

    if (m_canvas) {
        m_canvas->disconnectCanvasObserver(this);
    }

    KisCanvas2* kisCanvas = dynamic_cast<KisCanvas2*>(canvas);
    m_canvas = kisCanvas;

    if (!kisCanvas) {
        return;
    }

    m_colorSelector->setDisplayConverter(kisCanvas->displayColorConverter());
}

void SpecificColorSelectorDock::unsetCanvas()
{
    setEnabled(false);
    m_canvas = 0;
    m_colorSelector->setDisplayConverter(0);
}

void SpecificColorSelectorDock::setViewManager(KisViewManager* kisview)
{
    m_view = kisview;
    connect(m_view->canvasResourceProvider(), SIGNAL(sigFGColorChanged(KoColor)), m_colorSelector, SLOT(setColor(KoColor)));
    connect(m_colorSelector, SIGNAL(colorChanged(KoColor)), m_view->canvasResourceProvider(), SLOT(slotSetFGColor(KoColor)));
}

#include "moc_specificcolorselector_dock.cpp"
