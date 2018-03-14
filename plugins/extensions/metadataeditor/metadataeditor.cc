/*
 *  Copyright (c) 2007 Cyrille Berger <cberger@cberger.net>
 *
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; either version 2.1 of the License, or
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

#include "metadataeditor.h"
#include <stdlib.h>

#include <QDialog>
#include <QVBoxLayout>

#include <kis_debug.h>
#include <kpluginfactory.h>
#include <klocalizedstring.h>

#include "kis_config.h"
#include "kis_cursor.h"
#include "kis_global.h"
#include "kis_layer.h"
#include "kis_node_manager.h"
#include "kis_types.h"
#include "KisViewManager.h"
#include "kis_action.h"
#include "kis_image.h"

#include <metadata/kis_meta_data_store.h>
#include <metadata/kis_meta_data_entry.h>
#include <metadata/kis_meta_data_value.h>
#include <metadata/kis_meta_data_schema.h>

#include "kis_entry_editor.h"
#include "kis_meta_data_editor.h"

K_PLUGIN_FACTORY_WITH_JSON(metadataeditorPluginFactory, "kritametadataeditor.json", registerPlugin<metadataeditorPlugin>();)

metadataeditorPlugin::metadataeditorPlugin(QObject *parent, const QVariantList &)
        : KisActionPlugin(parent)
{
    KisAction *action  = createAction("EditLayerMetaData");
    connect(action, SIGNAL(triggered()), this, SLOT(slotEditLayerMetaData()));
}

metadataeditorPlugin::~metadataeditorPlugin()
{
}

void metadataeditorPlugin::slotEditLayerMetaData()
{
    KisImageWSP image = viewManager()->image();
    if (!image) return;

    KisMetaDataEditor editor(viewManager()->mainWindow(), viewManager()->nodeManager()->activeLayer()->metaData());
    editor.exec();
}

#include "metadataeditor.moc"
