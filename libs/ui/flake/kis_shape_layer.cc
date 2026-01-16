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
#include <KoXmlWriter.h>
#include <KoSelection.h>
#include <KoShapeMoveCommand.h>
#include <KoShapeTransformCommand.h>

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
#include <kis_signal_auto_connection.h>
#include <QThread>
#include <QApplication>

#include "kis_layer_properties_icons.h"


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
        child->setResolution(m_xRes, m_yRes);
    }

    void remove(KoShape *child) override {
        KIS_SAFE_ASSERT_RECOVER_NOOP(inheritsTransform(child));
        if (inheritsTransform(child)) {
            QTransform parentTransform = q->absoluteTransformation();
            child->applyAbsoluteTransformation(parentTransform);
        }

        SimpleShapeContainerModel::remove(child);
    }

    void setResolution(qreal xRes, qreal yRes) {
        if (!qFuzzyCompare(m_xRes, xRes) || !qFuzzyCompare(m_yRes, yRes)) {
            m_xRes = xRes;
            m_yRes = yRes;
            Q_FOREACH (KoShape *shape, shapes()) {
                shape->setResolution(xRes, yRes);
            }
        }
    }

private:
    KisShapeLayer *q;
    qreal m_xRes{72.0};
    qreal m_yRes{72.0};
};


struct KisShapeLayer::Private
{
public:
    Private()
        : canvas(0)
        , controller(0)
        , x(0)
        , y(0)
        , isAntialiased(true)
         {}

    KisPaintDeviceSP paintDevice;
    KisShapeLayerCanvasBase *canvas;
    KoShapeControllerBase *controller;
    int x;
    int y;
    bool isAntialiased;
    KisSignalAutoConnectionsStore imageConnections;
};


KisShapeLayer::KisShapeLayer(KoShapeControllerBase* controller,
                             KisImageWSP image,
                             const QString &name,
                             quint8 opacity)
    : KisShapeLayer(controller, image, name, opacity,
                    [&] () { return new KisShapeLayerCanvas(image->colorSpace(), new KisDefaultBounds(image), this);})
{
}

KisShapeLayer::KisShapeLayer(const KisShapeLayer& rhs)
    : KisShapeLayer(rhs,
                    rhs.m_d->controller)
{
}

KisShapeLayer::KisShapeLayer(const KisShapeLayer& rhs, KoShapeControllerBase* controller)
    : KisShapeLayer(rhs,
                    controller,
                    [&] () {
                            const KisShapeLayerCanvas* shapeLayerCanvas = dynamic_cast<const KisShapeLayerCanvas*>(rhs.m_d->canvas);
                            KIS_ASSERT(shapeLayerCanvas);
                            return new KisShapeLayerCanvas(*shapeLayerCanvas, this);})
{
}

KisShapeLayer::KisShapeLayer(const KisShapeLayer& _rhs, KoShapeControllerBase* controller,
                             std::function<KisShapeLayerCanvasBase *()> canvasFactory)
        : KisExternalLayer(_rhs)
        , KoShapeLayer(new ShapeLayerContainerModel(this)) //no _rhs here otherwise both layer have the same KoShapeContainerModel
        , m_d(new Private())
{
    initShapeLayerImpl(controller, canvasFactory());
    m_d->isAntialiased = _rhs.m_d->isAntialiased;

    /**
     * The transformations of the added shapes are automatically merged into the transformation
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

KisShapeLayer::KisShapeLayer(const KisShapeLayer& baseTemplate, const QList<KoShape*> &newShapes)
        : KisExternalLayer(baseTemplate)
        , KoShapeLayer(new ShapeLayerContainerModel(this)) //no _merge here otherwise both layer have the same KoShapeContainerModel
        , m_d(new Private())
{
    // Make sure our new layer is visible otherwise the shapes cannot be painted.
    setVisible(true);

    m_d->isAntialiased = baseTemplate.m_d->isAntialiased;

    const KisShapeLayerCanvas* shapeLayerCanvas = dynamic_cast<const KisShapeLayerCanvas*>(baseTemplate.canvas());
    KIS_ASSERT(shapeLayerCanvas);
    initShapeLayerImpl(baseTemplate.m_d->controller, new KisShapeLayerCanvas(*shapeLayerCanvas, this));

    /**
     * With current implementation this matrix will always be an identity, because
     * we do not copy the transformation from any of the source layers. But we should
     * handle this anyway, to not be caught by this in the future.
     */
    const QTransform thisInvertedTransform = this->absoluteTransformation().inverted();

    QList<KoShape *> clonedShapes;

    // copy all the new shapes; we expect them to be sorted and homogenized

    const bool isSorted = std::is_sorted(newShapes.begin(), newShapes.end(), KoShape::compareShapeZIndex);
    KIS_SAFE_ASSERT_RECOVER_NOOP(isSorted);

    Q_FOREACH (KoShape *shape, newShapes) {
        KoShape *clonedShape = shape->cloneShape();
        KIS_SAFE_ASSERT_RECOVER(clonedShape) { continue; }
        clonedShape->setTransformation(shape->absoluteTransformation() * thisInvertedTransform);
        clonedShapes.append(clonedShape);
    }

    Q_FOREACH (KoShape *shape, clonedShapes) {
        addShape(shape);
    }
}

KisShapeLayer::KisShapeLayer(KoShapeControllerBase* controller,
                             KisImageWSP image,
                             const QString &name,
                             quint8 opacity,
                             std::function<KisShapeLayerCanvasBase *()> canvasFactory)
        : KisExternalLayer(image, name, opacity)
        , KoShapeLayer(new ShapeLayerContainerModel(this))
        , m_d(new Private())
{
    initShapeLayerImpl(controller, canvasFactory());
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
                                       KisShapeLayerCanvasBase *canvas)
{
    setSupportsLodMoves(false);
    setShapeId(KIS_SHAPE_LAYER_ID);

    KIS_SAFE_ASSERT_RECOVER_RETURN(canvas);

    m_d->paintDevice = canvas->projection();

    m_d->canvas = canvas;
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

    if (this->image()) {
        m_d->imageConnections.addUniqueConnection(this->image(), SIGNAL(sigResolutionChanged(double, double)), this, SLOT(slotImageResolutionChanged()));
        slotImageResolutionChanged();
    }
}

bool KisShapeLayer::allowAsChild(KisNodeSP node) const
{
    return node->inherits("KisMask");
}

void KisShapeLayer::setImage(KisImageWSP _image)
{
    m_d->imageConnections.clear();
    KisLayer::setImage(_image);
    m_d->canvas->setImage(_image);
    if (m_d->paintDevice) {
        m_d->paintDevice->setDefaultBounds(new KisDefaultBounds(_image));
    }
    if (_image) {
        m_d->imageConnections.addUniqueConnection(_image, SIGNAL(sigResolutionChanged(double, double)), this, SLOT(slotImageResolutionChanged()));
        slotImageResolutionChanged();
    }
}


KisBaseNode::PropertyList KisShapeLayer::sectionModelProperties() const
{
    KisBaseNode::PropertyList l = KisLayer::sectionModelProperties();

    l << KisLayerPropertiesIcons::getProperty(KisLayerPropertiesIcons::antialiased, antialiased());

    return l;
}

void KisShapeLayer::setSectionModelProperties(const KisBaseNode::PropertyList &properties)
{
    Q_FOREACH (const KisBaseNode::Property &property, properties) {
        if (property.name == i18n("Anti-aliasing")) {
            setAntialiased(property.state.toBool());
        }
    }

    KisLayer::setSectionModelProperties(properties);
}

KisLayerSP KisShapeLayer::tryCreateInternallyMergedLayerFromMutipleLayers(QList<KisLayerSP> layers)
{
    // NOTE: layers.first() may possibly point to `this`!

    QList<KisShapeLayer*> shapeLayers;
    Q_FOREACH(KisLayerSP layer, layers) {
        KisShapeLayer *shapeLayer = dynamic_cast<KisShapeLayer*>(layer.data());
        if (!shapeLayer) return nullptr;

        shapeLayers << shapeLayer;
    }

    QList<KoShape*> allShapes;
    QList<KoShapeReorderCommand::IndexedShape> allIndexedShapes;
    Q_FOREACH(KisShapeLayer *layer, shapeLayers) {
        QList<KoShape*> shapes = layer->shapes();
        std::sort(shapes.begin(), shapes.end(), KoShape::compareShapeZIndex);

        Q_FOREACH(KoShape *shape, shapes) {
            allShapes << shape;
            allIndexedShapes << shape;
        }
    }

    allIndexedShapes = KoShapeReorderCommand::homogenizeZIndexesLazy(allIndexedShapes);

    KoShapeReorderCommand cmd(allIndexedShapes);
    cmd.redo();

    KisLayerSP newLayer = new KisShapeLayer(*this, allShapes);

    newLayer->setCompositeOpId(this->compositeOpId());
    if (!this->channelFlags().isEmpty()) {
        newLayer->setChannelFlags(this->channelFlags());
    }

    newLayer->setCompositeOpId(this->compositeOpId());
    newLayer->setPinnedToTimeline(this->isPinnedToTimeline());
    newLayer->setColorLabelIndex(this->colorLabelIndex());

    return newLayer;
}

KisLayerSP KisShapeLayer::createMergedLayerTemplate(KisLayerSP prevLayer)
{
    KisLayerSP newLayer;

    KisShapeLayer *prevShape = dynamic_cast<KisShapeLayer*>(prevLayer.data());
    if (prevShape) {
        newLayer = tryCreateInternallyMergedLayerFromMutipleLayers({prevLayer, this});
        KIS_SAFE_ASSERT_RECOVER_NOOP(newLayer);
    }

    return newLayer ? newLayer : KisExternalLayer::createMergedLayerTemplate(prevLayer);
}

void KisShapeLayer::fillMergedLayerTemplate(KisLayerSP dstLayer, KisLayerSP prevLayer, bool skipPaintingThisLayer)
{
    if (!dynamic_cast<KisShapeLayer*>(dstLayer.data())) {
        KisLayer::fillMergedLayerTemplate(dstLayer, prevLayer, skipPaintingThisLayer);
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
    Q_EMIT sigMoveShapes(diff);

    // Save new value to satisfy LSP
    m_d->x = x;
}

void KisShapeLayer::setY(qint32 y)
{
    qint32 delta = y - this->y();
    QPointF diff = QPointF(0, m_d->canvas->viewConverter()->viewToDocumentY(delta));
    Q_EMIT sigMoveShapes(diff);

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

const KoViewConverter* KisShapeLayer::converter() const
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

QList<KoShape *> KisShapeLayer::createShapesFromSvg(QIODevice *device, const QString &baseXmlDir, const QRectF &rectInPixels, qreal resolutionPPI, KoDocumentResourceManager *resourceManager, bool loadingFromKra, QSizeF *fragmentSize, QStringList *warnings, QStringList *errors)
{

    QString errorMsg;
    int errorLine = 0;
    int errorColumn;

    QDomDocument doc = SvgParser::createDocumentFromSvg(device, &errorMsg, &errorLine, &errorColumn);
    if (doc.isNull()) {
        errKrita << "Parsing error in contents.svg! Aborting!" << Qt::endl
        << " In line: " << errorLine << ", column: " << errorColumn << Qt::endl
        << " Error message: " << errorMsg << Qt::endl;

        if (errors) {
            *errors << i18n("Parsing error in the main document at line %1, column %2\nError message: %3"
                         , errorLine , errorColumn , errorMsg);
        }
        return QList<KoShape*>();
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

    QList<KoShape *> result = parser.parseSvg(doc.documentElement(), fragmentSize);

    if (warnings) {
        *warnings = parser.warnings();
    }

    return result;
}


bool KisShapeLayer::saveLayer(KoStore * store) const
{
    // FIXME: we handle xRes() only!

    const QSizeF sizeInPx = image()->bounds().size();
    const QSizeF sizeInPt(sizeInPx.width() / image()->xRes(), sizeInPx.height() / image()->yRes());

    return saveShapesToStore(store, this->shapes(), sizeInPt);
}

bool KisShapeLayer::loadSvg(QIODevice *device, const QString &baseXmlDir, QStringList *warnings)
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
                            &fragmentSize,
                            warnings);

    Q_FOREACH (KoShape *shape, shapes) {
        addShape(shape);
    }

    return true;
}

bool KisShapeLayer::loadLayer(KoStore* store, QStringList *warnings)
{
    if (!store) {
        warnKrita << "No store backend";
        return false;
    }

    if (store->open("content.svg")) {
        KoStoreDevice storeDev(store);
        storeDev.open(QIODevice::ReadOnly);

        loadSvg(&storeDev, "", warnings);

        store->close();

        return true;
    }

    return false;

}

void KisShapeLayer::resetCache(const KoColorSpace *colorSpace)
{
    m_d->canvas->resetCache(colorSpace);
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

    void undo() override
    {
        KIS_SAFE_ASSERT_RECOVER_NOOP(QThread::currentThread() != qApp->thread());
        m_blockingConnection.start(m_savedTransform);
    }

    void redo() override
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
     * We cannot transform shapes in the worker thread. Therefor we Q_EMIT blocking-queued
     * signal to transform them in the GUI thread and then return.
     */
    const KisImageViewConverter *converter = dynamic_cast<const KisImageViewConverter*>(this->converter());
    KIS_ASSERT(converter);
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

KisShapeLayerCanvasBase *KisShapeLayer::canvas() const
{
    return m_d->canvas;
}

void KisShapeLayer::slotImageResolutionChanged()
{
    ShapeLayerContainerModel *model = dynamic_cast<ShapeLayerContainerModel*>(this->model());
    KIS_SAFE_ASSERT_RECOVER_RETURN(model);
    if (this->image()) {
        model->setResolution(image()->xRes() * 72.0, image()->yRes() * 72.0);
    }
}


bool KisShapeLayer::antialiased() const
{
    return m_d->isAntialiased;
}

void KisShapeLayer::setAntialiased(const bool antialiased)
{
    const bool oldAntialiased = m_d->isAntialiased;

    if (antialiased != oldAntialiased) {
        m_d->isAntialiased = antialiased;
        // is it the best way to rerender the vector layer?
        if(m_d->canvas) m_d->canvas->resetCache(colorSpace());
    }
}
