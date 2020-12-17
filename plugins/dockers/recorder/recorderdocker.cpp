/*
 *  SPDX-FileCopyrightText: 2009 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: LGPL-2.1-only
 */

#include "recorderdocker.h"

#include <stdlib.h>

#include <QTimer>

#include <kis_debug.h>
#include <klocalizedstring.h>
#include <kpluginfactory.h>

#include <KoDockFactoryBase.h>

#include "KisViewManager.h"
#include "kis_config.h"
#include "kis_cursor.h"
#include "kis_global.h"
#include "kis_types.h"

#include "recorderdocker_dock.h"
#include <KoDockRegistry.h>

K_PLUGIN_FACTORY_WITH_JSON(RecorderDockerPluginFactory, "krita_recorderdocker.json",
                           registerPlugin<RecorderDockerPlugin>();)

class RecorderDockerDockFactory : public KoDockFactoryBase
{
public:
    RecorderDockerDockFactory()
    {
    }

    QString id() const override
    {
        return QString("RecorderDocker");
    }

    virtual Qt::DockWidgetArea defaultDockWidgetArea() const
    {
        return Qt::RightDockWidgetArea;
    }

    QDockWidget* createDockWidget() override
    {
        RecorderDockerDock* dockWidget = new RecorderDockerDock();
        dockWidget->setObjectName(id());

        return dockWidget;
    }

    DockPosition defaultDockPosition() const override
    {
        return DockMinimized;
    }

private:
};

RecorderDockerPlugin::RecorderDockerPlugin(QObject* parent, const QVariantList&)
    : QObject(parent)
{
    KoDockRegistry::instance()->add(new RecorderDockerDockFactory());
}

RecorderDockerPlugin::~RecorderDockerPlugin()
{
    m_view = nullptr;
}

#include "recorderdocker.moc"
