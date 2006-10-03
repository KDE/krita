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
#include <kdebug.h>

#include "kis_import_catcher.h"
#include "kis_types.h"

#include "kis_view.h"
#include "kis_doc.h"
#include "kis_image.h"
#include "kis_layer.h"
#include "kis_group_layer.h"

KisImportCatcher::KisImportCatcher(KURL url, KisImageSP image)
    : QObject()
    , m_doc( new KisDoc() )
    , m_image( image )
    , m_url( url )
{
    m_doc->openURL(url);
    if ( !m_doc->isLoading() ) {
        slotLoadingFinished();
    }
    else {
        connect(m_doc, SIGNAL(loadingFinished()), this, SLOT(slotLoadingFinished()));
    }
}

void KisImportCatcher::slotLoadingFinished()
{
    KisImageSP importedImage = m_doc->currentImage();

    if (importedImage) {
        KisLayerSP importedImageLayer = importedImage->rootLayer().data();

        if (importedImageLayer != 0) {

            if (importedImageLayer->numLayers() == 2) {
                // Don't import the root if this is not a layered image (1 group layer
                // plus 1 other).
                importedImageLayer = importedImageLayer->firstChild();
                importedImageLayer->parent()->removeLayer(importedImageLayer);
            }

            importedImageLayer->setName(m_url.prettyURL());

            KisGroupLayerSP parent = 0;
            KisLayerSP currentActiveLayer = m_image->activeLayer();

            if (currentActiveLayer) {
                parent = currentActiveLayer->parent();
            }

            if (parent == 0) {
                parent = m_image->rootLayer();
            }

            m_image->addLayer(importedImageLayer.data(), parent, currentActiveLayer);
        }
    }
    m_doc->deleteLater();
    deleteLater();
}


