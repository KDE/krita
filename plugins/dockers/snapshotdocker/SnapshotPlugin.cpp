/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2010 Matus Talcik <matus.talcik@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */
#include "SnapshotPlugin.h"


#include <kpluginfactory.h>
#include <klocalizedstring.h>

#include <KoDockFactoryBase.h>
#include <KoDockRegistry.h>

#include "SnapshotDocker.h"

K_PLUGIN_FACTORY_WITH_JSON(SnapshotPluginFactory, "kritasnapshotdocker.json", registerPlugin<SnapshotPlugin>();)

class SnapshotDockFactory : public KoDockFactoryBase
{
public:
    SnapshotDockFactory() {
    }

    QString id() const override {
        return QString("Snapshot");
    }

    virtual Qt::DockWidgetArea defaultDockWidgetArea() const {
        return DockMinimized;
    }

    QDockWidget *createDockWidget() override {
        SnapshotDocker *dockWidget = new SnapshotDocker();
        dockWidget->setObjectName(id());

        return dockWidget;
    }

    DockPosition defaultDockPosition() const override {
        return DockRight;
    }
};


SnapshotPlugin::SnapshotPlugin(QObject *parent, const QVariantList &)
        : QObject(parent)
{

    KoDockRegistry::instance()->add(new SnapshotDockFactory());
}

SnapshotPlugin::~SnapshotPlugin()
{
}

#include "SnapshotPlugin.moc"
