/*
 *  Copyright (c) 2008 Cyrille Berger <cberger@cberger.net>
 *  Copyright (c) 2014 Wolthera van Hövell <griffinvalley@gmail.com>
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

#include "kis_color_slider.h"
#include <stdlib.h>

#include <QTimer>


#include <kis_debug.h>
#include <kpluginfactory.h>
#include <klocale.h>

#include <KoDockFactoryBase.h>
#include <KoDockRegistry.h>

#include "kis_config.h"
#include "kis_cursor.h"
#include "kis_global.h"
#include "kis_types.h"
#include "KisViewManager.h"

#include "kis_color_slider_dock.h"

K_PLUGIN_FACTORY(ColorSliderPluginFactory, registerPlugin<ColorSliderPlugin>();)
K_EXPORT_PLUGIN(ColorSliderPluginFactory("krita"))

class ColorSliderDockFactory : public KoDockFactoryBase
{
public:
    ColorSliderDockFactory() {
    }

    virtual QString id() const {
        return QString("ColorSlider");
    }

    virtual Qt::DockWidgetArea defaultDockWidgetArea() const {
        return Qt::RightDockWidgetArea;
    }

    virtual QDockWidget* createDockWidget() {
        ColorSliderDock * dockWidget = new ColorSliderDock();

        dockWidget->setObjectName(id());

        return dockWidget;
    }

    KoDockFactoryBase::DockPosition defaultDockPosition() const {
        return DockMinimized;
    }
};


ColorSliderPlugin::ColorSliderPlugin(QObject *parent, const QVariantList &)
        : QObject(parent)
{
    dbgPlugins << "ColorSliderPlugin";

    KoDockRegistry::instance()->add(new ColorSliderDockFactory());
}

ColorSliderPlugin::~ColorSliderPlugin()
{
}

#include "kis_color_slider.moc"
