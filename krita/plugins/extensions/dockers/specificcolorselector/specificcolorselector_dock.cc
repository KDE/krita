/*
 *  Copyright (c) 2008 Cyrille Berger <cberger@cberger.net>
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

#include "specificcolorselector_dock.h"

#include <klocale.h>

#include <kis_layer.h>
#include <kis_view2.h>
#include <canvas/kis_canvas2.h>
#include <kis_canvas_resource_provider.h>
#include <kis_image.h>

#include "kis_specific_color_selector_widget.h"

SpecificColorSelectorDock::SpecificColorSelectorDock()
    : QDockWidget(i18n("Specific Color Selector"))
    , m_canvas(0)
    , m_view(0)
    , m_colorSelector(0)
{
}

void SpecificColorSelectorDock::setCanvas(KoCanvasBase * canvas)
{
    if (m_canvas) {
        m_canvas->disconnectCanvasObserver(this);
    }
    if (m_view) {
        m_view->resourceProvider()->disconnect(m_colorSelector);
        m_view->resourceProvider()->disconnect(this);
        m_view->image()->disconnect(m_colorSelector);
    }

    KisCanvas2* kisCanvas = dynamic_cast<KisCanvas2*>(canvas);
    Q_ASSERT(canvas);
    KisView2* view = kisCanvas->view();

    if (m_colorSelector) {
        m_colorSelector->disconnect(); // explicit disconnect in case Qt gets confused.
        delete m_colorSelector;
    }
    m_colorSelector = new KisSpecificColorSelectorWidget(this);
    setWidget(m_colorSelector);

    connect(m_colorSelector, SIGNAL(colorChanged(const KoColor&)), view->resourceProvider(), SLOT(slotSetFGColor(const KoColor&)));
    connect(view->resourceProvider(), SIGNAL(sigFGColorChanged(const KoColor&)), m_colorSelector, SLOT(setColor(const KoColor&)));

    connect(view->resourceProvider(), SIGNAL(sigNodeChanged(const KisNodeSP)), this, SLOT(layerChanged(const KisNodeSP)));
    connect(view->image(), SIGNAL(sigColorSpaceChanged(const KoColorSpace*)), m_colorSelector, SLOT(setColorSpace(const KoColorSpace*)));

    m_canvas = kisCanvas;
    m_view = view;
}

void SpecificColorSelectorDock::unsetCanvas()
{
    m_canvas = 0;
    m_view = 0;

    delete m_colorSelector;
    m_colorSelector = 0;
}

void SpecificColorSelectorDock::layerChanged(const KisNodeSP node)
{
    if (!node) return;
    if (!m_colorSelector) return;
    if (!m_colorSelector->customColorSpaceUsed()) {
        if (node->inherits("KisMask") && node->parent())
            m_colorSelector->setColorSpace(static_cast<const KisLayer*>(node->parent().data())->colorSpace());
        else
            m_colorSelector->setColorSpace(static_cast<const KisLayer*>(node.data())->colorSpace());
    }
    m_colorSelector->setColor(m_view->resourceProvider()->fgColor());
}


#include "specificcolorselector_dock.moc"
