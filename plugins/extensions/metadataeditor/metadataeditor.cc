/*
 *  SPDX-FileCopyrightText: 2007 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: LGPL-2.1-or-later
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

#include <kis_meta_data_store.h>
#include <kis_meta_data_entry.h>
#include <kis_meta_data_value.h>
#include <kis_meta_data_schema.h>

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
