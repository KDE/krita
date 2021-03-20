/*
 *  SPDX-FileCopyrightText: 2008 Cyrille Berger <cberger@cberger.net>
 *  SPDX-FileCopyrightText: 2010 Adam Celarek <kdedev at xibo dot at>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "colorselectorng.h"

#include <kpluginfactory.h>

#include <KoDockFactoryBase.h>
#include <KoDockRegistry.h>

#include "kis_color_selector_ng_dock.h"
#include "kis_color_selector_settings.h"
#include "kis_preference_set_registry.h"

K_PLUGIN_FACTORY_WITH_JSON(ColorSelectorNgPluginFactory, "krita_colorselectorng.json", registerPlugin<ColorSelectorNgPlugin>();)


class ColorSelectorNgDockFactory : public KoDockFactoryBase
{
public:
    ColorSelectorNgDockFactory() {
    }

    QString id() const override {
        return QString("ColorSelectorNg");
    }

    virtual Qt::DockWidgetArea defaultDockWidgetArea() const {
        return Qt::RightDockWidgetArea;
    }

    KDDockWidgets::DockWidgetBase *createDockWidget() override {
        KisColorSelectorNgDock * dockWidget = new KisColorSelectorNgDock();
        dockWidget->setObjectName(id());

        return dockWidget;
    }

    DockPosition defaultDockPosition() const override {
        return DockRight;
    }
};


ColorSelectorNgPlugin::ColorSelectorNgPlugin(QObject *parent, const QVariantList &)
        : QObject(parent)
{
    KoDockRegistry::instance()->add(new ColorSelectorNgDockFactory());

    KisPreferenceSetRegistry *preferenceSetRegistry = KisPreferenceSetRegistry::instance();

    KisColorSelectorSettingsFactory* settingsFactory = new KisColorSelectorSettingsFactory();

    //load and save preferences
    //if something in kritarc is missing, then the default from this load function will be used and saved back to kconfig.
    //this way, cfg.readEntry() in any part won't be able to set its own default
    KisPreferenceSet* settings = settingsFactory->createPreferenceSet();
    Q_ASSERT(settings);
    settings->loadPreferences();
    settings->savePreferences();
    delete settings;

    preferenceSetRegistry->add("KisColorSelectorSettingsFactory", settingsFactory);
}

ColorSelectorNgPlugin::~ColorSelectorNgPlugin()
{

}

#include "colorselectorng.moc"
