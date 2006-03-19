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
#ifndef _KIS_PART_LAYER_
#define _KIS_PART_LAYER_

#include <qrect.h>

#include <KoDocument.h>
#include <KoDocumentChild.h>

#include "kis_paint_layer.h"
#include "kis_types.h"
#include "kis_doc.h"
#include "kis_part_layer_iface.h"
#include "kis_view.h"
#include "kis_layer_visitor.h"

class KoFrame;
class KoDocument;


/**
 * The child document is responsible for saving and loading the embedded layers.
 */
class KisChildDoc : public KoDocumentChild
{

public:
    KisChildDoc ( KisDoc * kisDoc, const QRect& rect, KoDocument * childDoc );
    KisChildDoc ( KisDoc * kisDdoc );

    virtual ~KisChildDoc();

    KisDoc * parent() const { return m_doc; }

    void setPartLayer (KisPartLayerSP layer) { m_partLayer = layer; }

    KisPartLayerSP partLayer() const { return m_partLayer; }
protected:

    KisDoc * m_doc;
    KisPartLayerSP m_partLayer;
};


/**
 * A PartLayer is a layer that contains a KOffice Part like a KWord document
 * or a KSpread spreadsheet. Or whatever. A Karbon drawing.
 *
 * The part is rendered into an RBGA8 paint device so we can composite it with
 * the other layers.
 *
 * When it is activated (see activate()), it draws a rectangle around itself on the kisdoc,
 * whereas when it is deactivated (deactivate()), it removes that rectangle and commits
 * the child to the paint device.
 *
 * Embedded parts should get loaded and saved to the Native Krita Fileformat natively.
 */
class KisPartLayerImpl : public KisPartLayer {
    Q_OBJECT
    typedef KisPartLayer super;
public:
    KisPartLayerImpl(KisImageSP img, KisChildDoc * doc);
    virtual ~KisPartLayerImpl();

    virtual KisLayerSP clone() const;

    /// Called when the layer is made active
    virtual void activate() {}

    /// Called when another layer is made inactive
    virtual void deactivate() {}

    /// Returns the childDoc so that we can access the doc from other places, if need be (KisDoc)
    virtual KisChildDoc* childDoc() const { return m_doc; }

    void setDocType(const QString& type) { m_docType = type; }
    QString docType() const { return m_docType; }

    virtual void setX(Q_INT32 x);
    virtual void setY(Q_INT32 y);
    virtual Q_INT32 x() const { return m_doc->geometry() . x(); }
    virtual Q_INT32 y() const { return m_doc->geometry() . y(); } //m_paintLayer->y(); }
    virtual QRect extent() const { return m_doc->geometry(); }
    virtual QRect exactBounds() const { return m_doc->geometry(); }

    virtual QImage createThumbnail(Q_INT32 w, Q_INT32 h);

    virtual bool accept(KisLayerVisitor& visitor) {
        return visitor.visit(this);
    }

    virtual KisPaintDeviceSP prepareProjection(KisPaintDeviceSP projection, const QRect& r);

    virtual void paintSelection(QImage &img, Q_INT32 x, Q_INT32 y, Q_INT32 w, Q_INT32 h);

    virtual bool saveToXML(QDomDocument doc, QDomElement elem);
private slots:
    /// Repaints our device with the data from the embedded part
    //void repaint();
    /// When we activate the embedding, we clear ourselves
    void childActivated(KoDocumentChild* child);
    void childDeactivated(bool activated);


private:
    // KisPaintLayerSP m_paintLayer;
    KisPaintDeviceSP m_cache;
    KoFrame * m_frame; // The widget that holds the editable view of the embedded part
    KisChildDoc * m_doc; // The sub-document
    QString m_docType;
    bool m_activated;
};

/**
 * Visitor that connects all partlayers in an image to a KisView's signals
 */
class KisConnectPartLayerVisitor : public KisLayerVisitor {
    KisImageSP m_img;
    KisView* m_view;
    bool m_connect; // connects, or disconnects signals
public:
    KisConnectPartLayerVisitor(KisImageSP img, KisView* view, bool mode);
    virtual ~KisConnectPartLayerVisitor() {}

    virtual bool visit(KisPaintLayer *layer);
    virtual bool visit(KisGroupLayer *layer);
    virtual bool visit(KisPartLayer *layer);
    virtual bool visit(KisAdjustmentLayer *layer);
};

#endif // _KIS_PART_LAYER_
