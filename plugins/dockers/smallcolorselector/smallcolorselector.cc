/*
 *  SPDX-FileCopyrightText: 2008 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
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
