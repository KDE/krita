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
#include <KoCanvasBase.h>
#include "kis_canvas2.h"

#include "kis_color_selector_ng_docker_widget.h"
#include "kis_canvas_resource_provider.h"

#include <KoColorSpaceRegistry.h>

KisColorSelectorNgDock::KisColorSelectorNgDock()
        : QDockWidget()
        , m_canvas(0)
{
    m_colorSelectorNgWidget = new KisColorSelectorNgDockerWidget(this);

    setWidget(m_colorSelectorNgWidget);
    m_colorSelectorNgWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
//    connect(m_colorSelectorNgWidget, SIGNAL(colorChanged(const QColor&)),
//            this, SLOT(colorChangedProxy(const QColor&)));

    setWindowTitle(i18n("Advanced Color Selector"));
}

void KisColorSelectorNgDock::setCanvas(KoCanvasBase * canvas)
{
    m_colorSelectorNgWidget->setCanvas(dynamic_cast<KisCanvas2*>(canvas));
//    m_canvas = canvas;
//    connect(m_canvas->resourceManager(), SIGNAL(resourceChanged(int, const QVariant&)),
//            this, SLOT(resourceChanged(int, const QVariant&)));
//    m_colorSelectorNgWidget->setQColor(m_canvas->resourceManager()->foregroundColor().toQColor());
}

void KisColorSelectorNgDock::colorChangedProxy(const QColor& c)
{
    Q_UNUSED(c);
//    if (m_canvas)
//        m_canvas->resourceManager()->setForegroundColor(KoColor(c , KoColorSpaceRegistry::instance()->rgb8()));
}

void KisColorSelectorNgDock::resourceChanged(int key, const QVariant& v)
{
    Q_UNUSED(key);
    Q_UNUSED(v);
//    if (key == KoCanvasResource::ForegroundColor) {
//        m_colorSelectorNgWidget->setQColor(v.value<KoColor>().toQColor());
//    }
}

#include "kis_color_selector_ng_dock.moc"
