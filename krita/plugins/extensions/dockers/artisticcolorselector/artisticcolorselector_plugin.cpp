/*
 *  Copyright (c) 2009 Cyrille Berger <cberger@cberger.net>
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

#include "artisticcolorselector_plugin.h"
#include "artisticcolorselector_dock.h"

#include <kpluginfactory.h>
#include <klocale.h>
#include <KoDockFactoryBase.h>
#include <KoDockRegistry.h>

K_PLUGIN_FACTORY(PaletteDockPluginFactory, registerPlugin<ArtisticColorSelectorPlugin>();)
K_EXPORT_PLUGIN(PaletteDockPluginFactory("krita"))

class ArtisticColorSelectorDockFactory: public KoDockFactoryBase
{
public:
    virtual QString id() const {
        return QString("ArtisticColorSelector");
    }

    virtual Qt::DockWidgetArea defaultDockWidgetArea() const {
        return Qt::RightDockWidgetArea;
    }

    virtual QDockWidget* createDockWidget() {
        ArtisticColorSelectorDock* dockWidget = new ArtisticColorSelectorDock();
        dockWidget->setObjectName(id());
        return dockWidget;
    }

    DockPosition defaultDockPosition() const {
        return DockMinimized;
    }
};


ArtisticColorSelectorPlugin::ArtisticColorSelectorPlugin(QObject* parent, const QVariantList &):
    QObject(parent)
{
    KoDockRegistry::instance()->add(new ArtisticColorSelectorDockFactory());
}
