/*
 *  SPDX-FileCopyrightText: 2009 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "artisticcolorselector_plugin.h"
#include "artisticcolorselector_dock.h"

#include <kpluginfactory.h>
#include <klocalizedstring.h>
#include <KoDockFactoryBase.h>
#include <KoDockRegistry.h>

K_PLUGIN_FACTORY_WITH_JSON(PaletteDockPluginFactory, "krita_artisticcolorselector.json", registerPlugin<ArtisticColorSelectorPlugin>();)

class ArtisticColorSelectorDockFactory: public KoDockFactoryBase
{
public:
    QString id() const override {
        return QString("ArtisticColorSelector");
    }

    virtual Qt::DockWidgetArea defaultDockWidgetArea() const {
        return Qt::RightDockWidgetArea;
    }

    KDDockWidgets::DockWidgetBase *createDockWidget() override {
        ArtisticColorSelectorDock* dockWidget = new ArtisticColorSelectorDock();
        dockWidget->setObjectName(id());
        return dockWidget;
    }

    DockPosition defaultDockPosition() const override {
        return DockMinimized;
    }
};


ArtisticColorSelectorPlugin::ArtisticColorSelectorPlugin(QObject* parent, const QVariantList &):
    QObject(parent)
{
    KoDockRegistry::instance()->add(new ArtisticColorSelectorDockFactory());
}

#include "artisticcolorselector_plugin.moc"
