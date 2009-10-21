/*
 * Copyright (c) 2009 Cyrille Berger <cberger@cberger.net>
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

#include "ExtensionsManagerPlugin.h"

#include <stdlib.h>

#include <kactioncollection.h>
#include <kcomponentdata.h>
#include <kis_debug.h>
#include <kgenericfactory.h>
#include <klocale.h>
#include <kstandarddirs.h>

#include "kis_config.h"
#include "kis_cursor.h"
#include "kis_global.h"
#include "kis_types.h"
#include "kis_view2.h"
#include "ExtensionsManagerWindow.h"

typedef KGenericFactory<ExtensionsManagerPlugin> ExtensionsManagerPluginFactory;
K_EXPORT_COMPONENT_FACTORY(kritaextensionsmanager, ExtensionsManagerPluginFactory("krita"))


ExtensionsManagerPlugin::ExtensionsManagerPlugin(QObject *parent, const QStringList &)
        : KParts::Plugin(parent), m_emWindow(0)
{
    if (parent->inherits("KisView2")) {
        m_view = (KisView2*) parent;

        setComponentData(ExtensionsManagerPluginFactory::componentData());

        setXMLFile(KStandardDirs::locate("data", "kritaplugins/extensionsmanager.rc"), true);

        KAction *action  = new KAction(i18n("Extensions manager..."), this);
        actionCollection()->addAction("ExtensionsManager", action);
        connect(action, SIGNAL(triggered()), this, SLOT(slotMyAction()));
    }
}

ExtensionsManagerPlugin::~ExtensionsManagerPlugin()
{
    m_view = 0;
}

void ExtensionsManagerPlugin::slotMyAction()
{
    if (!m_emWindow) {
        m_emWindow = new ExtensionsManagerWindow;
    }
    m_emWindow->setVisible(true);
}

#include "ExtensionsManagerPlugin.moc"
