/*
 *  Copyright (c) 2009 Cyrille Berger <cberger@cberger.net>
 *
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
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
