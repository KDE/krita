/*
 * Copyright (c) 2007 Boudewijn Rempt <boud@kde.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "kis_layer_compose_plugin.h"
#include <klocale.h>
#include <kiconloader.h>
#include <kcomponentdata.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <kis_debug.h>
#include <kpluginfactory.h>
#include <kactioncollection.h>

#include <kis_view2.h>
#include <kis_types.h>
#include <kis_image.h>
#include <kis_paint_device.h>
#include <kis_layer.h>
#include <kis_statusbar.h>
#include <kis_layer_manager.h>


#include "dlg_compose.h"

K_PLUGIN_FACTORY(KisLayerComposePluginFactory, registerPlugin<KisLayerComposePlugin>();)
K_EXPORT_PLUGIN(KisLayerComposePluginFactory("krita"))

KisLayerComposePlugin::KisLayerComposePlugin(QObject *parent, const QVariantList &)
        : KParts::Plugin(parent)
{
    if (parent->inherits("KisView2")) {
        setComponentData(KisLayerComposePluginFactory::componentData());

        setXMLFile(KStandardDirs::locate("data", "kritaplugins/imageseparate.rc"), true);
        m_view = (KisView2*) parent;
        KAction *action  = new KAction(i18n("Compose Layer..."), this);
        actionCollection()->addAction("layercompose", action);
        connect(action, SIGNAL(triggered(bool)), SLOT(slotCompose()));
    }
}

KisLayerComposePlugin::~KisLayerComposePlugin()
{
}

void KisLayerComposePlugin::slotCompose()
{
    KisImageWSP image = m_view->image();
    if (!image) return;

    DlgCompose dlgCompose(image, m_view);

    dlgCompose.setCaption(i18n("Compose Layer"));

    if (dlgCompose.exec() == QDialog::Accepted) {
        // XXX: Implement
    }

}

#include "kis_layer_compose_plugin.moc"
