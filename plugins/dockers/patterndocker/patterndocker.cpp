/*
 *  SPDX-FileCopyrightText: 2009 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "patterndocker.h"
#include "patterndocker_dock.h"

#include <kpluginfactory.h>

#include <KoDockFactoryBase.h>
#include <KoDockRegistry.h>

K_PLUGIN_FACTORY_WITH_JSON(PatternDockerPluginFactory, "krita_patterndocker.json", registerPlugin<PatternDockerPlugin>();)

class PatternDockerDockFactory : public KoDockFactoryBase {
public:
    PatternDockerDockFactory()
    {
    }

    QString id() const override
    {
        return QString( "PatternDocker" );
    }

    virtual Qt::DockWidgetArea defaultDockWidgetArea() const
    {
        return Qt::RightDockWidgetArea;
    }

    QDockWidget* createDockWidget() override
    {
        PatternDockerDock * dockWidget = new PatternDockerDock();
        dockWidget->setObjectName(id());

        return dockWidget;
    }

    DockPosition defaultDockPosition() const override
    {
        return DockMinimized;
    }
private:


};


PatternDockerPlugin::PatternDockerPlugin(QObject *parent, const QVariantList &)
    : QObject(parent)
{
    KoDockRegistry::instance()->add(new PatternDockerDockFactory());
}

PatternDockerPlugin::~PatternDockerPlugin()
{
}

#include "patterndocker.moc"
