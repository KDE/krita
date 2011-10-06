/*
 *  Copyright (c) 2008 Cyrille Berger <cberger@cberger.net>
 *  Copyright (c) 2010 Adam Celarek <kdedev at xibo dot at>
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

#include "kis_color_selector_ng_dock.h"
#include "kis_color_selector_settings.h"
#include "kis_preference_set_registry.h"

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
        KisColorSelectorNgDock * dockWidget = new KisColorSelectorNgDock();
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

    KisPreferenceSetRegistry *preferenceSetRegistry = KisPreferenceSetRegistry::instance();

    KisColorSelectorSettingsFactory* settingsFactory = new KisColorSelectorSettingsFactory();

    //load and save preferences
    //if something in kritarc is missing, then the default from this load function will be used and saved back to kconfig.
    //this way, cfg.readEntry() in any part won't be able to set its own default
    KisColorSelectorSettings* settings;
    settings = dynamic_cast<KisColorSelectorSettings*>(settingsFactory->createPreferenceSet());
    Q_ASSERT(settings);
    settings->loadPreferences();
    settings->savePreferences();

    preferenceSetRegistry->add("KisColorSelectorSettingsFactory", settingsFactory);
}

ColorSelectorNgPlugin::~ColorSelectorNgPlugin()
{

}

#include "colorselectorng.moc"
