/*
 *  SPDX-FileCopyrightText: 2013 Sven Langkamp <sven.langkamp@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "palettedocker.h"


#include <kis_debug.h>
#include <kpluginfactory.h>
#include <klocalizedstring.h>

#include <KoDockFactoryBase.h>

#include "kis_config.h"
#include "kis_cursor.h"
#include "kis_global.h"
#include "kis_types.h"
#include "KisViewManager.h"

#include "palettedocker_dock.h"
#include <KoDockRegistry.h>

K_PLUGIN_FACTORY_WITH_JSON(PaletteDockerPluginFactory, "krita_palettedocker.json", registerPlugin<PaletteDockerPlugin>();)

class PaletteDockerDockFactory : public KoDockFactoryBase {
public:
    PaletteDockerDockFactory()
    {
    }

    QString id() const override
    {
        return QString( "PaletteDocker" );
    }

    virtual Qt::DockWidgetArea defaultDockWidgetArea() const
    {
        return Qt::RightDockWidgetArea;
    }

    QDockWidget* createDockWidget() override
    {
        PaletteDockerDock * dockWidget = new PaletteDockerDock();
        
        dockWidget->setObjectName(id());

        return dockWidget;
    }

    DockPosition defaultDockPosition() const override
    {
        return DockMinimized;
    }
private:


};


PaletteDockerPlugin::PaletteDockerPlugin(QObject *parent, const QVariantList &)
    : QObject(parent)
{
    KoDockRegistry::instance()->add(new PaletteDockerDockFactory());
}

PaletteDockerPlugin::~PaletteDockerPlugin()
{
}

#include "palettedocker.moc"
