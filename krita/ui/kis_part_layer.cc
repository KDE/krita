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
//Added by qt3to4:
#include <Q3PtrList>

#include <kaboutdata.h>
#include <kinstance.h>
#include <klocale.h>

#include "KoDocument.h"
#include "KoDocumentChild.h"
#include "KoFrame.h"
#include "KoView.h"

#include "kis_layer.h"
#include "kis_types.h"
#include "KoColorSpaceRegistry.h"
#include "kis_part_layer.h"
#include "kis_group_layer.h"
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


KisPartLayerImpl::KisPartLayerImpl(KisImageSP img, KisChildDoc * doc)
    : super(img.data(), i18n("Embedded Document"), OPACITY_OPAQUE), m_doc(doc)
{
    m_cache = new KisPaintDevice(
            KisMetaRegistry::instance()->csRegistry()->colorSpace("RGBA",0), name().toLatin1() );
    m_activated = false;
}

KisPartLayerImpl::~KisPartLayerImpl()
{
}

KoDocumentSectionModel::PropertyList KisPartLayerImpl::properties() const
{
    PropertyList l = super::properties();
    QString type = docType();
    if (type.isEmpty())
        type = childDoc()->document()->instance()->aboutData()->programName();
    l << Property(i18n("Document type"), type);
    return l;
}

KisLayerSP KisPartLayerImpl::clone() const {
    return KisLayerSP(new KisPartLayerImpl(KisImageSP(image()), childDoc()));
}

// Called when the layer is made active
void KisPartLayerImpl::childActivated(KoDocumentChild* child)
{
    // Clear the image, so that if we move the part while activated, no ghosts show up
    if (!m_activated && child == m_doc) {
        QRect rect = extent();
        m_activated = true;
        setDirty(rect);
        QList<KoView*> views = child->parentDocument()->views();
        Q_ASSERT(views.count());
        // XXX iterate over views
        connect(views.at(0), SIGNAL(activated(bool)),
                this, SLOT(childDeactivated(bool)));
    }
}

// Called when another layer is made inactive
void KisPartLayerImpl::childDeactivated(bool activated)
{
    // We probably changed, notify the image that it needs to repaint where we currently updated
    // We use the original geometry
    if (m_activated && !activated /* no clue, but debugging suggests it is false here */) {
        QList<KoView*> views = m_doc->parentDocument()->views();
        Q_ASSERT(views.count());
        views.at(0)->disconnect(SIGNAL(activated(bool)));
        m_activated = false;
        setDirty(m_doc->geometry());
    }
}

void KisPartLayerImpl::setX(qint32 x) {
    QRect rect = m_doc->geometry();

    // KisPaintDevice::move moves to absolute coordinates, not relative. Work around that here,
    // since the part is not necesarily started at (0,0)
    rect.translate(x - this->x(), 0);
    m_doc->setGeometry(rect);
}

void KisPartLayerImpl::setY(qint32 y) {
    QRect rect = m_doc->geometry();

    // KisPaintDevice::move moves to absolute coordinates, not relative. Work around that here,
    // since the part is not necesarily started at (0,0)
    rect.translate(0, y - this->y());
    m_doc->setGeometry(rect);
}

void KisPartLayerImpl::paintSelection(QImage &img, qint32 x, qint32 y, qint32 w, qint32 h) {
    uchar *j = img.bits();
    QRect rect = m_doc->geometry();

    for (int y2 = y; y2 < h + y; ++y2) {
        for (int x2 = x; x2 < w + x; ++x2) {
            if (!rect.contains(x2, y2)) {
                quint8 g = (*(j + 0)  + *(j + 1 ) + *(j + 2 )) / 9;
                *(j+0) = 165+g;
                *(j+1) = 128+g;
                *(j+2) = 128+g;
            }
            j+=4;
        }
    }

}

KisPaintDeviceSP KisPartLayerImpl::prepareProjection(KisPaintDeviceSP projection, const QRect& r)
{
    if (!m_doc || !m_doc->document() || m_activated) return KisPaintDeviceSP(0);

    m_cache->clear();

    QRect intersection(r.intersect(exactBounds()));
    if (intersection.isEmpty())
        return m_cache;
    // XXX: have a look at the comments and see if they still truthfully represent the code :/

    // We know the embedded part's size through the ChildDoc
    // We move it to (0,0), since that is what we will start painting from in paintEverything.
    QRect embedRect(intersection);
    embedRect.translate(- exactBounds().x(), - exactBounds().y());
    QRect paintRect(exactBounds());
    paintRect.translate(- exactBounds().x(), - exactBounds().y());

    QPixmap pm1 = QPixmap::fromImage(projection->convertToQImage(0 /*srgb XXX*/,
                                              intersection.x(), intersection.y(),
                                              intersection.width(), intersection.height()));
    QPixmap pm2(extent().width(), extent().height());
    QPainter painter(&pm2);

    painter.drawPixmap(embedRect.x(), embedRect.y(), pm1, 0, 0, embedRect.width(), embedRect.height());
    painter.setClipRect(embedRect);

    // KWord's KWPartFrameSet::drawFrameContents has some interesting remarks concerning
    // the semantics of the paintEverything call.
    // Since a Krita Device really is displaysize/zoom agnostic, caring about zoom is not
    // really as important here. What we paint at the moment, is just (0,0)x(w,h)
    // Paint transparent, no zoom:
    m_doc->document()->paintEverything(painter, paintRect, true);

    painter.end();
    painter.begin(&pm1);

    painter.drawPixmap(0, 0, pm2, embedRect.x(), embedRect.y(), embedRect.width(), embedRect.height());
    QImage qimg = pm1.toImage();

    //assume the part is sRGB for now, and that "" is sRGB
    // And we need to paint offsetted
    // XXX: Bug! We need to set alpha correctly, here.
    m_cache->convertFromQImage(qimg, "", intersection.left(), intersection.top());

    return m_cache;
}

QImage KisPartLayerImpl::createThumbnail(qint32 w, qint32 h) {
    QRect bounds(exactBounds());
    QPixmap pm(w, h);
    QPainter painter(&pm);

    painter.fillRect(0, 0, w, h, Qt::white);

    painter.scale(w / bounds.width(), h / bounds.height());
    m_doc->document()->paintEverything(painter, bounds);
    QImage qimg = pm.toImage();

    return qimg;
}

bool KisPartLayerImpl::saveToXML(QDomDocument doc, QDomElement elem)
{
    QDomElement embeddedElement = doc.createElement("layer");
    embeddedElement.setAttribute("name", name());

    // x and y are loaded from the rect element in the embedded object tag
    embeddedElement.setAttribute("x", 0);
    embeddedElement.setAttribute("y", 0);

    embeddedElement.setAttribute("opacity", opacity());
    embeddedElement.setAttribute("compositeop", compositeOp().id().id());
    embeddedElement.setAttribute("visible", visible());
    embeddedElement.setAttribute("locked", locked());
    embeddedElement.setAttribute("layertype", "partlayer");
    elem.appendChild(embeddedElement);

    QDomElement objectElem = childDoc()->save(doc);
    embeddedElement.appendChild(objectElem);

    return true;
}

KisConnectPartLayerVisitor::KisConnectPartLayerVisitor(KisImageSP img, KisView* view, bool mode)
    : m_img(img), m_view(view), m_connect(mode)
{
}

bool KisConnectPartLayerVisitor::visit(KisGroupLayer *layer) {
    KisLayerSP child = layer->lastChild();

    while (child) {
        child->accept(*this);
        child = child->prevSibling();
    }

    return true;
}

bool KisConnectPartLayerVisitor::visit(KisPartLayer *layer) {
    if (m_connect) {
        QObject::connect(m_view, SIGNAL(childActivated(KoDocumentChild*)),
                         layer, SLOT(childActivated(KoDocumentChild*)));
    } else {
        QObject::disconnect(m_view, SIGNAL(childActivated(KoDocumentChild*)), layer, 0 );
    }

    return true;
}

bool KisConnectPartLayerVisitor::visit(KisPaintLayer*) {
    return true;
}

bool KisConnectPartLayerVisitor::visit(KisAdjustmentLayer*) {
    return true;
}

#include "kis_part_layer.moc"
