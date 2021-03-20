/*
 *  SPDX-FileCopyrightText: 2015 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "presethistory.h"

#include <kpluginfactory.h>

#include <KoDockFactoryBase.h>
#include <KoDockRegistry.h>

#include "presethistory_dock.h"

K_PLUGIN_FACTORY_WITH_JSON(PresetHistoryPluginFactory, "krita_presethistory.json", registerPlugin<PresetHistoryPlugin>();)

class PresetHistoryDockFactory : public KoDockFactoryBase
{
public:
    PresetHistoryDockFactory()
    {
    }

    QString id() const override
    {
        return QString( "PresetHistory" );
    }

    virtual Qt::DockWidgetArea defaultDockWidgetArea() const
    {
        return Qt::RightDockWidgetArea;
    }

    QDockWidget* createDockWidget() override
    {
        PresetHistoryDock * dockWidget = new PresetHistoryDock();
        dockWidget->setObjectName(id());
        return dockWidget;
    }

    DockPosition defaultDockPosition() const override
    {
        return DockMinimized;
    }
};


PresetHistoryPlugin::PresetHistoryPlugin(QObject *parent, const QVariantList &)
    : QObject(parent)
{
    KoDockRegistry::instance()->add(new PresetHistoryDockFactory());
}

PresetHistoryPlugin::~PresetHistoryPlugin()
{
}

#include "presethistory.moc"
