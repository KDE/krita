/*
 *  SPDX-FileCopyrightText: 2016 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "griddocker.h"

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

#include "griddocker_dock.h"
#include <KoDockRegistry.h>

K_PLUGIN_FACTORY_WITH_JSON(GridDockerPluginFactory, "krita_griddocker.json", registerPlugin<GridDockerPlugin>();)

class GridDockerDockFactory : public KoDockFactoryBase {
public:
    GridDockerDockFactory()
    {
    }

    QString id() const override
    {
        return QString( "GridDocker" );
    }

    virtual Qt::DockWidgetArea defaultDockWidgetArea() const
    {
        return Qt::RightDockWidgetArea;
    }

    KDDockWidgets::DockWidgetBase *createDockWidget() override
    {
        GridDockerDock * dockWidget = new GridDockerDock();
        dockWidget->setObjectName(id());

        return dockWidget;
    }

    DockPosition defaultDockPosition() const override
    {
        return DockMinimized;
    }
private:


};


GridDockerPlugin::GridDockerPlugin(QObject *parent, const QVariantList &)
    : QObject(parent)
{
    KoDockRegistry::instance()->add(new GridDockerDockFactory());
}

GridDockerPlugin::~GridDockerPlugin()
{
    m_view = 0;
}

#include "griddocker.moc"
