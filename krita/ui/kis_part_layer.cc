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
    // XXX doesn't this get deleted by itself or by anything else? Certainly looks so
    // (otherwise I get a double deletion of a QObject, and krita crashes)
    //delete m_doc;
}


KisPartLayer::KisPartLayer(KisImageSP img, KisChildDoc * doc)
    : super(img, "embedded document", OPACITY_OPAQUE,
            KisMetaRegistry::instance()->csRegistry()->getColorSpace(KisID("RGBA",""),""))
    , m_doc(doc)
{
    repaint();
}

KisPartLayer::~KisPartLayer()
{
}

// Called when the layer is made active: this is essentially a duplication of the code in KoView::partActivateEvent( KParts::PartActivateEvent *event )
void KisPartLayer::activate()
{
    kdDebug() << "Activate object layer\n";
    repaint();
}

// Called when another layer is made inactive
void KisPartLayer::deactivate()
{
    kdDebug() << "Deactivate object layer: going to render onto paint device.\n";
    repaint();
}

void KisPartLayer::setX(Q_INT32 x) {
    QRect rect = m_doc -> geometry();

    // KisPaintDevice::move moves to absolute coordinates, not relative. Work around that here,
    // since the part is not necesarily started at (0,0)
    rect.moveBy(x - this -> x(), 0);
    m_doc -> setGeometry(rect);

    super::setX(x);
}

void KisPartLayer::setY(Q_INT32 y) {
    QRect rect = m_doc -> geometry();

    // KisPaintDevice::move moves to absolute coordinates, not relative. Work around that here,
    // since the part is not necesarily started at (0,0)
    rect.moveBy(0, y - this -> y());
    m_doc -> setGeometry(rect);

    super::setY(y);
}

void KisPartLayer::paintSelection(QImage &img, Q_INT32 x, Q_INT32 y, Q_INT32 w, Q_INT32 h) {
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

void KisPartLayer::repaint() {
    if (!m_doc || !m_doc->document()) return;

    paintDevice() -> clear();

    // XXX: zoom!

    // We know the embedded part's size through the ChildDoc
    // We move it to (0,0), since that is what we will start painting from in paintEverything.
    QRect rect = m_doc -> geometry();
    QRect sizeRect = rect;
    sizeRect.moveTopLeft(QPoint(0,0));

    QPixmap pm(sizeRect.width(), sizeRect.height());
    pm.fill(QColor(qRgba(255,255,255,255)));

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
    paintDevice() -> convertFromQImage(qimg, "", rect.left(), rect.top());

    // We probably changed, notify the image that it needs to repaint where we currently updated
    // We use the original geometry
    image() -> notify(rect);
}

#include "kis_part_layer.moc"
