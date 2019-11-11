/*
 *  Copyright (c) 2009 Cyrille Berger <cberger@cberger.net>
 *
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2.1 of the License.
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
