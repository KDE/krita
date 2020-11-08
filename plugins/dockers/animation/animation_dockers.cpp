/*
 *  Copyright (c) 2015 Jouni Pentikäinen <joupent@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "animation_dockers.h"
#include "timeline_docker.h"
#include "onion_skins_docker.h"
#include "kis_animation_curve_docker.h"

#include <kpluginfactory.h>

#include <KoDockFactoryBase.h>
#include <KoDockRegistry.h>

#include "KisViewManager.h"

K_PLUGIN_FACTORY_WITH_JSON(AnimationDockersPluginFactory, "krita_animationdocker.json", registerPlugin<AnimationDockersPlugin>();)

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
    KoDockRegistry::instance()->add(new TimelineDockerFactory());
    KoDockRegistry::instance()->add(new OnionSkinsDockerFactory());
    KoDockRegistry::instance()->add(new AnimationCurvesDockerFactory());
}

AnimationDockersPlugin::~AnimationDockersPlugin()
{
    m_view = 0;
}

#include "animation_dockers.moc"
