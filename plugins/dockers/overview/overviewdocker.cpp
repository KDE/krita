/*
 *  SPDX-FileCopyrightText: 2009 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "overviewdocker.h"

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

#include "overviewdocker_dock.h"
#include <KoDockRegistry.h>

K_PLUGIN_FACTORY_WITH_JSON(OverviewDockerPluginFactory, "krita_overviewdocker.json", registerPlugin<OverviewDockerPlugin>();)

class OverviewDockerDockFactory : public KoDockFactoryBase {
public:
    OverviewDockerDockFactory()
    {
    }

    QString id() const override
    {
        return QString( "OverviewDocker" );
    }

    virtual Qt::DockWidgetArea defaultDockWidgetArea() const
    {
        return Qt::RightDockWidgetArea;
    }

    KDDockWidgets::DockWidgetBase *createDockWidget() override
    {
        OverviewDockerDock * dockWidget = new OverviewDockerDock();
        dockWidget->setObjectName(id());

        return dockWidget;
    }

    DockPosition defaultDockPosition() const override
    {
        return DockMinimized;
    }
private:


};


OverviewDockerPlugin::OverviewDockerPlugin(QObject *parent, const QVariantList &)
    : QObject(parent)
{
    KoDockRegistry::instance()->add(new OverviewDockerDockFactory());
}

OverviewDockerPlugin::~OverviewDockerPlugin()
{
    m_view = 0;
}

#include "overviewdocker.moc"
