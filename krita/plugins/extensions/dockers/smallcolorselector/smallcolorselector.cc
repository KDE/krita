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

#include "smallcolorselector.h"

#include <kcomponentdata.h>
#include <kgenericfactory.h>

#include <KoDockFactory.h>
#include <KoDockRegistry.h>
#include <KoCanvasResourceProvider.h>

#include "smallcolorselector_dock.h"

typedef KGenericFactory<SmallColorSelectorPlugin> SmallColorSelectorPluginFactory;
K_EXPORT_COMPONENT_FACTORY(kritasmallcolorselector, SmallColorSelectorPluginFactory("krita"))

class SmallColorSelectorDockFactory : public KoDockFactory
{
public:
    SmallColorSelectorDockFactory() {
    }

    virtual QString id() const {
        return QString("SmallColorSelector");
    }

    virtual Qt::DockWidgetArea defaultDockWidgetArea() const {
        return Qt::RightDockWidgetArea;
    }

    virtual QDockWidget* createDockWidget() {
        SmallColorSelectorDock * dockWidget = new SmallColorSelectorDock();
        dockWidget->setObjectName(id());

        return dockWidget;
    }

    DockPosition defaultDockPosition() const {
        return DockRight;
    }
};


SmallColorSelectorPlugin::SmallColorSelectorPlugin(QObject *parent, const QStringList &)
        : KParts::Plugin(parent)
{
    setComponentData(SmallColorSelectorPluginFactory::componentData());

    KoDockRegistry::instance()->add(new SmallColorSelectorDockFactory());
}

SmallColorSelectorPlugin::~SmallColorSelectorPlugin()
{
}

#include "smallcolorselector.moc"
