/*
 *  Copyright (c) 2015 Jouni Pentikäinen <mctyyppi42@gmail.com>
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

#include "animation_docker.h"
#include "animation_docker_dock.h"

#include <QVariantList>
#include <kpluginfactory.h>
#include <KoDockFactoryBase.h>
#include "KisViewManager.h"
#include <KoDockRegistry.h>

K_PLUGIN_FACTORY(AnimationDockerPluginFactory, registerPlugin<AnimationDockerPlugin>();)
K_EXPORT_PLUGIN(AnimationDockerPluginFactory( "krita" ) )

class AnimationDockerDockFactory : public KoDockFactoryBase {
public:
    AnimationDockerDockFactory()
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

    virtual QDockWidget * createDockWidget()
    {
        AnimationDockerDock *dockWidget = new AnimationDockerDock();
        dockWidget->setObjectName(id());

        return dockWidget;
    }

    DockPosition defaultDockPosition() const
    {
        return DockMinimized;
    }
private:


};

AnimationDockerPlugin::AnimationDockerPlugin(QObject *parent, const QVariantList &)
    : QObject(parent)
{
    KoDockRegistry::instance()->add(new AnimationDockerDockFactory());
}

AnimationDockerPlugin::~AnimationDockerPlugin()
{
    m_view = 0;
}

#include "animation_docker.moc"
