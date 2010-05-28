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

#include "colorselectorng_dock.h"

#include <klocale.h>
#include <KoCanvasBase.h>

#include "colorselectorngwidget.h"
#include "kis_canvas_resource_provider.h"

#include <KoColorSpaceRegistry.h>

ColorSelectorNgDock::ColorSelectorNgDock()
        : QDockWidget()
        , m_canvas(0)
{
    m_colorSelectorNgWidget = new ColorSelectorNgWidget(this);
    setWidget(m_colorSelectorNgWidget);
    m_colorSelectorNgWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
//    connect(m_colorSelectorNgWidget, SIGNAL(colorChanged(const QColor&)),
//            this, SLOT(colorChangedProxy(const QColor&)));

    setWindowTitle(i18n("Color Selector Ng"));
}

void ColorSelectorNgDock::setCanvas(KoCanvasBase * canvas)
{
//    m_canvas = canvas;
//    connect(m_canvas->resourceManager(), SIGNAL(resourceChanged(int, const QVariant&)),
//            this, SLOT(resourceChanged(int, const QVariant&)));
//    m_colorSelectorNgWidget->setQColor(m_canvas->resourceManager()->foregroundColor().toQColor());
}

void ColorSelectorNgDock::colorChangedProxy(const QColor& c)
{
//    if (m_canvas)
//        m_canvas->resourceManager()->setForegroundColor(KoColor(c , KoColorSpaceRegistry::instance()->rgb8()));
}

void ColorSelectorNgDock::resourceChanged(int key, const QVariant& v)
{
//    if (key == KoCanvasResource::ForegroundColor) {
//        m_colorSelectorNgWidget->setQColor(v.value<KoColor>().toQColor());
//    }
}

#include "colorselectorng_dock.moc"
