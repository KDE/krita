/*
 *  SPDX-FileCopyrightText: 2016 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "griddocker.h"
#include "griddocker_dock.h"

#include <kpluginfactory.h>

#include <KoDockFactoryBase.h>
#include <KoDockRegistry.h>

K_PLUGIN_FACTORY_WITH_JSON(GridDockerPluginFactory, "krita_griddocker.json", registerPlugin<GridDockerPlugin>();)

class GridDockerDockFactory : public KoDockFactoryBase {
public:
    GridDockerDockFactory()
    {
    }

    QString id() const override
    {
        return QString( "GridDocker" );
    }

    virtual Qt::DockWidgetArea defaultDockWidgetArea() const
    {
        return Qt::RightDockWidgetArea;
    }

    QDockWidget* createDockWidget() override
    {
        GridDockerDock * dockWidget = new GridDockerDock();
        dockWidget->setObjectName(id());

        return dockWidget;
    }

    DockPosition defaultDockPosition() const override
    {
        return DockMinimized;
    }
private:


};


GridDockerPlugin::GridDockerPlugin(QObject *parent, const QVariantList &)
    : QObject(parent)
{
    KoDockRegistry::instance()->add(new GridDockerDockFactory());
}

GridDockerPlugin::~GridDockerPlugin()
{
}

#include "griddocker.moc"
