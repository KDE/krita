/*
 *  SPDX-FileCopyrightText: 2020 Saurabh Kumar <saurabhk660@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "StoryboardDocker.h"

#include <kpluginfactory.h>
#include <klocalizedstring.h>

#include <KoDockFactoryBase.h>
#include <KoDockRegistry.h>

#include "StoryboardDockerDock.h"

K_PLUGIN_FACTORY_WITH_JSON(StoryboardDockerPluginFactory, "krita_storyboarddocker.json", registerPlugin<StoryboardDockerPlugin>();)

class StoryboardDockerDockFactory : public KoDockFactoryBase {
public:
    StoryboardDockerDockFactory()
    {
    }

    QString id() const override
    {
        return QString( "StoryboardDocker" );
    }

    virtual Qt::DockWidgetArea defaultDockWidgetArea() const
    {
        return Qt::RightDockWidgetArea;
    }

    QDockWidget* createDockWidget() override
    {
        StoryboardDockerDock * dockWidget = new StoryboardDockerDock();
        
        dockWidget->setObjectName(id());

        return dockWidget;
    }

    DockPosition defaultDockPosition() const override
    {
        return DockMinimized;
    }
private:


};


StoryboardDockerPlugin::StoryboardDockerPlugin(QObject *parent, const QVariantList &)
    : QObject(parent)
{
    KoDockRegistry::instance()->add(new StoryboardDockerDockFactory());
}

StoryboardDockerPlugin::~StoryboardDockerPlugin()
{
}

#include "StoryboardDocker.moc"
