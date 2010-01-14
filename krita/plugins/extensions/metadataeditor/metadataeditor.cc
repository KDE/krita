/*
 *  Copyright (c) 2007 Cyrille Berger <cberger@cberger.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
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

#include "metadataeditor.h"
#include <stdlib.h>

#include <QDialog>
#include <QUiLoader>
#include <QVBoxLayout>

#include <kactioncollection.h>
#include <kcomponentdata.h>
#include <kis_debug.h>
#include <kpluginfactory.h>
#include <klocale.h>
#include <kstandarddirs.h>

#include "kis_config.h"
#include "kis_cursor.h"
#include "kis_global.h"
#include "kis_layer.h"
#include "kis_layer_manager.h"
#include "kis_types.h"
#include "kis_view2.h"
#include "kis_image.h"

#include <kis_meta_data_store.h>
#include <kis_meta_data_entry.h>
#include <kis_meta_data_value.h>
#include <kis_meta_data_schema.h>

#include "kis_entry_editor.h"
#include "kis_meta_data_editor.h"

K_PLUGIN_FACTORY(metadataeditorPluginFactory, registerPlugin<metadataeditorPlugin>();)
K_EXPORT_PLUGIN(metadataeditorPluginFactory("krita"))

metadataeditorPlugin::metadataeditorPlugin(QObject *parent, const QVariantList &)
        : KParts::Plugin(parent)
{
    if (parent->inherits("KisView2")) {
        m_view = (KisView2*) parent;

        setComponentData(metadataeditorPluginFactory::componentData());

        setXMLFile(KStandardDirs::locate("data", "kritaplugins/metadataeditor.rc"), true);

        KAction *action  = new KAction(i18n("&Edit metadata..."), this);
        actionCollection()->addAction("EditLayerMetaData", action);
        connect(action, SIGNAL(triggered()), this, SLOT(slotEditLayerMetaData()));
    }

}

metadataeditorPlugin::~metadataeditorPlugin()
{
    m_view = 0;
}

void metadataeditorPlugin::slotEditLayerMetaData()
{
    KisImageWSP image = m_view->image();
    if (!image) return;

    KisMetaDataEditor editor(m_view, m_view->layerManager()->activeLayer()->metaData());
    editor.exec();
}

#include "metadataeditor.moc"
