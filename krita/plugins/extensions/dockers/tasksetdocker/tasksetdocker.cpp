/*
 *  Copyright (c) 2011 Sven Langkamp <sven.langkamp@gmail.com>
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

#include "tasksetdocker.h"

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
#include "kis_view2.h"

#include "tasksetdocker_dock.h"
#include <KoDockRegistry.h>

K_PLUGIN_FACTORY(TasksetDockerPluginFactory, registerPlugin<TasksetDockerPlugin>();)
K_EXPORT_PLUGIN(TasksetDockerPluginFactory( "krita" ) )

class TasksetDockerDockFactory : public KoDockFactoryBase {
public:
    TasksetDockerDockFactory()
    {
    }

    virtual QString id() const
    {
        return QString( "TasksetDocker" );
    }

    virtual Qt::DockWidgetArea defaultDockWidgetArea() const
    {
        return Qt::RightDockWidgetArea;
    }

    virtual QDockWidget* createDockWidget()
    {
        TasksetDockerDock * dockWidget = new TasksetDockerDock();
        
        dockWidget->setObjectName(id());

        return dockWidget;
    }

    DockPosition defaultDockPosition() const
    {
        return DockMinimized;
    }
private:


};


TasksetDockerPlugin::TasksetDockerPlugin(QObject *parent, const QVariantList &)
    : QObject(parent)
{
    KoDockRegistry::instance()->add(new TasksetDockerDockFactory());
}

TasksetDockerPlugin::~TasksetDockerPlugin()
{
}

#include "tasksetdocker.moc"
