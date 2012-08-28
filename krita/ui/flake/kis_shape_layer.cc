/*
 *  Copyright (c) 2006-2008 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (c) 2007 Thomas Zander <zander@kde.org>
 *  Copyright (c) 2009 Cyrille Berger <cberger@cberger.net>
 *  Copyright (c) 2011 Jan Hambrecht <jaham@gmx.net>
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

#include "kis_shape_layer.h"

#include <QPainter>
#include <QPainterPath>
#include <QRect>
#include <QDomElement>
#include <QDomDocument>
#include <QString>
#include <QList>
#include <QMap>
#include <QDebug>
#include <kundo2command.h>
#include <QMimeData>

#include <ktemporaryfile.h>
#include <kdebug.h>

#include <KoIcon.h>
#include <KoElementReference.h>
#include <KoColorSpace.h>
#include <KoCompositeOp.h>
#include <KoDataCenterBase.h>
#include <KoDocument.h>
#include <KoEmbeddedDocumentSaver.h>
#include <KoGenStyle.h>
#include <KoImageCollection.h>
#include <KoOdf.h>
#include <KoOdfReadStore.h>
#include <KoOdfStylesReader.h>
#include <KoOdfWriteStore.h>
#include <KoPageLayout.h>
#include <KoProperties.h>
#include <KoShapeContainer.h>
#include <KoShapeLayer.h>
#include <KoShapeGroup.h>
#include <KoShapeLoadingContext.h>
#include <KoShapeManager.h>
#include <KoShapeRegistry.h>
#include <KoShapeSavingContext.h>
#include <KoStore.h>
#include <KoShapeBasedDocumentBase.h>
#include <KoStoreDevice.h>
#include <KoViewConverter.h>
#include <KoXmlNS.h>
#include <KoXmlReader.h>
#include <KoXmlWriter.h>
#include <KoSelection.h>
#include <KoShapeMoveCommand.h>
#include <KoShapeTransformCommand.h>

#include <kis_types.h>
#include <kis_image.h>
#include <kis_paint_device.h>
#include "kis_shape_layer_canvas.h"
#include "kis_image_view_converter.h"
#include <kis_painter.h>
#include "kis_node_visitor.h"
#include "kis_processing_visitor.h"
#include "kis_effect_mask.h"

#include "kis_shape_layer_paste.h"


#include <SimpleShapeContainerModel.h>
class ShapeLayerContainerModel : public SimpleShapeContainerModel
{
public:
    ShapeLayerContainerModel(KisShapeLayer *parent)
        : q(parent)
{}

    void add(KoShape *child) {
        SimpleShapeContainerModel::add(child);
        q->shapeManager()->addShape(child);
    }

    void remove(KoShape *child) {
        q->shapeManager()->remove(child);
        SimpleShapeContainerModel::remove(child);
    }

private:
    KisShapeLayer *q;
};


struct KisShapeLayer::Private
{
public:
    Private()
        : converter(0)
        , canvas(0)
        , controller(0)
        , x(0)
        , y(0)
         {}

    KoViewConverter * converter;
    KisPaintDeviceSP paintDevice;
    KisShapeLayerCanvas * canvas;
    KoShapeBasedDocumentBase* controller;
    int x;
    int y;
};


KisShapeLayer::KisShapeLayer(KoShapeContainer * parent,
                             KoShapeBasedDocumentBase* controller,
                             KisImageWSP image,
                             const QString &name,
                             quint8 opacity)
    : KisExternalLayer(image, name, opacity),
      KoShapeLayer(new ShapeLayerContainerModel(this)),
      m_d(new Private())
{
    KoShapeContainer::setParent(parent);
    initShapeLayer(controller);
}

KisShapeLayer::KisShapeLayer(const KisShapeLayer& _rhs)
        : KisExternalLayer(_rhs)
        , KoShapeLayer(new ShapeLayerContainerModel(this)) //no _rhs here otherwise both layer have the same KoShapeContainerModel
        , m_d(new Private())
{
    // Make sure our new layer is visible otherwise the shapes cannot be painted.
    setVisible(true);

    KoShapeContainer::setParent(_rhs.KoShapeContainer::parent());
    initShapeLayer(_rhs.m_d->controller);

    KoShapeOdfSaveHelper saveHelper(_rhs.shapes());
    KoDrag drag;
    drag.setOdf(KoOdf::mimeType(KoOdf::Text), saveHelper);
    QMimeData* mimeData = drag.mimeData();

    Q_ASSERT(mimeData->hasFormat(KoOdf::mimeType(KoOdf::Text)));

    KisShapeLayerShapePaste paste(this, m_d->controller);
    bool success = paste.paste(KoOdf::Text, mimeData);
    Q_ASSERT(success);
    Q_UNUSED(success); // for release build
}

KisShapeLayer::~KisShapeLayer()
{
    /**
     * Small hack alert: we set the image to null to disable
     * updates those will be emitted on shape deletion
     */
    KisLayer::setImage(0);

    foreach(KoShape *shape, shapes()) {
        shape->setParent(0);
    }

    delete m_d->converter;
    delete m_d->canvas;
    delete m_d;
}

void KisShapeLayer::initShapeLayer(KoShapeBasedDocumentBase* controller)
{
    setShapeId(KIS_SHAPE_LAYER_ID);

    m_d->converter = new KisImageViewConverter(image());
    m_d->paintDevice = new KisPaintDevice(image()->colorSpace());
    m_d->canvas = new KisShapeLayerCanvas(this, m_d->converter);
    m_d->canvas->setProjection(m_d->paintDevice);
    m_d->controller = controller;

    m_d->canvas->shapeManager()->selection()->disconnect(this);

    connect(m_d->canvas->shapeManager()->selection(), SIGNAL(selectionChanged()), this, SIGNAL(selectionChanged()));
    connect(m_d->canvas->shapeManager()->selection(), SIGNAL(currentLayerChanged(const KoShapeLayer*)), this, SIGNAL(currentLayerChanged(const KoShapeLayer*)));

    connect(this, SIGNAL(sigMoveShapes(const QPointF&)), SLOT(slotMoveShapes(const QPointF&)));
}

bool KisShapeLayer::allowAsChild(KisNodeSP node) const
{
    return node->inherits("KisMask");
}

void KisShapeLayer::setImage(KisImageWSP _image)
{
    KisLayer::setImage(_image);
    delete m_d->converter;
    m_d->converter = new KisImageViewConverter(image());
    m_d->paintDevice = new KisPaintDevice(image()->colorSpace());
}

QIcon KisShapeLayer::icon() const
{
    return koIcon("bookmark-new");
}

KisPaintDeviceSP KisShapeLayer::original() const
{
    return m_d->paintDevice;
}

KisPaintDeviceSP KisShapeLayer::paintDevice() const
{
    return 0;
}

qint32 KisShapeLayer::x() const
{
    return m_d->x;
}

qint32 KisShapeLayer::y() const
{
    return m_d->y;
}

void KisShapeLayer::setX(qint32 x)
{
    qint32 delta = x - this->x();
    QPointF diff = QPointF(m_d->converter->viewToDocumentX(delta), 0);
    emit sigMoveShapes(diff);

    // Save new value to satisfy LSP
    m_d->x = x;
}

void KisShapeLayer::setY(qint32 y)
{
    qint32 delta = y - this->y();
    QPointF diff = QPointF(0, m_d->converter->viewToDocumentY(delta));
    emit sigMoveShapes(diff);

    // Save new value to satisfy LSP
    m_d->y = y;
}

void KisShapeLayer::slotMoveShapes(const QPointF &diff)
{
    QList<QPointF> prevPos;
    QList<QPointF> newPos;

    foreach (KoShape* shape, shapeManager()->shapes()) {
        QPointF pos = shape->position();
        prevPos << pos;
        newPos << pos + diff;
    }

    KoShapeMoveCommand cmd(shapeManager()->shapes(), prevPos, newPos);
    cmd.redo();
}

bool KisShapeLayer::accept(KisNodeVisitor& visitor)
{
    return visitor.visit(this);
}

void KisShapeLayer::accept(KisProcessingVisitor &visitor, KisUndoAdapter *undoAdapter)
{
    return visitor.visit(this, undoAdapter);
}

KoShapeManager* KisShapeLayer::shapeManager() const
{
    return m_d->canvas->shapeManager();
}

KoViewConverter* KisShapeLayer::converter() const
{
    return m_d->converter;
}

bool KisShapeLayer::visible(bool recursive) const
{
    return KisExternalLayer::visible(recursive);
}

void KisShapeLayer::setVisible(bool visible)
{
    KisExternalLayer::setVisible(visible);
}

bool KisShapeLayer::saveLayer(KoStore * store) const
{
    store->disallowNameExpansion();
    KoOdfWriteStore odfStore(store);
    KoXmlWriter* manifestWriter = odfStore.manifestWriter("application/vnd.oasis.opendocument.graphics");
    KoEmbeddedDocumentSaver embeddedSaver;
    KoDocument::SavingContext documentContext(odfStore, embeddedSaver);

    if (!store->open("content.xml"))
        return false;

    KoStoreDevice storeDev(store);
    KoXmlWriter * docWriter = KoOdfWriteStore::createOasisXmlWriter(&storeDev, "office:document-content");

    // for office:master-styles
    KTemporaryFile masterStyles;
    masterStyles.open();
    KoXmlWriter masterStylesTmpWriter(&masterStyles, 1);

    KoPageLayout page;
    page.format = KoPageFormat::defaultFormat();
    QRectF rc = boundingRect();
    page.width = rc.width();
    page.height = rc.height();
    if (page.width > page.height) {
        page.orientation = KoPageFormat::Landscape;
    } else {
        page.orientation = KoPageFormat::Portrait;
    }

    KoGenStyles mainStyles;
    KoGenStyle pageLayout = page.saveOdf();
    QString layoutName = mainStyles.insert(pageLayout, "PL");
    KoGenStyle masterPage(KoGenStyle::MasterPageStyle);
    masterPage.addAttribute("style:page-layout-name", layoutName);
    mainStyles.insert(masterPage, "Default", KoGenStyles::DontAddNumberToName);

    KTemporaryFile contentTmpFile;
    contentTmpFile.open();
    KoXmlWriter contentTmpWriter(&contentTmpFile, 1);

    contentTmpWriter.startElement("office:body");
    contentTmpWriter.startElement("office:drawing");

    KoShapeSavingContext shapeContext(contentTmpWriter, mainStyles, documentContext.embeddedSaver);

    shapeContext.xmlWriter().startElement("draw:page");
    shapeContext.xmlWriter().addAttribute("draw:name", "");

    KoElementReference elementRef("page", 1);
    elementRef.saveOdf(&shapeContext.xmlWriter(), KoElementReference::DrawId);

    shapeContext.xmlWriter().addAttribute("draw:master-page-name", "Default");

    saveOdf(shapeContext);

    shapeContext.xmlWriter().endElement(); // draw:page

    contentTmpWriter.endElement(); // office:drawing
    contentTmpWriter.endElement(); // office:body

    mainStyles.saveOdfStyles(KoGenStyles::DocumentAutomaticStyles, docWriter);

    // And now we can copy over the contents from the tempfile to the real one
    contentTmpFile.seek(0);
    docWriter->addCompleteElement(&contentTmpFile);

    docWriter->endElement(); // Root element
    docWriter->endDocument();
    delete docWriter;

    if (!store->close())
        return false;

    embeddedSaver.saveEmbeddedDocuments(documentContext);

    manifestWriter->addManifestEntry("content.xml", "text/xml");

    if (! mainStyles.saveOdfStylesDotXml(store, manifestWriter)) {
        return false;
    }

    manifestWriter->addManifestEntry("settings.xml", "text/xml");

    if (! shapeContext.saveDataCenter(documentContext.odfStore.store(),
                                      documentContext.odfStore.manifestWriter()))
        return false;

    // Write out manifest file
    if (!odfStore.closeManifestWriter()) {
        dbgImage << "closing manifestWriter failed";
        return false;
    }

    return true;
}

bool KisShapeLayer::loadLayer(KoStore* store)
{
    KoOdfReadStore odfStore(store);
    QString errorMessage;

    odfStore.loadAndParse(errorMessage);

    if (!errorMessage.isEmpty()) {
        warnKrita << errorMessage;
        return false;
    }

    KoXmlElement contents = odfStore.contentDoc().documentElement();

    //    qDebug() <<"Start loading OASIS document..." << contents.text();
    //    qDebug() <<"Start loading OASIS contents..." << contents.lastChild().localName();
    //    qDebug() <<"Start loading OASIS contents..." << contents.lastChild().namespaceURI();
    //    qDebug() <<"Start loading OASIS contents..." << contents.lastChild().isElement();

    KoXmlElement body(KoXml::namedItemNS(contents, KoXmlNS::office, "body"));

    if (body.isNull()) {
        //setErrorMessage( i18n( "Invalid OASIS document. No office:body tag found." ) );
        return false;
    }

    body = KoXml::namedItemNS(body, KoXmlNS::office, "drawing");
    if (body.isNull()) {
        //setErrorMessage( i18n( "Invalid OASIS document. No office:drawing tag found." ) );
        return false;
    }

    KoXmlElement page(KoXml::namedItemNS(body, KoXmlNS::draw, "page"));
    if (page.isNull()) {
        //setErrorMessage( i18n( "Invalid OASIS document. No draw:page tag found." ) );
        return false;
    }

    KoXmlElement * master = 0;
    if (odfStore.styles().masterPages().contains("Standard"))
        master = odfStore.styles().masterPages().value("Standard");
    else if (odfStore.styles().masterPages().contains("Default"))
        master = odfStore.styles().masterPages().value("Default");
    else if (! odfStore.styles().masterPages().empty())
        master = odfStore.styles().masterPages().begin().value();

    if (master) {
        const KoXmlElement *style = odfStore.styles().findStyle(
                                        master->attributeNS(KoXmlNS::style, "page-layout-name", QString()));
        KoPageLayout pageLayout;
        pageLayout.loadOdf(*style);
        setSize(QSizeF(pageLayout.width, pageLayout.height));
    }
    // We work fine without a master page

    KoOdfLoadingContext context(odfStore.styles(), odfStore.store());
    context.setManifestFile(QString("tar:/") + odfStore.store()->currentPath() + "META-INF/manifest.xml");
    KoShapeLoadingContext shapeContext(context, m_d->controller->resourceManager());


    KoXmlElement layerElement;
    forEachElement(layerElement, context.stylesReader().layerSet()) {
        // FIXME: investigate what is this
        //        KoShapeLayer * l = new KoShapeLayer();
        if (!loadOdf(layerElement, shapeContext)) {
            kWarning() << "Could not load vector layer!";
            return false;
        }
    }

    KoXmlElement child;
    forEachElement(child, page) {
        KoShape * shape = KoShapeRegistry::instance()->createShapeFromOdf(child, shapeContext);
        if (shape) {
            addShape(shape);
        }
    }

    return true;

}

void KisShapeLayer::resetCache()
{
    m_d->paintDevice->clear();

    QList<KoShape*> shapes = m_d->canvas->shapeManager()->shapes();
    foreach(const KoShape* shape, shapes) {
        shape->update();
    }
}

KUndo2Command* KisShapeLayer::crop(const QRect & rect) {
    QRectF croppedRect = m_d->converter->viewToDocument(rect);
    QList<KoShape*> shapes = m_d->canvas->shapeManager()->shapes();
    if(shapes.isEmpty())
        return 0;

    QList<QPointF> previousPositions;
    QList<QPointF> newPositions;
    foreach(const KoShape* shape, shapes) {
        previousPositions.append(shape->position());
        newPositions.append(shape->position() - croppedRect.topLeft());
    }
    return new KoShapeMoveCommand(shapes, previousPositions, newPositions);
}

KUndo2Command* KisShapeLayer::transform(const QTransform &transform) {
    QList<KoShape*> shapes = m_d->canvas->shapeManager()->shapes();
    if(shapes.isEmpty()) return 0;

    KisImageViewConverter *converter = dynamic_cast<KisImageViewConverter*>(m_d->converter);
    QTransform realTransform = converter->documentToView() *
        transform * converter->viewToDocument();

    QList<QTransform> oldTransformations;
    QList<QTransform> newTransformations;

    // this code won't work if there are shapes, that inherit the transformation from the parent container.
    // the chart and tree shapes are examples for that, but they aren't used in krita and there are no other shapes like that.
    foreach(const KoShape* shape, shapes) {
        QTransform oldTransform = shape->transformation();
        oldTransformations.append(oldTransform);
        if (dynamic_cast<const KoShapeGroup*>(shape)) {
            newTransformations.append(oldTransform);
        } else {
            QTransform globalTransform = shape->absoluteTransformation(0);
            QTransform localTransform = globalTransform * realTransform * globalTransform.inverted();
            newTransformations.append(localTransform*oldTransform);
        }
    }

    return new KoShapeTransformCommand(shapes, oldTransformations, newTransformations);
}

#include "kis_shape_layer.moc"
