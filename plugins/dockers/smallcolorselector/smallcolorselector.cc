/*
 *  Copyright (c) 2008 Cyrille Berger <cberger@cberger.net>
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

#include "smallcolorselector.h"


#include <kpluginfactory.h>

#include <KoDockFactoryBase.h>
#include <KoDockRegistry.h>

#include "smallcolorselector_dock.h"
#include "opengl/kis_opengl.h"

K_PLUGIN_FACTORY_WITH_JSON(SmallColorSelectorPluginFactory, "krita_smallcolorselector.json", registerPlugin<SmallColorSelectorPlugin>();)

class SmallColorSelectorDockFactory : public KoDockFactoryBase
{
public:
    SmallColorSelectorDockFactory() {
    }

    QString id() const override {
        return QString("SmallColorSelector");
    }

    virtual Qt::DockWidgetArea defaultDockWidgetArea() const {
        return Qt::RightDockWidgetArea;
    }

    QDockWidget* createDockWidget() override {
        SmallColorSelectorDock * dockWidget = new SmallColorSelectorDock();
        dockWidget->setObjectName(id());

        return dockWidget;
    }

    DockPosition defaultDockPosition() const override {
        return DockRight;
    }
};

SmallColorSelectorPlugin::SmallColorSelectorPlugin(QObject *parent, const QVariantList &)
        : QObject(parent)
{
    if (KisOpenGL::hasOpenGL3() || KisOpenGL::hasOpenGLES()) {
        KoDockRegistry::instance()->add(new SmallColorSelectorDockFactory());
    }
}

SmallColorSelectorPlugin::~SmallColorSelectorPlugin()
{
}

#include "smallcolorselector.moc"
