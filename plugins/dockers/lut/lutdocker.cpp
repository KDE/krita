/*
 *  SPDX-FileCopyrightText: 2009 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "lutdocker.h"

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

#include "lutdocker_dock.h"
#include <KoDockRegistry.h>

#include <OpenColorIO/OpenColorIO.h>
namespace OCIO = OCIO_NAMESPACE;


K_PLUGIN_FACTORY_WITH_JSON(LutDockerPluginFactory, "krita_lutdocker.json", registerPlugin<LutDockerPlugin>();)

class LutDockerDockFactory : public KoDockFactoryBase {
public:
    LutDockerDockFactory()
    {
    }

    QString id() const override
    {
        return QString( "LutDocker" );
    }

    virtual Qt::DockWidgetArea defaultDockWidgetArea() const
    {
        return Qt::RightDockWidgetArea;
    }

    QDockWidget* createDockWidget() override
    {
        LutDockerDock * dockWidget = new LutDockerDock();
        dockWidget->setObjectName(id());

        return dockWidget;
    }

    DockPosition defaultDockPosition() const override
    {
        return DockMinimized;
    }
private:

    OCIO::ConstConfigRcPtr m_config;
};


LutDockerPlugin::LutDockerPlugin(QObject *parent, const QVariantList &)
    : QObject(parent)
{
    KoDockRegistry::instance()->add(new LutDockerDockFactory());
}

LutDockerPlugin::~LutDockerPlugin()
{
}

#include "lutdocker.moc"
