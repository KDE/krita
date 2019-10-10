/* This file is part of the KDE project
 * Copyright (C) 2010 Matus Talcik <matus.talcik@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
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
        return Qt::RightDockWidgetArea;
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
