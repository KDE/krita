/*
 *  SPDX-FileCopyrightText: 2011 Sven Langkamp <sven.langkamp@gmail.com>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
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

    KDDockWidgets::DockWidgetBase *createDockWidget() override
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
