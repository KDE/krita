/*
 *  SPDX-FileCopyrightText: 2009 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "brushhuddocker.h"
#include "brushhud_dock.h"

#include <kpluginfactory.h>

#include <KoDockFactoryBase.h>
#include <KoDockRegistry.h>


K_PLUGIN_FACTORY_WITH_JSON(PresetDockerPluginFactory, "krita_brushhud.json", registerPlugin<BrushHudDockerPlugin>();)

class BrushHudDockerDockFactory : public KoDockFactoryBase {
public:
    BrushHudDockerDockFactory()
    {
    }

    QString id() const override
    {
        return QString( "BrushHudDocker" );
    }

    virtual Qt::DockWidgetArea defaultDockWidgetArea() const
    {
        return Qt::RightDockWidgetArea;
    }

    QDockWidget* createDockWidget() override
    {
        BrushHudDock * dockWidget = new BrushHudDock();
        dockWidget->setObjectName(id());

        return dockWidget;
    }

    DockPosition defaultDockPosition() const override
    {
        return DockMinimized;
    }
private:


};


BrushHudDockerPlugin::BrushHudDockerPlugin(QObject *parent, const QVariantList &)
    : QObject(parent)
{
    KoDockRegistry::instance()->add(new BrushHudDockerDockFactory());
}

BrushHudDockerPlugin::~BrushHudDockerPlugin()
{
}

#include "brushhuddocker.moc"
