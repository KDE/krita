/*
 *  SPDX-FileCopyrightText: 2009 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "presetdocker.h"

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

#include "presetdocker_dock.h"
#include <KoDockRegistry.h>

K_PLUGIN_FACTORY_WITH_JSON(PresetDockerPluginFactory, "krita_presetdocker.json", registerPlugin<PresetDockerPlugin>();)

class PresetDockerDockFactory : public KoDockFactoryBase {
public:
    PresetDockerDockFactory()
    {
    }

    QString id() const override
    {
        return QString( "PresetDocker" );
    }

    virtual Qt::DockWidgetArea defaultDockWidgetArea() const
    {
        return Qt::RightDockWidgetArea;
    }

    QDockWidget* createDockWidget() override
    {
        PresetDockerDock * dockWidget = new PresetDockerDock();
        dockWidget->setObjectName(id());

        return dockWidget;
    }

    DockPosition defaultDockPosition() const override
    {
        return DockMinimized;
    }
private:


};


PresetDockerPlugin::PresetDockerPlugin(QObject *parent, const QVariantList &)
    : QObject(parent)
{
    KoDockRegistry::instance()->add(new PresetDockerDockFactory());
}

PresetDockerPlugin::~PresetDockerPlugin()
{
    m_view = 0;
}

#include "presetdocker.moc"
