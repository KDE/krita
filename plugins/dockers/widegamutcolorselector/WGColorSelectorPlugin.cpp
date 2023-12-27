/*
 * SPDX-FileCopyrightText: 2020 Mathias Wein <lynx.mw+kde@gmail.com>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "WGColorSelectorDock.h"

#include <kpluginfactory.h>
//#include <klocalizedstring.h>
#include <KoDockFactoryBase.h>
#include <KoDockRegistry.h>

class WGColorSelectorPlugin: public QObject
{
	Q_OBJECT
public:
    WGColorSelectorPlugin(QObject *parent, const QVariantList &);
    ~WGColorSelectorPlugin() override;
};

K_PLUGIN_FACTORY_WITH_JSON(WGColorSelectorPluginFactory, "krita_widegamutcolorselector.json", registerPlugin<WGColorSelectorPlugin>();)

class WGColorSelectorDockFactory : public KoDockFactoryBase
{
public:
    WGColorSelectorDockFactory() {
    }

    QString id() const override {
        return QString("WideGamutColorSelector");
    }

    QDockWidget* createDockWidget() override {
        WGColorSelectorDock * dockWidget = new WGColorSelectorDock();
        dockWidget->setObjectName(id());

        return dockWidget;
    }

    DockPosition defaultDockPosition() const override {
        return DockMinimized;
    }
};


WGColorSelectorPlugin::WGColorSelectorPlugin(QObject *parent, const QVariantList &)
        : QObject(parent)
{
    KoDockRegistry::instance()->add(new WGColorSelectorDockFactory());

    // TODO: own preference set
}

WGColorSelectorPlugin::~WGColorSelectorPlugin()
{

}

#include "WGColorSelectorPlugin.moc"
