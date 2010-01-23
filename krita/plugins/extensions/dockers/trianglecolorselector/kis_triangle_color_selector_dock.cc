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

#include "kis_triangle_color_selector_dock.h"

#include <klocale.h>

#include <KoColorSpaceRegistry.h>
#include <KoCanvasBase.h>
#include <KoResourceManager.h>
#include <KoTriangleColorSelector.h>
#include <KoColor.h>

KisTriangleColorSelectorDock::KisTriangleColorSelectorDock() : QDockWidget(i18n("Triangle Color Selector")), m_canvas(0)
{
    m_colorSelector = new KoTriangleColorSelector(this);
    setWidget(m_colorSelector);
    connect(m_colorSelector, SIGNAL(colorChanged(const QColor&)), this, SLOT(colorChangedProxy(const QColor&)));
}

void KisTriangleColorSelectorDock::setCanvas(KoCanvasBase * canvas)
{
    m_canvas = canvas;
    connect(m_canvas->resourceManager(), SIGNAL(resourceChanged(int, const QVariant&)),
            this, SLOT(resourceChanged(int, const QVariant&)));
    m_colorSelector->setQColor(m_canvas->resourceManager()->foregroundColor().toQColor());
}


void KisTriangleColorSelectorDock::colorChangedProxy(const QColor& c)
{
    if (m_canvas)
        m_canvas->resourceManager()->setForegroundColor(KoColor(c , KoColorSpaceRegistry::instance()->rgb8()));
}

void KisTriangleColorSelectorDock::resourceChanged(int key, const QVariant& v)
{
    if (key == KoCanvasResource::ForegroundColor)
        m_colorSelector->setQColor(v.value<KoColor>().toQColor());
}


#include "kis_triangle_color_selector_dock.moc"
