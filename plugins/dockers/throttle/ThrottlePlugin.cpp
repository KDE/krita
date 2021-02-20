/*
 *  SPDX-FileCopyrightText: 2017 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: LGPL-2.1-only
 */

#include "ThrottlePlugin.h"

#include <QApplication>
#include <QDesktopWidget>

#include <kis_debug.h>
#include <kpluginfactory.h>
#include <klocalizedstring.h>
#include <kis_action.h>
#include <kis_config.h>
#include <kis_types.h>
#include <KisViewManager.h>
#include <KoDockFactoryBase.h>
#include <KoDockRegistry.h>


#include "Throttle.h"

K_PLUGIN_FACTORY_WITH_JSON(ThrottlePluginFactory, "krita_throttle.json", registerPlugin<ThrottlePlugin>();)

class ThrottleDockerDockFactory : public KoDockFactoryBase
{
public:
    ThrottleDockerDockFactory()
    {
    }

    QString id() const override
    {
        return QStringLiteral("ThrottleDocker");
    }

    virtual Qt::DockWidgetArea defaultDockWidgetArea() const
    {
        return Qt::RightDockWidgetArea;
    }

    QDockWidget* createDockWidget() override
    {
        QDockWidget *dockWidget = new BasicDocker();
        dockWidget->setObjectName(id());
        Throttle *throttle = new Throttle(dockWidget);
        dockWidget->setWidget(throttle);
        return dockWidget;
    }

    DockPosition defaultDockPosition() const override
    {
        return DockMinimized;
    }

};


ThrottlePlugin::ThrottlePlugin(QObject *parent, const QVariantList &)
    : QObject(parent)
{
    KoDockRegistry::instance()->add(new ThrottleDockerDockFactory());
}

ThrottlePlugin::~ThrottlePlugin()
{
}


#include "ThrottlePlugin.moc"
