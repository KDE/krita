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
#include "onion_skins_docker.h"
#include "kis_animation_curve_docker.h"

#include <kpluginfactory.h>

#include <KoDockFactoryBase.h>
#include <KoDockRegistry.h>

#include "KisViewManager.h"

K_PLUGIN_FACTORY_WITH_JSON(AnimationDockersPluginFactory, "krita_animationdocker.json", registerPlugin<AnimationDockersPlugin>();)

class AnimationDockerFactory : public KoDockFactoryBase {
public:
    AnimationDockerFactory()
    {
    }

    QString id() const override
    {
        return QString( "AnimationDocker" );
    }

    virtual Qt::DockWidgetArea defaultDockWidgetArea() const
    {
        return Qt::RightDockWidgetArea;
    }

    QDockWidget *createDockWidget() override
    {
        AnimationDocker *dockWidget = new AnimationDocker();
        dockWidget->setObjectName(id());

        return dockWidget;
    }

    DockPosition defaultDockPosition() const override
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

    QString id() const override
    {
        return QString( "TimelineDocker" );
    }

    virtual Qt::DockWidgetArea defaultDockWidgetArea() const
    {
        return Qt::RightDockWidgetArea;
    }

    QDockWidget *createDockWidget() override
    {
        TimelineDocker *dockWidget = new TimelineDocker();
        dockWidget->setObjectName(id());

        return dockWidget;
    }

    DockPosition defaultDockPosition() const override
    {
        return DockMinimized;
    }
private:
};

class OnionSkinsDockerFactory : public KoDockFactoryBase {
public:
    OnionSkinsDockerFactory()
    {
    }

    QString id() const override
    {
        return QString( "OnionSkinsDocker" );
    }

    virtual Qt::DockWidgetArea defaultDockWidgetArea() const
    {
        return Qt::RightDockWidgetArea;
    }

    QDockWidget *createDockWidget() override
    {
        OnionSkinsDocker *dockWidget = new OnionSkinsDocker();
        dockWidget->setObjectName(id());

        return dockWidget;
    }

    DockPosition defaultDockPosition() const override
    {
        return DockMinimized;
    }
private:
};

class AnimationCurvesDockerFactory : public KoDockFactoryBase {
public:
    AnimationCurvesDockerFactory()
    {
    }

    QString id() const override
    {
        return QString( "AnimationCurvesDocker" );
    }

    virtual Qt::DockWidgetArea defaultDockWidgetArea() const
    {
        return Qt::RightDockWidgetArea;
    }

    QDockWidget *createDockWidget() override
    {
        KisAnimationCurveDocker *dockWidget = new KisAnimationCurveDocker();
        dockWidget->setObjectName(id());

        return dockWidget;
    }

    DockPosition defaultDockPosition() const override
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
    KoDockRegistry::instance()->add(new OnionSkinsDockerFactory());
    KoDockRegistry::instance()->add(new AnimationCurvesDockerFactory());
}

AnimationDockersPlugin::~AnimationDockersPlugin()
{
    m_view = 0;
}

#include "animation_dockers.moc"
