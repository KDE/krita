/*
 *  SPDX-FileCopyrightText: 2024 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "TouchDocker.h"
#include "TouchDockerDock.h"

#include <kpluginfactory.h>

#include <KoDockFactoryBase.h>
#include <KoDockRegistry.h>

K_PLUGIN_FACTORY_WITH_JSON(TouchDockerPluginFactory, "kritatouchdocker.json", registerPlugin<TouchDockerPlugin>();)

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
private:


};


TouchDockerPlugin::TouchDockerPlugin(QObject *parent, const QVariantList &)
    : QObject(parent)
{
    KoDockRegistry::instance()->add(new TouchDockerDockFactory());
}

TouchDockerPlugin::~TouchDockerPlugin()
{
}

#include "TouchDocker.moc"
