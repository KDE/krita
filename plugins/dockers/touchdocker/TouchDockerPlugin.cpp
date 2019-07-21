/*
 *  Copyright (c) 2017 Boudewijn Rempt <boud@valdyas.org>
 *
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2 of the License, or
 *  (at your option) any later version.
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

#include "TouchDockerPlugin.h"

#include <kis_debug.h>
#include <kpluginfactory.h>
#include <klocalizedstring.h>
#include <KoDockFactoryBase.h>

#include "kis_config.h"
#include "kis_types.h"
#include "KisViewManager.h"

#include "TouchDockerDock.h"
#include <KoDockRegistry.h>

K_PLUGIN_FACTORY_WITH_JSON(TouchDockerPluginFactory, "krita_touchdocker.json", registerPlugin<TouchDockerPlugin>();)

class TouchDockerDockFactory : public KoDockFactoryBase {
public:
    TouchDockerDockFactory()
    {
    }

    QString id() const override
    {
        return QString( "TouchDocker" );
    }

    virtual Qt::DockWidgetArea defaultDockWidgetArea() const
    {
        return Qt::RightDockWidgetArea;
    }

    QDockWidget* createDockWidget() override
    {
        TouchDockerDock * dockWidget = new TouchDockerDock();

        dockWidget->setObjectName(id());

        return dockWidget;
    }

    DockPosition defaultDockPosition() const override
    {
        return DockMinimized;
    }

};


TouchDockerPlugin::TouchDockerPlugin(QObject *parent, const QVariantList &)
    : QObject(parent)
{
    KoDockRegistry::instance()->add(new TouchDockerDockFactory());
}

TouchDockerPlugin::~TouchDockerPlugin()
{
}

#include "TouchDockerPlugin.moc"
