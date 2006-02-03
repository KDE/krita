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
    m_activated = false;

    connect(v, SIGNAL(childActivated(KoDocumentChild*)),
            this, SLOT(childDeactivated(KoDocumentChild*)));
    connect(v, SIGNAL(childDeactivated(KoDocumentChild*)),
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
KisPaintDeviceSP KisPartLayerImpl::prepareProjection(KisPaintDeviceSP projection,
        const QRect& r) {
    if (!m_doc || !m_doc->document()/* || !m_activated*/) return 0;

    m_cache -> clear();

    QRect intersection(r.intersect(exactBounds()));
    if (intersection.isEmpty())
        return m_cache;
    // XXX: zoom!

    // We know the embedded part's size through the ChildDoc
    // We move it to (0,0), since that is what we will start painting from in paintEverything.
    QRect embedRect(intersection);
    embedRect.moveBy(- exactBounds().x(), - exactBounds().y());

    QPixmap pm1(projection -> convertToQImage(0 /*srgb XXX*/,
                                              intersection.x(), intersection.y(),
                                              intersection.width(), intersection.height()));
    QPixmap pm2(extent().width(), extent().height());
    copyBlt(&pm2, embedRect.x(), embedRect.y(), &pm1,
             0, 0, embedRect.width(), embedRect.height());
    QPainter painter(&pm2);
    painter.setClipRect(embedRect);

    // KWord's KWPartFrameSet::drawFrameContents has some interesting remarks concerning
    // the semantics of the paintEverything call.
    // Since a Krita Device really is displaysize/zoom agnostic, caring about zoom is not
    // really as important here. What we paint at the moment, is just (0,0)x(w,h)
    // Paint transparent, no zoom:
    m_doc -> document() -> paintEverything(painter, exactBounds(), true);

    copyBlt(&pm1, 0, 0, &pm2,
             embedRect.x(), embedRect.y(), embedRect.width(), embedRect.height());
    QImage qimg = pm1.convertToImage();

    //assume the part is sRGB for now, and that "" is sRGB
    // And we need to paint offsetted
    m_cache -> convertFromQImage(qimg, "", intersection.left(), intersection.top());

    return m_cache;
}

QImage KisPartLayerImpl::createThumbnail(Q_INT32 w, Q_INT32 h) {
    QRect bounds(exactBounds());
    QPixmap pm(w, h);
    QPainter painter(&pm);

    painter.fillRect(0, 0, w, h, Qt::white);

    painter.scale(w / bounds.width(), h / bounds.height());
    m_doc -> document() -> paintEverything(painter, bounds);
    QImage qimg = pm.convertToImage();

    return qimg;
}


#include "kis_part_layer.moc"
