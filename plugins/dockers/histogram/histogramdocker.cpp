/*
 *  SPDX-FileCopyrightText: 2016 Eugene Ingerman <geneing at gmail dot com>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "histogramdocker.h"

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

#include "histogramdocker_dock.h"
#include <KoDockRegistry.h>

K_PLUGIN_FACTORY_WITH_JSON(HistogramDockerPluginFactory, "krita_histogramdocker.json", registerPlugin<HistogramDockerPlugin>();)

class HistogramDockerDockFactory : public KoDockFactoryBase {
public:
    HistogramDockerDockFactory()
    {
    }

    QString id() const override
    {
        return QString( "HistogramDocker" );
    }

    virtual Qt::DockWidgetArea defaultDockWidgetArea() const
    {
        return Qt::RightDockWidgetArea;
    }

    QDockWidget* createDockWidget() override
    {
        HistogramDockerDock * dockWidget = new HistogramDockerDock();
        dockWidget->setObjectName(id());

        return dockWidget;
    }

    DockPosition defaultDockPosition() const override
    {
        return DockRight;
    }
private:


};


HistogramDockerPlugin::HistogramDockerPlugin(QObject *parent, const QVariantList &)
    : QObject(parent)
{
    KoDockRegistry::instance()->add(new HistogramDockerDockFactory());
}

HistogramDockerPlugin::~HistogramDockerPlugin()
{
    m_view = 0;
}

#include "histogramdocker.moc"
