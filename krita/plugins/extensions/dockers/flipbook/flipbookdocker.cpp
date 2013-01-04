/*
 *  Copyright (c) 2012 Boudewijn Rempt <boud@valdyas.org>
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

#include "flipbookdocker.h"

#include <stdlib.h>

#include <QTimer>

#include <kactioncollection.h>
#include <kcomponentdata.h>
#include <kis_debug.h>
#include <kpluginfactory.h>
#include <klocale.h>
#include <kstandarddirs.h>

#include <KoDockFactoryBase.h>

#include "kis_config.h"
#include "kis_cursor.h"
#include "kis_global.h"
#include "kis_types.h"
#include "kis_view2.h"

#include "flipbookdocker_dock.h"
#include <KoDockRegistry.h>

K_PLUGIN_FACTORY(FlipbookDockerPluginFactory, registerPlugin<FlipbookDockerPlugin>();)
K_EXPORT_PLUGIN(FlipbookDockerPluginFactory( "krita" ) )

class FlipbookDockerDockFactory : public KoDockFactoryBase {
public:
    FlipbookDockerDockFactory()
    {
    }

    virtual QString id() const
    {
        return QString( "FlipbookDocker" );
    }

    virtual Qt::DockWidgetArea defaultDockWidgetArea() const
    {
        return Qt::RightDockWidgetArea;
    }

    virtual QDockWidget* createDockWidget()
    {
        FlipbookDockerDock * dockWidget = new FlipbookDockerDock();
        
        dockWidget->setObjectName(id());

        return dockWidget;
    }

    DockPosition defaultDockPosition() const
    {
        return DockMinimized;
    }

};


FlipbookDockerPlugin::FlipbookDockerPlugin(QObject *parent, const QVariantList &)
    : QObject(parent)
{
    KoDockRegistry::instance()->add(new FlipbookDockerDockFactory());
}

FlipbookDockerPlugin::~FlipbookDockerPlugin()
{
}

#include "flipbookdocker.moc"
