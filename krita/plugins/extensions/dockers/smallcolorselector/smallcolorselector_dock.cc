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

#include "smallcolorselector_dock.h"

#include <klocale.h>
#include <KoCanvasBase.h>

#include "kis_small_color_widget.h"
#include "kis_canvas_resource_provider.h"

#include <KoColorSpaceRegistry.h>

SmallColorSelectorDock::SmallColorSelectorDock()
        : QDockWidget()
        , m_canvas(0)
{
    m_smallColorWidget = new KisSmallColorWidget(this);
    setWidget(m_smallColorWidget);
    m_smallColorWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
    connect(m_smallColorWidget, SIGNAL(colorChanged(const QColor&)),
            this, SLOT(colorChangedProxy(const QColor&)));

    setWindowTitle(i18n("Small Color Selector"));
}

void SmallColorSelectorDock::setCanvas(KoCanvasBase * canvas)
{
    m_canvas = canvas;
    connect(m_canvas->resourceProvider(), SIGNAL(resourceChanged(int, const QVariant&)),
            this, SLOT(resourceChanged(int, const QVariant&)));
    m_smallColorWidget->setQColor(m_canvas->resourceProvider()->foregroundColor().toQColor());
}

void SmallColorSelectorDock::colorChangedProxy(const QColor& c)
{
    if (m_canvas)
        m_canvas->resourceProvider()->setForegroundColor(KoColor(c , KoColorSpaceRegistry::instance()->rgb8()));
}

void SmallColorSelectorDock::resourceChanged(int key, const QVariant& v)
{
    if (key == KoCanvasResource::ForegroundColor) {
        m_smallColorWidget->setQColor(v.value<KoColor>().toQColor());
    }
}

#include "smallcolorselector_dock.moc"
