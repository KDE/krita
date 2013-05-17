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
#include <KoColorSpaceRegistry.h>

#include "kis_node_manager.h"
#include "kis_types.h"
#include "kis_count_visitor.h"
#include "kis_view2.h"
#include "kis_doc2.h"
#include "kis_image.h"
#include "kis_layer.h"
#include "kis_painter.h"
#include "kis_selection.h"
#include "kis_node_commands_adapter.h"
#include "kis_group_layer.h"
#include "kis_statusbar.h"
#include "kis_progress_widget.h"

#include <QMessageBox>

struct KisImportCatcher::Private
{
public:
    KisDoc2* doc;
    KisView2* view;
    KUrl url;
    bool importAsLayer;

    QString prettyLayerName() const;
    void importAsPaintLayer(KisPaintDeviceSP device);
    void importAsTransparencyMask(KisPaintDeviceSP device);
};

QString KisImportCatcher::Private::prettyLayerName() const
{
    QString name = url.fileName();
    return !name.isEmpty() ? name : url.prettyUrl();
}

void KisImportCatcher::Private::importAsPaintLayer(KisPaintDeviceSP device)
{
    KisLayerSP newLayer =
        new KisPaintLayer(view->image(),
                          prettyLayerName(),
                          OPACITY_OPAQUE_U8,
                          device);

    KisNodeSP parent = 0;
    KisLayerSP currentActiveLayer = view->activeLayer();

    if (currentActiveLayer) {
        parent = currentActiveLayer->parent();
    }

    if (parent.isNull()) {
        parent = view->image()->rootLayer();
    }

    KisNodeCommandsAdapter adapter(view);
    adapter.addNode(newLayer, parent, currentActiveLayer);
}

void KisImportCatcher::Private::importAsTransparencyMask(KisPaintDeviceSP device)
{
    KisLayerSP currentActiveLayer = view->activeLayer();

    if (!currentActiveLayer) {
        KisNodeSP node = view->activeNode();
        if (!node) return;

        do {
            currentActiveLayer = dynamic_cast<KisLayer*>(node.data());
        } while (!currentActiveLayer && (node = node->parent()));

        if (!currentActiveLayer) return;
    }

    KisTransparencyMaskSP mask = new KisTransparencyMask();
    mask->setSelection(new KisSelection(new KisDefaultBounds(currentActiveLayer->image())));
    mask->setName(prettyLayerName());

    QRect rc(device->exactBounds());
    KisPainter painter(mask->paintDevice());
    painter.bitBlt(rc.topLeft(), device, rc);

    KisNodeCommandsAdapter adapter(view);
    adapter.addNode(mask,
                    currentActiveLayer,
                    currentActiveLayer->lastChild());
}

KisImportCatcher::KisImportCatcher(const KUrl & url, KisView2 * view, bool importAsLayer)
        : m_d(new Private)
{
    m_d->doc = new KisDoc2();

    KoProgressProxy *progressProxy = view->statusBar()->progress()->progressProxy();
    m_d->doc->setProgressProxy(progressProxy);
    m_d->view = view;
    m_d->url = url;
    m_d->importAsLayer = importAsLayer;
    connect(m_d->doc, SIGNAL(sigLoadingFinished()), this, SLOT(slotLoadingFinished()));
    bool result = m_d->doc->openUrl(url);

    if (!result) {
        deleteMyself();
    }
}

void KisImportCatcher::slotLoadingFinished()
{
    KisImageWSP importedImage = m_d->doc->image();
    importedImage->waitForDone();

    if (importedImage && importedImage->projection()->exactBounds().isValid()) {
        if (m_d->importAsLayer) {
            m_d->importAsPaintLayer(importedImage->projection());
        } else {
            m_d->importAsTransparencyMask(importedImage->projection());
        }
    }

    deleteMyself();
}

void KisImportCatcher::deleteMyself()
{
    m_d->doc->deleteLater();
    deleteLater();
}

KisImportCatcher::~KisImportCatcher()
{
    delete m_d;
}

#include "kis_import_catcher.moc"
