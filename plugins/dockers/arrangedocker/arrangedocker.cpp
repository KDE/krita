/*
 *  SPDX-FileCopyrightText: 2016 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "arrangedocker.h"
#include "arrangedocker_dock.h"

#include <kpluginfactory.h>

#include <KoDockFactoryBase.h>
#include <KoDockRegistry.h>

K_PLUGIN_FACTORY_WITH_JSON(ArrangeDockerPluginFactory, "krita_arrangedocker.json", registerPlugin<ArrangeDockerPlugin>();)

class ArrangeDockerDockFactory : public KoDockFactoryBase {
public:
    ArrangeDockerDockFactory()
    {
    }

    QString id() const override
    {
        return QString( "ArrangeDocker" );
    }

    virtual Qt::DockWidgetArea defaultDockWidgetArea() const
    {
        return Qt::RightDockWidgetArea;
    }

    QDockWidget* createDockWidget() override
    {
        ArrangeDockerDock * dockWidget = new ArrangeDockerDock();
        dockWidget->setObjectName(id());

        return dockWidget;
    }

    DockPosition defaultDockPosition() const override
    {
        return DockMinimized;
    }
private:


};


ArrangeDockerPlugin::ArrangeDockerPlugin(QObject *parent, const QVariantList &)
    : QObject(parent)
{
    KoDockRegistry::instance()->add(new ArrangeDockerDockFactory());
}

ArrangeDockerPlugin::~ArrangeDockerPlugin()
{
}

#include "arrangedocker.moc"
