/*
 *  Copyright (c) 2006 Boudewijn Rempt  <boud@valdyas.org>
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

#include "kis_import_catcher.h"
#include <kis_debug.h>

#include <kaboutdata.h>
#include <kimageio.h>
#include <kcmdlineargs.h>
#include <klocale.h>
#include <kglobal.h>
#include <kmimetype.h>
#include <kapplication.h>
#include <kdebug.h>
#include <kio/netaccess.h>
#include <kio/job.h>

#include <KoFilterManager.h>

#include "kis_node_manager.h"
#include "kis_types.h"
#include "kis_count_visitor.h"
#include "kis_view2.h"
#include "kis_doc2.h"
#include "kis_image.h"
#include "kis_layer.h"
#include "kis_group_layer.h"
#include <QMessageBox>

class KisImportCatcher::Private
{
public:
    KisDoc2* doc;
    KisView2* view;
    KUrl url;
};

KisImportCatcher::KisImportCatcher(const KUrl & url, KisView2 * view)
        : m_d(new Private)
{
    m_d->doc = new KisDoc2(0);
    m_d->view = view;
    m_d->url = url;
    KoFilterManager manager(m_d->doc);
    QByteArray nativeFormat = m_d->doc->nativeFormatMimeType();
    KoFilter::ConversionStatus status;
    QString s = manager.importDocument(url.path(), status);
    KisImageWSP importedImage = m_d->doc->image();

    if (importedImage) {
        KisLayerSP importedImageLayer = KisLayerSP(importedImage->rootLayer().data());

        if (!importedImageLayer.isNull()) {
            QStringList list;
            list << "KisLayer";

            KisCountVisitor visitor(list, KoProperties());
            importedImageLayer->accept(visitor);

            if (visitor.count() == 2) {
                // Don't import the root if this is not a layered image (1 group layer
                // plus 1 other).
                importedImageLayer = dynamic_cast<KisLayer*>(importedImageLayer->firstChild().data());
                if (importedImageLayer)
                    importedImage->removeNode(importedImageLayer.data());
            }

            importedImageLayer->setName(m_d->url.prettyUrl());
            importedImageLayer->setImage(m_d->view->image());

            KisNodeSP parent = 0;
            KisLayerSP currentActiveLayer = m_d->view->activeLayer();

            if (currentActiveLayer) {
                parent = currentActiveLayer->parent();
            }

            if (parent.isNull()) {
                parent = m_d->view->image()->rootLayer();
            }

            m_d->view->image()->addNode(importedImageLayer.data(), parent, currentActiveLayer.data());
            m_d->view->nodeManager()->activateNode(importedImageLayer.data());
            importedImageLayer->setDirty();
        }
    }
    m_d->doc->deleteLater();
    deleteLater();
}

KisImportCatcher::~KisImportCatcher()
{
    delete m_d;
}

#include "kis_import_catcher.moc"
