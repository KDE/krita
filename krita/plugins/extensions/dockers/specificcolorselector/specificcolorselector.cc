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

#include "specificcolorselector.h"
#include <stdlib.h>

#include <QTimer>

#include <kactioncollection.h>
#include <kcomponentdata.h>
#include <kis_debug.h>
#include <kpluginfactory.h>
#include <klocale.h>
#include <kstandarddirs.h>

#include <KoDockFactoryBase.h>
#include <KoDockRegistry.h>

#include "kis_config.h"
#include "kis_cursor.h"
#include "kis_global.h"
#include "kis_types.h"
#include "kis_view2.h"

#include "specificcolorselector_dock.h"

K_PLUGIN_FACTORY(SpecificColorSelectorPluginFactory, registerPlugin<SpecificColorSelectorPlugin>();)
K_EXPORT_PLUGIN(SpecificColorSelectorPluginFactory("krita"))

class SpecificColorSelectorDockFactory : public KoDockFactoryBase
{
public:
    SpecificColorSelectorDockFactory() {
    }

    virtual QString id() const {
        return QString("SpecificColorSelector");
    }

    virtual Qt::DockWidgetArea defaultDockWidgetArea() const {
        return Qt::RightDockWidgetArea;
    }

    virtual QDockWidget* createDockWidget() {
        SpecificColorSelectorDock * dockWidget = new SpecificColorSelectorDock();

        dockWidget->setObjectName(id());

        return dockWidget;
    }

    KoDockFactoryBase::DockPosition defaultDockPosition() const {
        return DockMinimized;
    }
};


SpecificColorSelectorPlugin::SpecificColorSelectorPlugin(QObject *parent, const QVariantList &)
        : QObject(parent)
{
    dbgPlugins << "SpecificColorSelectorPlugin";

    //setComponentData(SpecificColorSelectorPluginFactory::componentData());
    KoDockRegistry::instance()->add(new SpecificColorSelectorDockFactory());
}

SpecificColorSelectorPlugin::~SpecificColorSelectorPlugin()
{
}

#include "specificcolorselector.moc"
