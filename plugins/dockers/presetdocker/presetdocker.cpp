/*
 *  SPDX-FileCopyrightText: 2009 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "presetdocker.h"
#include "presetdocker_dock.h"

#include <kpluginfactory.h>

#include <KoDockFactoryBase.h>
#include <KoDockRegistry.h>

K_PLUGIN_FACTORY_WITH_JSON(PresetDockerPluginFactory, "krita_presetdocker.json", registerPlugin<PresetDockerPlugin>();)

class PresetDockerDockFactory : public KoDockFactoryBase {
public:
    PresetDockerDockFactory()
    {
    }

    QString id() const override
    {
        return QString( "PresetDocker" );
    }

    virtual Qt::DockWidgetArea defaultDockWidgetArea() const
    {
        return Qt::RightDockWidgetArea;
    }

    QDockWidget* createDockWidget() override
    {
        PresetDockerDock * dockWidget = new PresetDockerDock();
        dockWidget->setObjectName(id());

        return dockWidget;
    }

    DockPosition defaultDockPosition() const override
    {
        return DockMinimized;
    }
private:


};


PresetDockerPlugin::PresetDockerPlugin(QObject *parent, const QVariantList &)
    : QObject(parent)
{
    KoDockRegistry::instance()->add(new PresetDockerDockFactory());
}

PresetDockerPlugin::~PresetDockerPlugin()
{
}

#include "presetdocker.moc"
