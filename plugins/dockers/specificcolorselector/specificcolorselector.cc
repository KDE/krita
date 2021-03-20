/*
 *  SPDX-FileCopyrightText: 2008 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "specificcolorselector.h"
#include <stdlib.h>

#include <QTimer>


#include <kis_debug.h>
#include <kpluginfactory.h>
#include <klocalizedstring.h>

#include <KoDockFactoryBase.h>
#include <KoDockRegistry.h>

#include "kis_config.h"
#include "kis_cursor.h"
#include "kis_global.h"
#include "kis_types.h"
#include "KisViewManager.h"

#include "specificcolorselector_dock.h"

K_PLUGIN_FACTORY_WITH_JSON(SpecificColorSelectorPluginFactory, "krita_specificcolorselector.json", registerPlugin<SpecificColorSelectorPlugin>();)

class SpecificColorSelectorDockFactory : public KoDockFactoryBase
{
public:
    SpecificColorSelectorDockFactory() {
    }

    QString id() const override {
        return QString("SpecificColorSelector");
    }

    virtual Qt::DockWidgetArea defaultDockWidgetArea() const {
        return Qt::RightDockWidgetArea;
    }

    QDockWidget* createDockWidget() override {
        SpecificColorSelectorDock * dockWidget = new SpecificColorSelectorDock();

        dockWidget->setObjectName(id());

        return dockWidget;
    }

    KoDockFactoryBase::DockPosition defaultDockPosition() const override {
        return DockMinimized;
    }
};


SpecificColorSelectorPlugin::SpecificColorSelectorPlugin(QObject *parent, const QVariantList &)
        : QObject(parent)
{
    dbgPlugins << "SpecificColorSelectorPlugin";

    KoDockRegistry::instance()->add(new SpecificColorSelectorDockFactory());
}

SpecificColorSelectorPlugin::~SpecificColorSelectorPlugin()
{
}

#include "specificcolorselector.moc"
