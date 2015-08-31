/*
 *  Copyright (c) 2015 Jouni Pentikäinen <joupent@gmail.com>
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

#include "animation_dockers.h"
#include "animation_docker.h"
#include "timeline_docker.h"

#include <QVariantList>
#include <kpluginfactory.h>
#include <KoDockFactoryBase.h>
#include "KisViewManager.h"
#include <KoDockRegistry.h>

K_PLUGIN_FACTORY(AnimationDockersPluginFactory, registerPlugin<AnimationDockersPlugin>();)
K_EXPORT_PLUGIN(AnimationDockersPluginFactory( "krita" ) )

class AnimationDockerFactory : public KoDockFactoryBase {
public:
    AnimationDockerFactory()
    {
    }

    virtual QString id() const
    {
        return QString( "AnimationDocker" );
    }

    virtual Qt::DockWidgetArea defaultDockWidgetArea() const
    {
        return Qt::RightDockWidgetArea;
    }

    virtual QDockWidget *createDockWidget()
    {
        AnimationDocker *dockWidget = new AnimationDocker();
        dockWidget->setObjectName(id());

        return dockWidget;
    }

    DockPosition defaultDockPosition() const
    {
        return DockMinimized;
    }
private:
};

class TimelineDockerFactory : public KoDockFactoryBase {
public:
    TimelineDockerFactory()
    {
    }

    virtual QString id() const
    {
        return QString( "TimelineDocker" );
    }

    virtual Qt::DockWidgetArea defaultDockWidgetArea() const
    {
        return Qt::RightDockWidgetArea;
    }

    virtual QDockWidget *createDockWidget()
    {
        TimelineDocker *dockWidget = new TimelineDocker();
        dockWidget->setObjectName(id());

        return dockWidget;
    }

    DockPosition defaultDockPosition() const
    {
        return DockMinimized;
    }
private:
};

AnimationDockersPlugin::AnimationDockersPlugin(QObject *parent, const QVariantList &)
    : QObject(parent)
{
    KoDockRegistry::instance()->add(new AnimationDockerFactory());
    KoDockRegistry::instance()->add(new TimelineDockerFactory());
}

AnimationDockersPlugin::~AnimationDockersPlugin()
{
    m_view = 0;
}

#include "animation_dockers.moc"
