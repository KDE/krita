/*
 *  SPDX-FileCopyrightText: 2006-2008 Boudewijn Rempt <boud@valdyas.org>
 *  SPDX-FileCopyrightText: 2007 Thomas Zander <zander@kde.org>
 *  SPDX-FileCopyrightText: 2009 Cyrille Berger <cberger@cberger.net>
 *  SPDX-FileCopyrightText: 2011 Jan Hambrecht <jaham@gmx.net>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
#include "kis_do_something_command.h"
#include <KisSafeBlockingQueueConnectionProxy.h>
#include <QThread>
#include <QApplication>


#include <SimpleShapeContainerModel.h>
class ShapeLayerContainerModel : public SimpleShapeContainerModel
{
public:
    ShapeLayerContainerModel(KisShapeLayer *parent)
        : q(parent)
    {
    }

    void add(KoShape *child) override {
        SimpleShapeContainerModel::add(child);

        /**
         * The shape is always added with the absolute transformation set appropriately.
         * Here we should just squeeze it into the layer's transformation.
         */
        KIS_SAFE_ASSERT_RECOVER_NOOP(inheritsTransform(child));
        if (inheritsTransform(child)) {
            QTransform parentTransform = q->absoluteTransformation();
            child->applyAbsoluteTransformation(parentTransform.inverted());
        }
    }

    void remove(KoShape *child) override {
        KIS_SAFE_ASSERT_RECOVER_NOOP(inheritsTransform(child));
        if (inheritsTransform(child)) {
            QTransform parentTransform = q->absoluteTransformation();
            child->applyAbsoluteTransformation(parentTransform);
        }

        SimpleShapeContainerModel::remove(child);
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
    : KisShapeLayer(controller, image, name, opacity, nullptr)
{
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
    initClonedShapeLayer(controller, _rhs.m_d->paintDevice, canvas);

    /**
     * The transformaitons of the added shapes are automatically merged into the transformation
     * of the layer, so we should apply this extra transform separately
     */
    const QTransform thisInvertedTransform = this->absoluteTransformation().inverted();

    m_d->canvas->shapeManager()->setUpdatesBlocked(true);

    Q_FOREACH (KoShape *shape, _rhs.shapes()) {
        KoShape *clonedShape = shape->cloneShape();
        KIS_SAFE_ASSERT_RECOVER(clonedShape) { continue; }
        clonedShape->setTransformation(shape->absoluteTransformation() * thisInvertedTransform);
        addShape(clonedShape);
    }

    m_d->canvas->shapeManager()->setUpdatesBlocked(false);
}

KisShapeLayer::KisShapeLayer(const KisShapeLayer& _rhs, const KisShapeLayer &_addShapes)
        : KisExternalLayer(_rhs)
        , KoShapeLayer(new ShapeLayerContainerModel(this)) //no _merge here otherwise both layer have the same KoShapeContainerModel
        , m_d(new Private())
{
    // Make sure our new layer is visible otherwise the shapes cannot be painted.
    setVisible(true);

    initNewShapeLayer(_rhs.m_d->controller,
                      _rhs.m_d->paintDevice->colorSpace(),
                      _rhs.m_d->paintDevice->defaultBounds());

    /**
     * With current implementation this matrix will always be an identity, because
     * we do not copy the transformation from any of the source layers. But we should
     * handle this anyway, to not be caught by this in the future.
     */
    const QTransform thisInvertedTransform = this->absoluteTransformation().inverted();

    QList<KoShape *> shapesAbove;
    QList<KoShape *> shapesBelow;

    // copy in _rhs's shapes
    Q_FOREACH (KoShape *shape, _rhs.shapes()) {
        KoShape *clonedShape = shape->cloneShape();
        KIS_SAFE_ASSERT_RECOVER(clonedShape) { continue; }
        clonedShape->setTransformation(shape->absoluteTransformation() * thisInvertedTransform);
        shapesBelow.append(clonedShape);
    }

    // copy in _addShapes's shapes
    Q_FOREACH (KoShape *shape, _addShapes.shapes()) {
        KoShape *clonedShape = shape->cloneShape();
        KIS_SAFE_ASSERT_RECOVER(clonedShape) { continue; }
        clonedShape->setTransformation(shape->absoluteTransformation() * thisInvertedTransform);
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
    const KoColorSpace *cs = 0;
    KisDefaultBoundsBaseSP bounds;

    if (image) {
        cs = image->colorSpace();
        bounds = new KisDefaultBounds(this->image());
    } else {
        /// assert will always fail, because it is
        /// a recovery code path
        KIS_SAFE_ASSERT_RECOVER_NOOP(image);

        cs = KoColorSpaceRegistry::instance()->rgb8();
        bounds = new KisDefaultBounds();
    }

    initNewShapeLayer(controller, cs, bounds, canvas);
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

void KisShapeLayer::initShapeLayerImpl(KoShapeControllerBase* controller,
                                   KisPaintDeviceSP newProjectionDevice,
                                   KisShapeLayerCanvasBase *overrideCanvas)
{
    setSupportsLodMoves(false);
    setShapeId(KIS_SHAPE_LAYER_ID);

    m_d->paintDevice = newProjectionDevice;

    if (!overrideCanvas) {
        KisShapeLayerCanvas *slCanvas = new KisShapeLayerCanvas(this, image());
        slCanvas->setProjection(m_d->paintDevice);
        overrideCanvas = slCanvas;
    }

    m_d->canvas = overrideCanvas;
    m_d->canvas->moveToThread(this->thread());
    m_d->controller = controller;

    m_d->canvas->shapeManager()->selection()->disconnect(this);

    connect(m_d->canvas->selectedShapesProxy(), SIGNAL(selectionChanged()), this, SIGNAL(selectionChanged()));
    connect(m_d->canvas->selectedShapesProxy(), SIGNAL(currentLayerChanged(const KoShapeLayer*)),
            this, SIGNAL(currentLayerChanged(const KoShapeLayer*)));

    connect(this, SIGNAL(sigMoveShapes(QPointF)), SLOT(slotMoveShapes(QPointF)));

    ShapeLayerContainerModel *model = dynamic_cast<ShapeLayerContainerModel*>(this->model());
    KIS_SAFE_ASSERT_RECOVER_RETURN(model);
    model->setAssociatedRootShapeManager(m_d->canvas->shapeManager());
}

void KisShapeLayer::initNewShapeLayer(KoShapeControllerBase *controller,
                                      const KoColorSpace *projectionColorSpace,
                                      KisDefaultBoundsBaseSP bounds,
                                      KisShapeLayerCanvasBase *overrideCanvas)
{
    KisPaintDeviceSP projection = new KisPaintDevice(projectionColorSpace);
    projection->setDefaultBounds(bounds);
    projection->setParentNode(this);

    initShapeLayerImpl(controller, projection, overrideCanvas);
}

void KisShapeLayer::initClonedShapeLayer(KoShapeControllerBase *controller, KisPaintDeviceSP copyFromProjection, KisShapeLayerCanvasBase *overrideCanvas)
{
    KisPaintDeviceSP projection = new KisPaintDevice(*copyFromProjection);
    initShapeLayerImpl(controller, projection, overrideCanvas);
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
    Q_UNUSED(parent);
    KIS_ASSERT_RECOVER_RETURN(0);
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

QRect KisShapeLayer::theoreticalBoundingRect() const
{
    return kisGrowRect(m_d->canvas->viewConverter()->documentToView(this->boundingRect()).toAlignedRect(), 1);
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

void KisShapeLayer::slotTransformShapes(const QTransform &newTransform)
{
    KoShapeTransformCommand cmd({this}, {transformation()}, {newTransform});
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

KoSelectedShapesProxy *KisShapeLayer::selectedShapesProxy()
{
    return m_d->canvas->selectedShapesProxy();
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

QList<KoShape *> KisShapeLayer::createShapesFromSvg(QIODevice *device, const QString &baseXmlDir, const QRectF &rectInPixels, qreal resolutionPPI, KoDocumentResourceManager *resourceManager, bool loadingFromKra, QSizeF *fragmentSize)
{

    QString errorMsg;
    int errorLine = 0;
    int errorColumn;

    QDomDocument doc = SvgParser::createDocumentFromSvg(device, &errorMsg, &errorLine, &errorColumn);
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

    if (loadingFromKra) {
        /**
         * We set default text version to 1, it will make all the
         * files not having an explicit krita:textVersion tag load
         * as "legacy" files with the bug.
         *
         * The tag is not needed when loading from pure SVG, because
         * most probably they were not saved by a buggy Krita version.
         */
        parser.setDefaultKraTextVersion(1);
    }

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
                            true,
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

    return false;

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

class TransformShapeLayerDeferred : public KUndo2Command
{
public:
    TransformShapeLayerDeferred(KisShapeLayer *shapeLayer, const QTransform &globalDocTransform)
        : m_shapeLayer(shapeLayer),
          m_globalDocTransform(globalDocTransform),
          m_blockingConnection(std::bind(&KisShapeLayer::slotTransformShapes, shapeLayer, std::placeholders::_1))
    {
    }

    void undo()
    {
        KIS_SAFE_ASSERT_RECOVER_NOOP(QThread::currentThread() != qApp->thread());
        m_blockingConnection.start(m_savedTransform);
    }

    void redo()
    {
        m_savedTransform = m_shapeLayer->transformation();

        const QTransform globalTransform = m_shapeLayer->absoluteTransformation();
        const QTransform localTransform = globalTransform * m_globalDocTransform * globalTransform.inverted();

        KIS_SAFE_ASSERT_RECOVER_NOOP(QThread::currentThread() != qApp->thread());
        m_blockingConnection.start(localTransform * m_savedTransform);
    }

private:
    KisShapeLayer *m_shapeLayer;
    QTransform m_globalDocTransform;
    QTransform m_savedTransform;
    KisSafeBlockingQueueConnectionProxy<QTransform> m_blockingConnection;
};


KUndo2Command* KisShapeLayer::transform(const QTransform &transform)
{
    QList<KoShape*> shapes = shapesToBeTransformed();
    if (shapes.isEmpty()) return 0;
    KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(shapes.size() == 1 && shapes.first() == this, 0);

    /**
     * We cannot transform shapes in the worker thread. Therefor we emit blocking-queued
     * signal to transform them in the GUI thread and then return.
     */
    KisImageViewConverter *converter = dynamic_cast<KisImageViewConverter*>(this->converter());
    QTransform docSpaceTransform = converter->documentToView() *
        transform * converter->viewToDocument();

    return new TransformShapeLayerDeferred(this, docSpaceTransform);
}

KUndo2Command *KisShapeLayer::setProfile(const KoColorProfile *profile)
{
    using namespace KisDoSomethingCommandOps;

    KUndo2Command *cmd = new KUndo2Command();
    new KisDoSomethingCommand<ResetOp, KisShapeLayer*>(this, false, cmd);
    m_d->paintDevice->setProfile(profile, cmd);
    new KisDoSomethingCommand<ResetOp, KisShapeLayer*>(this, true, cmd);

    return cmd;
}

KUndo2Command *KisShapeLayer::convertTo(const KoColorSpace *dstColorSpace, KoColorConversionTransformation::Intent renderingIntent, KoColorConversionTransformation::ConversionFlags conversionFlags)
{
    using namespace KisDoSomethingCommandOps;

    KUndo2Command *cmd = new KUndo2Command();
    new KisDoSomethingCommand<ResetOp, KisShapeLayer*>(this, false, cmd);
    m_d->paintDevice->convertTo(dstColorSpace, renderingIntent, conversionFlags, cmd);
    new KisDoSomethingCommand<ResetOp, KisShapeLayer*>(this, true, cmd);
    return cmd;
}

KoShapeControllerBase *KisShapeLayer::shapeController() const
{
    return m_d->controller;
}
