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
#include <kis_view2.h>

#include "kis_small_color_widget.h"
#include "kis_canvas_resource_provider.h"

#include <KoColorSpaceRegistry.h>

SmallColorSelectorDock::SmallColorSelectorDock( KisView2 *view ) : QDockWidget(i18n("Small Color Selector")), m_view(view)
{
    m_smallColorWidget = new KisSmallColorWidget(this);
    setWidget( m_smallColorWidget );
    connect(m_smallColorWidget, SIGNAL(colorChanged(const QColor&)), this, SLOT(colorChangedProxy(const QColor&)));
    connect( m_view->resourceProvider(), SIGNAL(sigFGColorChanged(const KoColor&)), this, SLOT(setColorProxy(const KoColor&)));
}

void SmallColorSelectorDock::colorChangedProxy(const QColor& c)
{
    m_view->resourceProvider()->setFGColor( KoColor( c , KoColorSpaceRegistry::instance()->rgb8() ) );
}

void SmallColorSelectorDock::setColorProxy( const KoColor& c )
{
    m_smallColorWidget->setQColor( c.toQColor() );
}

#include "smallcolorselector_dock.moc"
