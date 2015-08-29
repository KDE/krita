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

#include "tool_grid.h"
#include "kis_tool_grid.h"

#include <stdlib.h>
#include <vector>

#include <QPoint>

#include <klocale.h>

#include <kis_debug.h>
#include <kpluginfactory.h>

#include <kis_global.h>
#include <kis_types.h>
#include <KoToolRegistry.h>

K_PLUGIN_FACTORY_WITH_JSON(GridFactory, "kritatoolgrid.json", registerPlugin<GridPlugin>();)


GridPlugin::GridPlugin(QObject *parent, const QVariantList &)
        : QObject(parent)
{
    KoToolRegistry * r = KoToolRegistry::instance();
    r->add(new KisToolGridFactory(QStringList()));
}

GridPlugin::~GridPlugin()
{
}

#include "tool_grid.moc"
