/*
 *  SPDX-FileCopyrightText: 2016 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "arrangedocker.h"

#include <kis_debug.h>
#include <kpluginfactory.h>
#include <klocalizedstring.h>

#include <KoDockFactoryBase.h>
#include <KoDockRegistry.h>
#include "KisViewManager.h"

#include "arrangedocker_dock.h"

K_PLUGIN_FACTORY_WITH_JSON(ArrangeDockerPluginFactory, "krita_arrangedocker.json", registerPlugin<ArrangeDockerPlugin>();)

class ArrangeDockerDockFactory : public KoDockFactoryBase {
public:
    ArrangeDockerDockFactory()
    {
    }

    QString id() const override
    {
        return QString( "ArrangeDocker" );
    }

    virtual Qt::DockWidgetArea defaultDockWidgetArea() const
    {
        return Qt::RightDockWidgetArea;
    }

    KDDockWidgets::DockWidgetBase *createDockWidget() override
    {
        ArrangeDockerDock * dockWidget = new ArrangeDockerDock();
        dockWidget->setObjectName(id());

        return dockWidget;
    }

    DockPosition defaultDockPosition() const override
    {
        return DockMinimized;
    }
private:


};


ArrangeDockerPlugin::ArrangeDockerPlugin(QObject *parent, const QVariantList &)
    : QObject(parent)
{
    KoDockRegistry::instance()->add(new ArrangeDockerDockFactory());
}

ArrangeDockerPlugin::~ArrangeDockerPlugin()
{
    m_view = 0;
}

#include "arrangedocker.moc"
