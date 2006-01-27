/*
 *  Copyright (c) 2005 Boudewijn Rempt <boud@valdyas.org>
 *            (c) 2005 Bart Coppens <kde@bartcoppens.be>
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

#include <klocale.h>

#include "koDocument.h"
#include "koDocumentChild.h"
#include "koFrame.h"
#include "koView.h"

#include "kis_layer.h"
#include "kis_types.h"
#include "kis_colorspace_factory_registry.h"
#include "kis_part_layer.h"
#include "kis_factory.h"
#include "kis_paint_device.h"
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
    // XXX doesn't this get deleted by itself or by anything else? Certainly looks so
    // (otherwise I get a double deletion of a QObject, and krita crashes)
    //delete m_doc;
}


KisPartLayerImpl::KisPartLayerImpl(KisView* v, KisImageSP img, KisChildDoc * doc)
    : super(img, i18n("Embedded Document"), OPACITY_OPAQUE), m_doc(doc)
{
    m_cache = new KisPaintDevice(
            KisMetaRegistry::instance()->csRegistry()->getColorSpace(KisID("RGBA",""),"") );
    m_activated = true; //false;
    //image() -> notify(m_doc -> geometry());

    // XXX Waiting for dfaure...
    /*QPtrList<KoView> views = doc -> document() -> views();
    for (uint i = 0; i < views.count(); i++) {
        connect(views.at(i), SIGNAL(childActivated(KoDocumentChild*)),
                this, SLOT(childActivated(KoDocumentChild*)));
        connect(views.at(i), SIGNAL(childDeactivated(KoDocumentChild*)),
                this, SLOT(childDeactivated(KoDocumentChild*)));
}*/
    connect(v, SIGNAL(childActivated(KoDocumentChild*)),
            this, SLOT(childDeactivated(KoDocumentChild*)));
    connect(v, SIGNAL(childDeactivated(KoDocumentChild*)),
            this, SLOT(childActivated(KoDocumentChild*)));
    connect(v, SIGNAL(childUnselected(KoDocumentChild*)),
            this, SLOT(childActivated(KoDocumentChild*)));
}

KisPartLayerImpl::~KisPartLayerImpl()
{
}

KisLayerSP KisPartLayerImpl::clone() const {
    kdDebug(41001) << "Whoops, clone for partlayers, how do I do that best?" << endl;
    return 0;
}

// Called when the layer is made active
void KisPartLayerImpl::childActivated(KoDocumentChild* child)
{
    kdDebug(41001) << "Activate object layer\n";

    // Clear the image, so that if we move the part while activated, no ghosts show up
    //if (!m_activated /*&& child == childDoc()*/) {
        QRect rect = extent();
        m_activated = true;
        image() -> notify(rect);
    //}
}

// Called when another layer is made inactive
void KisPartLayerImpl::childDeactivated(KoDocumentChild* child)
{
    kdDebug(41001) << "Deactivate object layer: going to render onto paint device.\n";
    // We probably changed, notify the image that it needs to repaint where we currently updated
    // We use the original geometry
    //if (m_activated/* && child == childDoc()*/) {
    //    m_activated = false;
        image() -> notify(m_doc -> geometry());
    //}
}

void KisPartLayerImpl::setX(Q_INT32 x) {
    QRect rect = m_doc -> geometry();

    // KisPaintDevice::move moves to absolute coordinates, not relative. Work around that here,
    // since the part is not necesarily started at (0,0)
    rect.moveBy(x - this -> x(), 0);
    m_doc -> setGeometry(rect);

//    m_paintLayer -> setX(x);
}

void KisPartLayerImpl::setY(Q_INT32 y) {
    QRect rect = m_doc -> geometry();

    // KisPaintDevice::move moves to absolute coordinates, not relative. Work around that here,
    // since the part is not necesarily started at (0,0)
    rect.moveBy(0, y - this -> y());
    m_doc -> setGeometry(rect);

//    m_paintLayer -> setY(y);
}

void KisPartLayerImpl::paintSelection(QImage &img, Q_INT32 x, Q_INT32 y, Q_INT32 w, Q_INT32 h) {
    uchar *j = img.bits();
    QRect rect = m_doc -> geometry();

    for (int y2 = y; y2 < h + y; ++y2) {
        for (int x2 = x; x2 < w + x; ++x2) {
            if (!rect.contains(x2, y2)) {
                Q_UINT8 g = (*(j + 0)  + *(j + 1 ) + *(j + 2 )) / 9;
                *(j+0) = 165+g;
                *(j+1) = 128+g;
                *(j+2) = 128+g;
            }
            j+=4;
        }
    }

}

//void KisPartLayerImpl::repaint() {
// XXX maybe add x,y,w,h params and use them
KisPaintDeviceSP KisPartLayerImpl::prepareProjection(KisPaintDeviceSP projection) {
    if (!m_doc || !m_doc->document() || !m_activated) return 0;

    // XXX: zoom!

    // We know the embedded part's size through the ChildDoc
    // We move it to (0,0), since that is what we will start painting from in paintEverything.
    QRect rect = m_doc -> geometry();
    QRect sizeRect = rect;
    sizeRect.moveTopLeft(QPoint(0,0));

    QPixmap pm(projection -> convertToQImage(0 /*srgb XXX*/,
                                             x(), y(), sizeRect.width(), sizeRect.height()));
    QPainter painter(&pm);

    // KWord's KWPartFrameSet::drawFrameContents has some interesting remarks concerning
    // the semantics of the paintEverything call.
    // Since a Krita Device really is displaysize/zoom agnostic, caring about zoom is not
    // really as important here. What we paint at the moment, is just (0,0)x(w,h)
    // Paint transparent, no zoom:
    m_doc -> document() -> paintEverything(painter, sizeRect, true);

    QImage qimg = pm.convertToImage();

    //assume the part is sRGB for now, and that "" is sRGB
    // And we need to paint offsetted
    m_cache -> clear();
    m_cache -> convertFromQImage(qimg, "", rect.left(), rect.top());

    return m_cache;
}

#include "kis_part_layer.moc"
