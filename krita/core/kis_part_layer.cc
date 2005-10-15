/*
 *  Copyright (c) 2005 Boudewijn Rempt <boud@valdyas.org>
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
#include "qpaintdevice.h"
#include "qpixmap.h"
#include "qimage.h"
#include "qpainter.h"

#include "koDocument.h"
#include "koDocumentChild.h"
#include "koFrame.h"

#include "kis_layer.h"
#include "kis_types.h"
#include "kis_colorspace_factory_registry.h"
#include "kis_part_layer.h"
#include "kis_factory.h"
#include <kis_meta_registry.h>

KisChildDoc::KisChildDoc ( KisDoc * kisDoc, const QRect & rect, KoDocument * childDoc )
    : KoDocumentChild( kisDoc, childDoc, rect )
    , m_doc(kisDoc)
    , m_partLayer(0)
{
}


KisChildDoc::KisChildDoc ( KisDoc * kisDoc )
    : KoDocumentChild( kisDoc)
    , m_partLayer(0)
{
}

KisChildDoc::~KisChildDoc ()
{
    delete m_doc;
}


KisPartLayer::KisPartLayer(KisImageSP img, KisChildDoc * doc)
    : KisLayer(img, "embedded document", OPACITY_OPAQUE, KisMetaRegistry::instance()->csRegistry()->getColorSpace(KisID("RGBA",""),""))
    , m_doc(doc)
{
}

KisPartLayer::~KisPartLayer()
{
}

// Called when the layer is made active: this is essentially a duplication of the code in KoView::partActivateEvent( KParts::PartActivateEvent *event )
void KisPartLayer::activate()
{
    kdDebug() << "Activate object layer\n";
    // Create a child widget
    // Show
}

// Called when another layer is made active
void KisPartLayer::deactivate()
{
    kdDebug() << "Deactivate object layer: going to render onto paint device.\n";

    if (!m_doc || !m_doc->document()) return;

    // XXX: zoom!

    // XXX: perhaps the embedded doc is smaller or bigger than our image? How can we get the size of the embedded document?

    QRect r = image()->bounds();
    QPixmap pm(r.width(), r.height());
    QPainter painter(&pm);

    m_doc->document()->paintContent(painter, image()->bounds(), true);
    QImage qimg = pm.convertToImage();
    convertFromImage(qimg);
}

#include "kis_part_layer.moc"
