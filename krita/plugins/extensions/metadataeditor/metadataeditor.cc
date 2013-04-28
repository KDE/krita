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

#include <kis_debug.h>
#include <kpluginfactory.h>
#include <klocale.h>

#include "kis_config.h"
#include "kis_cursor.h"
#include "kis_global.h"
#include "kis_layer.h"
#include "kis_node_manager.h"
#include "kis_types.h"
#include "kis_view2.h"
#include "kis_action.h"
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
        : KisViewPlugin(parent, "kritaplugins/metadataeditor.rc")
{
    KisAction *action  = new KisAction(i18n("&Edit metadata..."), this);
    action->setActivationFlags(KisAction::ACTIVE_LAYER);
    action->setActivationConditions(KisAction::ACTIVE_NODE_EDITABLE);
    addAction("EditLayerMetaData", action);
    connect(action, SIGNAL(triggered()), this, SLOT(slotEditLayerMetaData()));

    QStringList runtimeVersion = QString(qVersion()).split('.');
    QStringList compileVersion = QString(QT_VERSION_STR).split('.');

    if (runtimeVersion[1] != compileVersion[1]) {
        action->setActivationFlags(KisAction::NEVER_ACTIVATE);
    }
}

metadataeditorPlugin::~metadataeditorPlugin()
{
}

void metadataeditorPlugin::slotEditLayerMetaData()
{
    KisImageWSP image = m_view->image();
    if (!image) return;

    KisMetaDataEditor editor(m_view, m_view->nodeManager()->activeLayer()->metaData());
    editor.exec();
}

#include "metadataeditor.moc"
