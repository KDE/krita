/*
 *  Copyright (c) 2009 Cyrille Berger <cberger@cberger.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2 of the License.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "lutdocker.h"

#include <stdlib.h>

#include <QTimer>

#include <kactioncollection.h>
#include <kcomponentdata.h>
#include <kis_debug.h>
#include <kpluginfactory.h>
#include <klocale.h>
#include <kstandarddirs.h>

#include <KoDockFactoryBase.h>

#include "kis_config.h"
#include "kis_cursor.h"
#include "kis_global.h"
#include "kis_types.h"
#include "kis_config.h"

#include "lutdocker_dock.h"
#include <KoDockRegistry.h>

#include <OpenColorIO/OpenColorIO.h>
namespace OCIO = OCIO_NAMESPACE;


K_PLUGIN_FACTORY(LutDockerPluginFactory, registerPlugin<LutDockerPlugin>();)
K_EXPORT_PLUGIN(LutDockerPluginFactory( "krita" ) )

class LutDockerDockFactory : public KoDockFactoryBase {
public:
    LutDockerDockFactory(OCIO::ConstConfigRcPtr config)
        : m_config(config)
    {
    }

    virtual QString id() const
    {
        return QString( "LutDocker" );
    }

    virtual Qt::DockWidgetArea defaultDockWidgetArea() const
    {
        return Qt::RightDockWidgetArea;
    }

    virtual QDockWidget* createDockWidget()
    {
        LutDockerDock * dockWidget = new LutDockerDock(m_config);
        dockWidget->setObjectName(id());

        return dockWidget;
    }

    DockPosition defaultDockPosition() const
    {
        return DockMinimized;
    }
private:

    OCIO::ConstConfigRcPtr m_config;
};


LutDockerPlugin::LutDockerPlugin(QObject *parent, const QVariantList &)
    : QObject(parent)
{
    KisConfig cfg;
    try {
        OCIO::ConstConfigRcPtr config;
        if (cfg.useOcioEnvironmentVariable()) {
            dbgUI << "using OCIO from the environment";
            config = OCIO::Config::CreateFromEnv();
        }
        else {
            QString configFile = cfg.ocioConfigurationPath();
            dbgUI << "using OCIO config file" << configFile;
            config = OCIO::Config::CreateFromFile(configFile.toUtf8());
        }
        OCIO::SetCurrentConfig(config);
        KoDockRegistry::instance()->add(new LutDockerDockFactory(config));
    }
    catch (OCIO::Exception &exception) {
        KoDockRegistry::instance()->add(new LutDockerDockFactory(OCIO::GetCurrentConfig()));
    }

}

LutDockerPlugin::~LutDockerPlugin()
{
}

#include "lutdocker.moc"
