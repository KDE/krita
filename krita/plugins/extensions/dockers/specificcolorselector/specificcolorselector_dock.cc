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

#include <klocale.h>
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
        m_canvas->image()->disconnect(m_colorSelector);
    }
    
    KisCanvas2* kisCanvas = dynamic_cast<KisCanvas2*>(canvas);
    m_canvas = kisCanvas;

    if (!kisCanvas) {
        return;
    }

    connect(kisCanvas->image(), SIGNAL(sigColorSpaceChanged(const KoColorSpace*)), m_colorSelector, SLOT(setColorSpace(const KoColorSpace*)));
    m_colorSelector->setDisplayRenderer(kisCanvas->displayColorConverter()->displayRendererInterface());

    if (m_view && m_view->activeNode()) {
        m_colorSelector->setColorSpace(m_view->activeNode()->colorSpace());
    }

}

void SpecificColorSelectorDock::unsetCanvas()
{
    setEnabled(false);

    m_canvas->image()->disconnect(m_colorSelector);
    m_canvas = 0;

    m_colorSelector->setDisplayRenderer(0);
}

void SpecificColorSelectorDock::setMainWindow(KisViewManager* kisview)
{
    m_view = kisview;
    connect(m_view->resourceProvider(), SIGNAL(sigFGColorChanged(const KoColor&)), m_colorSelector, SLOT(setColor(const KoColor&)));
    connect(m_colorSelector, SIGNAL(colorChanged(const KoColor&)), m_view->resourceProvider(), SLOT(slotSetFGColor(const KoColor&)));
    connect(m_view->resourceProvider(), SIGNAL(sigNodeChanged(const KisNodeSP)), this, SLOT(layerChanged(const KisNodeSP)));
}

void SpecificColorSelectorDock::layerChanged(const KisNodeSP node)
{
    if (!node) return;
    if (!node->paintDevice()) return;
    if (!m_colorSelector) return;
    if (!m_colorSelector->customColorSpaceUsed()) {
        const KoColorSpace *cs = node->paintDevice() ?
            node->paintDevice()->compositionSourceColorSpace() :
            node->colorSpace();

        m_colorSelector->setColorSpace(cs);
    }
    m_colorSelector->setColor(m_view->resourceProvider()->fgColor());
}


#include "specificcolorselector_dock.moc"
