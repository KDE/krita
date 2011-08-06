/*
 *  Copyright (c) 2011 Silvio Heinrich <plassy@web.de>
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

#include "imagedocker.h"
#include "imagedocker_dock.h"

#include <kpluginfactory.h>
#include <klocale.h>
#include <KoDockFactoryBase.h>
#include <KoDockRegistry.h>

K_PLUGIN_FACTORY(ImageDockerPluginFactory, registerPlugin<ImageDockerPlugin>();)
K_EXPORT_PLUGIN(ImageDockerPluginFactory("krita"))

class ImageDockerDockFactory: public KoDockFactoryBase {
public:
    ImageDockerDockFactory() { }

    virtual QString id() const
    {
        return QString("ImageDocker");
    }

    virtual Qt::DockWidgetArea defaultDockWidgetArea() const
    {
        return Qt::RightDockWidgetArea;
    }

    virtual QDockWidget* createDockWidget()
    {
        ImageDockerDock* dockWidget = new ImageDockerDock();
        dockWidget->setObjectName(id());
        return dockWidget;
    }

    DockPosition defaultDockPosition() const
    {
        return DockMinimized;
    }
};

ImageDockerPlugin::ImageDockerPlugin(QObject* parent, const QVariantList&):
    QObject(parent)
{
    KoDockRegistry::instance()->add(new ImageDockerDockFactory());
}
