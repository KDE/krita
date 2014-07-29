/*
 * Copyright (c) 2014 Boudewijn Rempt (boud@valdyas.org)
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

#include "plugin.h"
#include "engine.h"
#include "utilities.h"

#include <klocale.h>
#include <kis_debug.h>
#include <kpluginfactory.h>

#include <kis_view2.h>
#include <kis_preference_set_registry.h>
#include <pyqtpluginsettings.h>

K_PLUGIN_FACTORY(KritaPyQtPluginFactory, registerPlugin<KritaPyQtPlugin>();)
K_EXPORT_PLUGIN(KritaPyQtPluginFactory("krita"))

KritaPyQtPlugin::KritaPyQtPlugin(QObject *parent, const QVariantList &)
    : KisViewPlugin(parent, "kritaplugins/kritapykritaplugin.rc")
    , m_engineFailureReason(m_engine.tryInitializeGetFailureReason())
    , m_autoReload(false)
{
    qDebug() << ">>>>>>>>>>>>>>>" << m_engineFailureReason;

    KisPreferenceSetRegistry *preferenceSetRegistry = KisPreferenceSetRegistry::instance();

    PyQtPluginSettingsFactory* settingsFactory = new PyQtPluginSettingsFactory(&m_engine);

    //load and save preferences
    //if something in kritarc is missing, then the default from this load function will be used and saved back to kconfig.
    //this way, cfg.readEntry() in any part won't be able to set its own default
    KisPreferenceSet* settings = settingsFactory->createPreferenceSet();
    Q_ASSERT(settings);
    settings->loadPreferences();
    settings->savePreferences();
    delete settings;

    preferenceSetRegistry->add("PyQtPluginSettingsFactory", settingsFactory);

    // Try to import the `pykrita` module
    PyKrita::Python py = PyKrita::Python();
    PyObject* pykritaPackage = py.moduleImport("pykrita");
    pykritaPackage = py.moduleImport("krita");
    if (pykritaPackage)
    {
        dbgKrita << "Loaded pykrita, now load plugins";
        m_engine.tryLoadEnabledPlugins();
        py.functionCall("_pykritaLoaded", "krita");
    }
    else
    {
        dbgScript << "Cannot load pykrita module";
        m_engine.setBroken();
    }



}

KritaPyQtPlugin::~KritaPyQtPlugin()
{

}

