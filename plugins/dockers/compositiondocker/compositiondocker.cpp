/*
 *  SPDX-FileCopyrightText: 2011 Sven Langkamp <sven.langkamp@gmail.com>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "compositiondocker.h"

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

#include "compositiondocker_dock.h"
#include <KoDockRegistry.h>

K_PLUGIN_FACTORY_WITH_JSON(CompositionDockerPluginFactory, "krita_compositiondocker.json", registerPlugin<CompositionDockerPlugin>();)

class CompositionDockerDockFactory : public KoDockFactoryBase {
public:
    CompositionDockerDockFactory()
    {
    }

    QString id() const override
    {
        return QString( "CompositionDocker" );
    }

    virtual Qt::DockWidgetArea defaultDockWidgetArea() const
    {
        return Qt::RightDockWidgetArea;
    }

    KDDockWidgets::DockWidgetBase *createDockWidget() override
    {
        CompositionDockerDock * dockWidget = new CompositionDockerDock();
        
        dockWidget->setObjectName(id());

        return dockWidget;
    }

    DockPosition defaultDockPosition() const override
    {
        return DockMinimized;
    }
private:


};


CompositionDockerPlugin::CompositionDockerPlugin(QObject *parent, const QVariantList &)
    : QObject(parent)
{
    KoDockRegistry::instance()->add(new CompositionDockerDockFactory());
}

CompositionDockerPlugin::~CompositionDockerPlugin()
{
}

#include "compositiondocker.moc"
