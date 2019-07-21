/*
 *  Copyright (c) 2016 Dmitry Kazakov <dimula73@gmail.com>
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

    QDockWidget* createDockWidget() override
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
