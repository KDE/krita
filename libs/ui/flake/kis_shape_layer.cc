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
#include <kis_debug.h>
#include <kundo2command.h>
#include <commands_new/kis_node_move_command2.h>
#include <QMimeData>

#include <kis_icon.h>
#include <KoColorSpace.h>
#include <KoCompositeOp.h>
#include <KisDocument.h>
#include <KoUnit.h>
#include <KoOdf.h>
#include <KoOdfReadStore.h>
#include <KoOdfStylesReader.h>
#include <KoOdfLoadingContext.h>
#include <KoPageLayout.h>
#include <KoShapeContainer.h>
#include <KoShapeLayer.h>
#include <KoShapeGroup.h>
#include <KoShapeLoadingContext.h>
#include <KoShapeManager.h>
#include <KoSelectedShapesProxy.h>
#include <KoShapeRegistry.h>
#include <KoShapeSavingContext.h>
#include <KoStore.h>
#include <KoShapeControllerBase.h>
#include <KoStoreDevice.h>
#include <KoViewConverter.h>
#include <KoXmlNS.h>
#include <KoXmlReader.h>
#include <KoXmlWriter.h>
#include <KoSelection.h>
#include <KoShapeMoveCommand.h>
#include <KoShapeTransformCommand.h>
#include <KoShapeShadow.h>
#include <KoShapeShadowCommand.h>

#include "SvgWriter.h"
#include "SvgParser.h"

#include <kis_types.h>
#include <kis_image.h>
#include "kis_default_bounds.h"
#include <kis_paint_device.h>
#include "kis_shape_layer_canvas.h"
#include "kis_image_view_converter.h"
#include <kis_painter.h>
#include "kis_node_visitor.h"
#include "kis_processing_visitor.h"
#include "kis_effect_mask.h"
#include "commands/KoShapeReorderCommand.h"

#include <SimpleShapeContainerModel.h>
class ShapeLayerContainerModel : public SimpleShapeContainerModel
{
public:
    ShapeLayerContainerModel(KisShapeLayer *parent)
        : q(parent)
{}

    void add(KoShape *child) override {
        SimpleShapeContainerModel::add(child);

        /**
         * The shape is always added with the absolute transformation set appropriately.
         * Here we should just squeeze it into the layer's transformation.
         */
        KIS_SAFE_ASSERT_RECOVER_NOOP(inheritsTransform(child));
        if (inheritsTransform(child)) {
            QTransform parentTransform = q->absoluteTransformation(0);
            child->applyAbsoluteTransformation(parentTransform.inverted());
        }
    }

    void remove(KoShape *child) override {
        KIS_SAFE_ASSERT_RECOVER_NOOP(inheritsTransform(child));
        if (inheritsTransform(child)) {
            QTransform parentTransform = q->absoluteTransformation(0);
            child->applyAbsoluteTransformation(parentTransform);
        }

        SimpleShapeContainerModel::remove(child);
    }

    void shapeHasBeenAddedToHierarchy(KoShape *shape, KoShapeContainer *addedToSubtree) override {
        q->shapeManager()->addShape(shape);
        SimpleShapeContainerModel::shapeHasBeenAddedToHierarchy(shape, addedToSubtree);
    }

    void shapeToBeRemovedFromHierarchy(KoShape *shape, KoShapeContainer *removedFromSubtree) override {
        q->shapeManager()->remove(shape);
        SimpleShapeContainerModel::shapeToBeRemovedFromHierarchy(shape, removedFromSubtree);
    }

private:
    KisShapeLayer *q;
};


struct KisShapeLayer::Private
{
public:
    Private()
        : canvas(0)
        , controller(0)
        , x(0)
        , y(0)
         {}

    KisPaintDeviceSP paintDevice;
    KisShapeLayerCanvasBase * canvas;
    KoShapeControllerBase* controller;
    int x;
    int y;
};


KisShapeLayer::KisShapeLayer(KoShapeControllerBase* controller,
                             KisImageWSP image,
                             const QString &name,
                             quint8 opacity)
    : KisExternalLayer(image, name, opacity),
      KoShapeLayer(new ShapeLayerContainerModel(this)),
      m_d(new Private())
{
    initShapeLayer(controller);
}

KisShapeLayer::KisShapeLayer(const KisShapeLayer& rhs)
    : KisShapeLayer(rhs, rhs.m_d->controller)
{
}

KisShapeLayer::KisShapeLayer(const KisShapeLayer& _rhs, KoShapeControllerBase* controller, KisShapeLayerCanvasBase *canvas)
        : KisExternalLayer(_rhs)
        , KoShapeLayer(new ShapeLayerContainerModel(this)) //no _rhs here otherwise both layer have the same KoShapeContainerModel
        , m_d(new Private())
{
    // copy the projection to avoid extra round of updates!
    initShapeLayer(controller, _rhs.m_d->paintDevice, canvas);

    /**
     * The transformaitons of the added shapes are automatically merged into the transformation
     * of the layer, so we should apply this extra transform separately
     */
    const QTransform thisInvertedTransform = this->absoluteTransformation(0).inverted();

    m_d->canvas->setUpdatesBlocked(true);

    Q_FOREACH (KoShape *shape, _rhs.shapes()) {
        KoShape *clonedShape = shape->cloneShape();
        KIS_SAFE_ASSERT_RECOVER(clonedShape) { continue; }
        clonedShape->setTransformation(shape->absoluteTransformation(0) * thisInvertedTransform);
        addShape(clonedShape);
    }

    m_d->canvas->setUpdatesBlocked(false);
}

KisShapeLayer::KisShapeLayer(const KisShapeLayer& _rhs, const KisShapeLayer &_addShapes)
        : KisExternalLayer(_rhs)
        , KoShapeLayer(new ShapeLayerContainerModel(this)) //no _merge here otherwise both layer have the same KoShapeContainerModel
        , m_d(new Private())
{
    // Make sure our new layer is visible otherwise the shapes cannot be painted.
    setVisible(true);

    initShapeLayer(_rhs.m_d->controller);

    /**
     * With current implementation this matrix will always be an identity, because
     * we do not copy the transformation from any of the source layers. But we should
     * handle this anyway, to not be caught by this in the future.
     */
    const QTransform thisInvertedTransform = this->absoluteTransformation(0).inverted();

    QList<KoShape *> shapesAbove;
    QList<KoShape *> shapesBelow;

    // copy in _rhs's shapes
    Q_FOREACH (KoShape *shape, _rhs.shapes()) {
        KoShape *clonedShape = shape->cloneShape();
        KIS_SAFE_ASSERT_RECOVER(clonedShape) { continue; }
        clonedShape->setTransformation(shape->absoluteTransformation(0) * thisInvertedTransform);
        shapesBelow.append(clonedShape);
    }

    // copy in _addShapes's shapes
    Q_FOREACH (KoShape *shape, _addShapes.shapes()) {
        KoShape *clonedShape = shape->cloneShape();
        KIS_SAFE_ASSERT_RECOVER(clonedShape) { continue; }
        clonedShape->setTransformation(shape->absoluteTransformation(0) * thisInvertedTransform);
        shapesAbove.append(clonedShape);
    }

    QList<KoShapeReorderCommand::IndexedShape> shapes =
        KoShapeReorderCommand::mergeDownShapes(shapesBelow, shapesAbove);
    KoShapeReorderCommand cmd(shapes);
    cmd.redo();

    Q_FOREACH (KoShape *shape, shapesBelow + shapesAbove) {
        addShape(shape);
    }
}

KisShapeLayer::KisShapeLayer(KoShapeControllerBase* controller,
                             KisImageWSP image,
                             const QString &name,
                             quint8 opacity,
                             KisShapeLayerCanvasBase *canvas)
        : KisExternalLayer(image, name, opacity)
        , KoShapeLayer(new ShapeLayerContainerModel(this))
        , m_d(new Private())
{
    initShapeLayer(controller, nullptr, canvas);
}

KisShapeLayer::~KisShapeLayer()
{
    /**
     * Small hack alert: we should avoid updates on shape deletion
     */
    m_d->canvas->prepareForDestroying();

    Q_FOREACH (KoShape *shape, shapes()) {
        shape->setParent(0);
        delete shape;
    }

    delete m_d->canvas;
    delete m_d;
}

void KisShapeLayer::initShapeLayer(KoShapeControllerBase* controller, KisPaintDeviceSP copyFromProjection, KisShapeLayerCanvasBase *canvas)
{
    setSupportsLodMoves(false);
    setShapeId(KIS_SHAPE_LAYER_ID);

    KIS_ASSERT_RECOVER_NOOP(this->image());

    if (!copyFromProjection) {
        m_d->paintDevice = new KisPaintDevice(image()->colorSpace());
        m_d->paintDevice->setDefaultBounds(new KisDefaultBounds(this->image()));
        m_d->paintDevice->setParentNode(this);
    } else {
        m_d->paintDevice = new KisPaintDevice(*copyFromProjection);
    }

    if (!canvas) {
        auto *slCanvas = new KisShapeLayerCanvas(this, image());
        slCanvas->setProjection(m_d->paintDevice);
        canvas = slCanvas;
    }

    m_d->canvas = canvas;
    m_d->canvas->moveToThread(this->thread());
    m_d->controller = controller;

    m_d->canvas->shapeManager()->selection()->disconnect(this);

    connect(m_d->canvas->selectedShapesProxy(), SIGNAL(selectionChanged()), this, SIGNAL(selectionChanged()));
    connect(m_d->canvas->selectedShapesProxy(), SIGNAL(currentLayerChanged(const KoShapeLayer*)),
            this, SIGNAL(currentLayerChanged(const KoShapeLayer*)));

    connect(this, SIGNAL(sigMoveShapes(QPointF)), SLOT(slotMoveShapes(QPointF)));
}

bool KisShapeLayer::allowAsChild(KisNodeSP node) const
{
    return node->inherits("KisMask");
}

void KisShapeLayer::setImage(KisImageWSP _image)
{
    KisLayer::setImage(_image);
    m_d->canvas->setImage(_image);
    m_d->paintDevice->convertTo(_image->colorSpace());
    m_d->paintDevice->setDefaultBounds(new KisDefaultBounds(_image));
}

KisLayerSP KisShapeLayer::createMergedLayerTemplate(KisLayerSP prevLayer)
{
    KisShapeLayer *prevShape = dynamic_cast<KisShapeLayer*>(prevLayer.data());

    if (prevShape)
        return new KisShapeLayer(*prevShape, *this);
    else
        return KisExternalLayer::createMergedLayerTemplate(prevLayer);
}

void KisShapeLayer::fillMergedLayerTemplate(KisLayerSP dstLayer, KisLayerSP prevLayer)
{
    if (!dynamic_cast<KisShapeLayer*>(dstLayer.data())) {
        KisLayer::fillMergedLayerTemplate(dstLayer, prevLayer);
    }
}

void KisShapeLayer::setParent(KoShapeContainer *parent)
{
    Q_UNUSED(parent)
    KIS_ASSERT_RECOVER_RETURN(0)
}

QIcon KisShapeLayer::icon() const
{
    return KisIconUtils::loadIcon("vectorLayer");
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
    QPointF diff = QPointF(m_d->canvas->viewConverter()->viewToDocumentX(delta), 0);
    emit sigMoveShapes(diff);

    // Save new value to satisfy LSP
    m_d->x = x;
}

void KisShapeLayer::setY(qint32 y)
{
    qint32 delta = y - this->y();
    QPointF diff = QPointF(0, m_d->canvas->viewConverter()->viewToDocumentY(delta));
    emit sigMoveShapes(diff);

    // Save new value to satisfy LSP
    m_d->y = y;
}
namespace {
void filterTransformableShapes(QList<KoShape*> &shapes)
{
    auto it = shapes.begin();
    while (it != shapes.end()) {
        if (shapes.size() == 1) break;

        if ((*it)->inheritsTransformFromAny(shapes)) {
            it = shapes.erase(it);
        } else {
            ++it;
        }
    }
}
}

QList<KoShape *> KisShapeLayer::shapesToBeTransformed()
{
    QList<KoShape*> shapes = shapeManager()->shapes();

    // We expect that **all** the shapes inherit the transform from its parent

    // SANITY_CHECK: we expect all the shapes inside the
    //               shape layer to inherit transform!
    Q_FOREACH (KoShape *shape, shapes) {
        if (shape->parent()) {
            KIS_SAFE_ASSERT_RECOVER(shape->parent()->inheritsTransform(shape)) {
                break;
            }
        }
    }

    shapes << this;
    filterTransformableShapes(shapes);
    return shapes;
}

void KisShapeLayer::slotMoveShapes(const QPointF &diff)
{
    QList<KoShape*> shapes = shapesToBeTransformed();
    if (shapes.isEmpty()) return;

    KoShapeMoveCommand cmd(shapes, diff);
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
    return m_d->canvas->viewConverter();
}

bool KisShapeLayer::visible(bool recursive) const
{
    return KisExternalLayer::visible(recursive);
}

void KisShapeLayer::setVisible(bool visible, bool isLoading)
{
    const bool oldVisible = this->visible(false);

    KoShapeLayer::setVisible(visible);
    KisExternalLayer::setVisible(visible, isLoading);

    if (visible && !oldVisible &&
        m_d->canvas->hasChangedWhileBeingInvisible()) {

        m_d->canvas->rerenderAfterBeingInvisible();
    }
}

void KisShapeLayer::setUserLocked(bool value)
{
    KoShapeLayer::setGeometryProtected(value);
    KisExternalLayer::setUserLocked(value);
}

bool KisShapeLayer::isShapeEditable(bool recursive) const
{
    return KoShapeLayer::isShapeEditable(recursive) && isEditable(true);
}

// we do not override KoShape::setGeometryProtected() as we consider
// the user not being able to access the layer shape from Krita UI!

void KisShapeLayer::forceUpdateTimedNode()
{
    m_d->canvas->forceRepaint();
}

bool KisShapeLayer::hasPendingTimedUpdates() const
{
    return m_d->canvas->hasPendingUpdates();
}

void KisShapeLayer::forceUpdateHiddenAreaOnOriginal()
{
    m_d->canvas->forceRepaintWithHiddenAreas();
}

bool KisShapeLayer::saveShapesToStore(KoStore *store, QList<KoShape *> shapes, const QSizeF &sizeInPt)
{
    if (!store->open("content.svg")) {
        return false;
    }

    KoStoreDevice storeDev(store);
    storeDev.open(QIODevice::WriteOnly);

    std::sort(shapes.begin(), shapes.end(), KoShape::compareShapeZIndex);

    SvgWriter writer(shapes);
    writer.save(storeDev, sizeInPt);

    if (!store->close()) {
        return false;
    }

    return true;
}

QList<KoShape *> KisShapeLayer::createShapesFromSvg(QIODevice *device, const QString &baseXmlDir, const QRectF &rectInPixels, qreal resolutionPPI, KoDocumentResourceManager *resourceManager, QSizeF *fragmentSize)
{

    QString errorMsg;
    int errorLine = 0;
    int errorColumn;

    KoXmlDocument doc = SvgParser::createDocumentFromSvg(device, &errorMsg, &errorLine, &errorColumn);
    if (doc.isNull()) {
        errKrita << "Parsing error in " << "contents.svg" << "! Aborting!" << endl
        << " In line: " << errorLine << ", column: " << errorColumn << endl
        << " Error message: " << errorMsg << endl;
        errKrita << i18n("Parsing error in the main document at line %1, column %2\nError message: %3"
                         , errorLine , errorColumn , errorMsg);
    }

    SvgParser parser(resourceManager);
    parser.setXmlBaseDir(baseXmlDir);
    parser.setResolution(rectInPixels /* px */, resolutionPPI /* ppi */);
    return parser.parseSvg(doc.documentElement(), fragmentSize);
}


bool KisShapeLayer::saveLayer(KoStore * store) const
{
    // FIXME: we handle xRes() only!

    const QSizeF sizeInPx = image()->bounds().size();
    const QSizeF sizeInPt(sizeInPx.width() / image()->xRes(), sizeInPx.height() / image()->yRes());

    return saveShapesToStore(store, this->shapes(), sizeInPt);
}

bool KisShapeLayer::loadSvg(QIODevice *device, const QString &baseXmlDir)
{
    QSizeF fragmentSize; // unused!
    KisImageSP image = this->image();

    // FIXME: we handle xRes() only!
    KIS_SAFE_ASSERT_RECOVER_NOOP(qFuzzyCompare(image->xRes(), image->yRes()));
    const qreal resolutionPPI = 72.0 * image->xRes();

    QList<KoShape*> shapes =
        createShapesFromSvg(device, baseXmlDir,
                            image->bounds(), resolutionPPI,
                            m_d->controller->resourceManager(),
                            &fragmentSize);

    Q_FOREACH (KoShape *shape, shapes) {
        addShape(shape);
    }

    return true;
}

bool KisShapeLayer::loadLayer(KoStore* store)
{
    if (!store) {
        warnKrita << i18n("No store backend");
        return false;
    }

    if (store->open("content.svg")) {
        KoStoreDevice storeDev(store);
        storeDev.open(QIODevice::ReadOnly);

        loadSvg(&storeDev, "");

        store->close();

        return true;
    }

    KoOdfReadStore odfStore(store);
    QString errorMessage;

    odfStore.loadAndParse(errorMessage);

    if (!errorMessage.isEmpty()) {
        warnKrita << errorMessage;
        return false;
    }

    KoXmlElement contents = odfStore.contentDoc().documentElement();

    //    dbgKrita <<"Start loading OASIS document..." << contents.text();
    //    dbgKrita <<"Start loading OASIS contents..." << contents.lastChild().localName();
    //    dbgKrita <<"Start loading OASIS contents..." << contents.lastChild().namespaceURI();
    //    dbgKrita <<"Start loading OASIS contents..." << contents.lastChild().isElement();

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
            dbgKrita << "Could not load vector layer!";
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
    m_d->canvas->resetCache();
}

KUndo2Command* KisShapeLayer::crop(const QRect & rect)
{
    QPoint oldPos(x(), y());
    QPoint newPos = oldPos - rect.topLeft();

    return new KisNodeMoveCommand2(this, oldPos, newPos);
}

KUndo2Command* KisShapeLayer::transform(const QTransform &transform) {
    QList<KoShape*> shapes = shapesToBeTransformed();
    if (shapes.isEmpty()) return 0;

    KisImageViewConverter *converter = dynamic_cast<KisImageViewConverter*>(this->converter());
    QTransform realTransform = converter->documentToView() *
        transform * converter->viewToDocument();

    QList<QTransform> oldTransformations;
    QList<QTransform> newTransformations;

    QList<KoShapeShadow*> newShadows;
    const qreal transformBaseScale = KoUnit::approxTransformScale(transform);

    Q_FOREACH (const KoShape* shape, shapes) {
        QTransform oldTransform = shape->transformation();
        oldTransformations.append(oldTransform);

        QTransform globalTransform = shape->absoluteTransformation(0);
        QTransform localTransform = globalTransform * realTransform * globalTransform.inverted();
        newTransformations.append(localTransform * oldTransform);

        KoShapeShadow *shadow = 0;

        if (shape->shadow()) {
            shadow = new KoShapeShadow(*shape->shadow());
            shadow->setOffset(transformBaseScale * shadow->offset());
            shadow->setBlur(transformBaseScale * shadow->blur());
        }

        newShadows.append(shadow);

    }

    KUndo2Command *parentCommand = new KUndo2Command();
    new KoShapeTransformCommand(shapes,
                                oldTransformations,
                                newTransformations,
                                parentCommand);

    new KoShapeShadowCommand(shapes,
                             newShadows,
                             parentCommand);

    return parentCommand;
}

KoShapeControllerBase *KisShapeLayer::shapeController() const
{
    return m_d->controller;
}
