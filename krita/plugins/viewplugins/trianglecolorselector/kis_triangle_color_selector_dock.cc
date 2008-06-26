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
#include <KoColorSpaceRegistry.h>

#include <kis_view2.h>

#include "kis_canvas_resource_provider.h"
#include <KoTriangleColorSelector.h>

KisTriangleColorSelectorDock::KisTriangleColorSelectorDock( KisView2 *view ) : QDockWidget(i18n("Triangle Color Selector")), m_view(view)
{
    m_colorSelector = new KoTriangleColorSelector(this);
    setWidget( m_colorSelector );
    connect(m_colorSelector, SIGNAL(colorChanged(const QColor&)), this, SLOT(colorChangedProxy(const QColor&)));
    connect( m_view->resourceProvider(), SIGNAL(sigFGColorChanged(const KoColor&)), this, SLOT(setColorProxy(const KoColor&)));
}

void KisTriangleColorSelectorDock::colorChangedProxy(const QColor& c)
{
    m_view->resourceProvider()->setFGColor( KoColor( c , KoColorSpaceRegistry::instance()->rgb8() ) );
}

void KisTriangleColorSelectorDock::setColorProxy( const KoColor& c )
{
    m_colorSelector->setQColor( c.toQColor() );
}


#include "kis_triangle_color_selector_dock.moc"
