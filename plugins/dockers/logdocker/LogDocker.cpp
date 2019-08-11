/*
 *  Copyright (c) 2018 Boudewijn Rempt <boud@valdyas.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#include "LogDocker.h"

#include <kpluginfactory.h>
#include <klocalizedstring.h>

#include <KoDockFactoryBase.h>
#include <KoDockRegistry.h>

#include "LogDockerDock.h"

K_PLUGIN_FACTORY_WITH_JSON(LogDockerPluginFactory,
                           "krita_logdocker.json",
                           registerPlugin<LogDockerPlugin>();)

class LogDockerDockFactory : public KoDockFactoryBase {
public:
    LogDockerDockFactory()
    {
    }

    virtual ~LogDockerDockFactory()
    {
    }

    QString id() const override
    {
        return QString( "LogDocker" );
    }

    virtual Qt::DockWidgetArea defaultDockWidgetArea() const
    {
        return Qt::RightDockWidgetArea;
    }

    QDockWidget* createDockWidget() override
    {
                                        LogDockerDock * dockWidget = new LogDockerDock();
        dockWidget->setObjectName(id());

        return dockWidget;
    }

    DockPosition defaultDockPosition() const override
    {
        return DockMinimized;
    }
private:


};


LogDockerPlugin::LogDockerPlugin(QObject *parent, const QVariantList &)
    : QObject(parent)
{
    KoDockRegistry::instance()->add(new LogDockerDockFactory());
}

LogDockerPlugin::~LogDockerPlugin()
{
}

#include "LogDocker.moc"
