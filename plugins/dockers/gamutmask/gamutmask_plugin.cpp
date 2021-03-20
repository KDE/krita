/*
 *  SPDX-FileCopyrightText: 2018 Anna Medonosova <anna.medonosova@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "gamutmask_plugin.h"
#include "gamutmask_dock.h"

#include <kpluginfactory.h>
#include <klocalizedstring.h>
#include <KoDockFactoryBase.h>
#include <KoDockRegistry.h>

K_PLUGIN_FACTORY_WITH_JSON(PaletteDockPluginFactory, "krita_gamutmask.json", registerPlugin<GamutMaskPlugin>();)

class GamutMaskDockFactory: public KoDockFactoryBase
{
public:
    QString id() const override {
        return QString("GamutMask");
    }

    virtual Qt::DockWidgetArea defaultDockWidgetArea() const {
        return Qt::RightDockWidgetArea;
    }

    KDDockWidgets::DockWidgetBase *createDockWidget() override {
        GamutMaskDock* dockWidget = new GamutMaskDock();
        dockWidget->setObjectName(id());
        return dockWidget;
    }

    DockPosition defaultDockPosition() const override {
        return DockMinimized;
    }
};


GamutMaskPlugin::GamutMaskPlugin(QObject* parent, const QVariantList &):
    QObject(parent)
{
    KoDockRegistry::instance()->add(new GamutMaskDockFactory());
}

#include "gamutmask_plugin.moc"
