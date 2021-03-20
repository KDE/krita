/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2010 Matus Talcik <matus.talcik@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */
#include "History.h"


#include <kpluginfactory.h>
#include <klocalizedstring.h>

#include <KoDockFactoryBase.h>
#include <KoDockRegistry.h>

#include "HistoryDock.h"

K_PLUGIN_FACTORY_WITH_JSON(HistoryPluginFactory, "kritahistorydocker.json", registerPlugin<HistoryPlugin>();)

class HistoryDockFactory : public KoDockFactoryBase
{
public:
    HistoryDockFactory() {
    }

    QString id() const override {
        return QString("History");
    }

    virtual Qt::DockWidgetArea defaultDockWidgetArea() const {
        return Qt::RightDockWidgetArea;
    }

    QDockWidget* createDockWidget() override {
        HistoryDock * dockWidget = new HistoryDock();
        dockWidget->setObjectName(id());

        return dockWidget;
    }

    DockPosition defaultDockPosition() const override {
        return DockRight;
    }
};


HistoryPlugin::HistoryPlugin(QObject *parent, const QVariantList &)
        : QObject(parent)
{

    KoDockRegistry::instance()->add(new HistoryDockFactory());
}

HistoryPlugin::~HistoryPlugin()
{
}

#include <History.moc>
