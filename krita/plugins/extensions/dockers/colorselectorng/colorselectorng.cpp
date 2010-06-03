/*
 *  Copyright (c) 2008 Cyrille Berger <cberger@cberger.net>
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

#include "colorselectorng.h"

#include <kcomponentdata.h>
#include <kpluginfactory.h>

#include <KoDockFactoryBase.h>
#include <KoDockRegistry.h>
#include <KoResourceManager.h>

#include "kis_colselng_dock.h"

K_PLUGIN_FACTORY(ColorSelectorNgPluginFactory, registerPlugin<ColorSelectorNgPlugin>();)
K_EXPORT_PLUGIN(ColorSelectorNgPluginFactory("krita"))

class ColorSelectorNgDockFactory : public KoDockFactoryBase
{
public:
    ColorSelectorNgDockFactory() {
    }

    virtual QString id() const {
        return QString("ColorSelectorNg");
    }

    virtual Qt::DockWidgetArea defaultDockWidgetArea() const {
        return Qt::RightDockWidgetArea;
    }

    virtual QDockWidget* createDockWidget() {
        KisColSelNgDock * dockWidget = new KisColSelNgDock();
        dockWidget->setObjectName(id());

        return dockWidget;
    }

    DockPosition defaultDockPosition() const {
        return DockRight;
    }
};


ColorSelectorNgPlugin::ColorSelectorNgPlugin(QObject *parent, const QVariantList &)
        : QObject(parent)
{

    KoDockRegistry::instance()->add(new ColorSelectorNgDockFactory());
}

ColorSelectorNgPlugin::~ColorSelectorNgPlugin()
{
}

#include "colorselectorng.moc"
