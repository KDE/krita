/*
 *  Copyright (c) 2020 Saurabh Kumar <saurabhk660@gmail.com>
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

#include "storyboarddocker.h"

#include <kpluginfactory.h>
#include <klocalizedstring.h>

#include <KoDockFactoryBase.h>
#include <KoDockRegistry.h>

#include "storyboarddocker_dock.h"

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

#include "storyboarddocker.moc"
