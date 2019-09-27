/*
 *  Copyright (c) 2008 Cyrille Berger <cberger@cberger.net>
 *  Copyright (c) 2010 Adam Celarek <kdedev at xibo dot at>
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

    QDockWidget* createDockWidget() override {
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
