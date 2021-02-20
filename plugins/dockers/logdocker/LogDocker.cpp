/*
 *  SPDX-FileCopyrightText: 2018 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
