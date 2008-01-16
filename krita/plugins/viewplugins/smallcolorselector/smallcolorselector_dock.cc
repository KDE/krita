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

SmallColorSelectorDock::SmallColorSelectorDock( KisView2 *view ) : QDockWidget(i18n("Small Color Selector")), m_view(view)
{
    m_smallColorWidget = new KisSmallColorWidget(this);
    setWidget( m_smallColorWidget );
}

#include "smallcolorselector_dock.moc"
