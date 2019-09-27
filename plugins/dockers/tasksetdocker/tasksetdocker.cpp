/*
 *  Copyright (c) 2011 Sven Langkamp <sven.langkamp@gmail.com>
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

#include "tasksetdocker.h"

#include <kpluginfactory.h>
#include <klocalizedstring.h>

#include <KoDockFactoryBase.h>
#include <KoDockRegistry.h>

#include "tasksetdocker_dock.h"

K_PLUGIN_FACTORY_WITH_JSON(TasksetDockerPluginFactory,
                           "krita_tasksetdocker.json",
                           registerPlugin<TasksetDockerPlugin>();)

class TasksetDockerDockFactory : public KoDockFactoryBase {
public:
    TasksetDockerDockFactory()
    {
    }

    QString id() const override
    {
        return QString( "TasksetDocker" );
    }

    virtual Qt::DockWidgetArea defaultDockWidgetArea() const
    {
        return Qt::RightDockWidgetArea;
    }

    QDockWidget* createDockWidget() override
    {
        TasksetDockerDock * dockWidget = new TasksetDockerDock();

        dockWidget->setObjectName(id());

        return dockWidget;
    }

    DockPosition defaultDockPosition() const override
    {
        return DockMinimized;
    }
private:


};


TasksetDockerPlugin::TasksetDockerPlugin(QObject *parent, const QVariantList &)
    : QObject(parent)
{
    KoDockRegistry::instance()->add(new TasksetDockerDockFactory());
}

TasksetDockerPlugin::~TasksetDockerPlugin()
{
}

#include "tasksetdocker.moc"
