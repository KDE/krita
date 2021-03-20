/*
 *  SPDX-FileCopyrightText: 2017 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "TouchDockerPlugin.h"

#include <kis_debug.h>
#include <kpluginfactory.h>
#include <klocalizedstring.h>
#include <KoDockFactoryBase.h>

#include "kis_config.h"
#include "kis_types.h"
#include "KisViewManager.h"

#include "TouchDockerDock.h"
#include <KoDockRegistry.h>

K_PLUGIN_FACTORY_WITH_JSON(TouchDockerPluginFactory, "krita_touchdocker.json", registerPlugin<TouchDockerPlugin>();)

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

    KDDockWidgets::DockWidgetBase *createDockWidget() override
    {
        TouchDockerDock * dockWidget = new TouchDockerDock();

        dockWidget->setObjectName(id());

        return dockWidget;
    }

    DockPosition defaultDockPosition() const override
    {
        return DockMinimized;
    }

};


TouchDockerPlugin::TouchDockerPlugin(QObject *parent, const QVariantList &)
    : QObject(parent)
{
    KoDockRegistry::instance()->add(new TouchDockerDockFactory());
}

TouchDockerPlugin::~TouchDockerPlugin()
{
}

#include "TouchDockerPlugin.moc"
