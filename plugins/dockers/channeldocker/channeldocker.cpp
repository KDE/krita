/*
 *  SPDX-FileCopyrightText: 2011 Sven Langkamp <sven.langkamp@gmail.com>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "channeldocker.h"

#include <stdlib.h>

#include <QTimer>


#include <kis_debug.h>
#include <kpluginfactory.h>
#include <klocalizedstring.h>

#include <KoDockFactoryBase.h>

#include "kis_config.h"
#include "kis_cursor.h"
#include "kis_global.h"
#include "kis_types.h"
#include "KisViewManager.h"

#include "channeldocker_dock.h"
#include <KoDockRegistry.h>

K_PLUGIN_FACTORY_WITH_JSON(ChannelDockerPluginFactory, "krita_channeldocker.json", registerPlugin<ChannelDockerPlugin>();)

class ChannelDockerDockFactory : public KoDockFactoryBase {
public:
    ChannelDockerDockFactory()
    {
    }

    QString id() const override
    {
        return QString( "ChannelDocker" );
    }

    virtual Qt::DockWidgetArea defaultDockWidgetArea() const
    {
        return Qt::RightDockWidgetArea;
    }

    KDDockWidgets::DockWidgetBase *createDockWidget() override
    {
        ChannelDockerDock * dockWidget = new ChannelDockerDock();
        
        dockWidget->setObjectName(id());

        return dockWidget;
    }

    DockPosition defaultDockPosition() const override
    {
        return DockMinimized;
    }
private:


};


ChannelDockerPlugin::ChannelDockerPlugin(QObject *parent, const QVariantList &)
    : QObject(parent)
{
    KoDockRegistry::instance()->add(new ChannelDockerDockFactory());
}

ChannelDockerPlugin::~ChannelDockerPlugin()
{
}

#include "channeldocker.moc"
