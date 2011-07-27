/*
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
 *  Copyright (c) 2007 Boudewijn Rempt <boud@valdyas.org>
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

#include "kis_image.h"

#include <KoConfig.h> // WORDS_BIGENDIAN

#include <stdlib.h>
#include <math.h>

#include <QImage>
#include <QPainter>
#include <QSize>
#include <QDateTime>
#include <QRect>
#include <QRegion>

#include <klocale.h>

#include "KoUnit.h"
#include "KoColorSpaceRegistry.h"
#include "KoColor.h"
#include "KoColorConversionTransformation.h"
#include "KoColorProfile.h"

#include "recorder/kis_action_recorder.h"
#include "kis_adjustment_layer.h"
#include "kis_annotation.h"
#include "kis_background.h"
#include "kis_change_profile_visitor.h"
#include "kis_colorspace_convert_visitor.h"
#include "kis_count_visitor.h"
#include "kis_filter_strategy.h"
#include "kis_group_layer.h"
#include "commands/kis_image_commands.h"
#include "kis_iterators_pixel.h"
#include "kis_layer.h"
#include "kis_meta_data_merge_strategy_registry.h"
#include "kis_name_server.h"
#include "kis_paint_device.h"
#include "kis_paint_layer.h"
#include "kis_painter.h"
#include "kis_perspective_grid.h"
#include "kis_selection.h"
#include "kis_shear_visitor.h"
#include "kis_transaction.h"
#include "kis_transform_visitor.h"
#include "kis_types.h"
#include "kis_crop_visitor.h"
#include "kis_meta_data_merge_strategy.h"

#include "kis_image_config.h"
#include "kis_projection.h"
#include "kis_update_scheduler.h"


// #define SANITY_CHECKS

#ifdef SANITY_CHECKS
#define SANITY_CHECK_LOCKED(name)                                       \
    if(!locked()) qDebug() << "Locking policy failed:" << name          \
                           << "has been called without the image"       \
                              "being locked";
#else
#define SANITY_CHECK_LOCKED(name)
#endif


KisAbstractUpdateScheduler* createUpdateScheduler(KisImageWSP image)
{
    KisImageConfig config;

    if(config.useUpdateScheduler()) {
        dbgImage<<"Creating KisUpdateScheduler";
        return new KisUpdateScheduler(image);
    }
    else {
        dbgImage<<"Created KisProjection";
        return new KisProjection(image);
    }
}


class KisImage::KisImagePrivate
{
public:
    KisBackgroundSP  backgroundPattern;
    quint32 lockCount;
    bool sizeChangedWhileLocked;
    KisPerspectiveGrid* perspectiveGrid;

    qint32 width;
    qint32 height;

    double xres;
    double yres;

    KoUnit unit;

    const KoColorSpace * colorSpace;

    KisGroupLayerSP rootLayer; // The layers are contained in here
    QList<KisLayer*> dirtyLayers; // for thumbnails

    KisNameServer *nserver;
    KisUndoAdapter *adapter;
    KisActionRecorder *recorder;

    vKisAnnotationSP annotations;

    KisSelectionSP globalSelection;
    KisSelectionSP deselectedGlobalSelection;

    KisAbstractUpdateScheduler* projection;

    bool startProjection;
};

KisImage::KisImage(KisUndoAdapter *adapter, qint32 width, qint32 height, const KoColorSpace * colorSpace, const QString& name, bool startProjection)
        : QObject(0)
        , KisShared()
        , m_d(new KisImagePrivate())
{
    setObjectName(name);
    dbgImage << "creating" << name;
    m_d->startProjection = startProjection;
    init(adapter, width, height, colorSpace);
}

KisImage::KisImage(const KisImage& rhs)
        : QObject()
        , KisNodeFacade(rhs)
        , KisNodeGraphListener(rhs)
        , KisShared()
        , m_d(new KisImagePrivate())
{

    if (this != &rhs) {

        dbgImage << "copying" << objectName() << "from" << rhs.objectName();

        if (rhs.m_d->perspectiveGrid)
            m_d->perspectiveGrid = new KisPerspectiveGrid(*rhs.m_d->perspectiveGrid);
        else
            m_d->perspectiveGrid = 0;

        m_d->width = rhs.m_d->width;
        m_d->height = rhs.m_d->height;
        m_d->xres = rhs.m_d->xres;
        m_d->yres = rhs.m_d->yres;
        m_d->unit = rhs.m_d->unit;
        m_d->colorSpace = rhs.m_d->colorSpace;
        m_d->adapter = rhs.m_d->adapter;
        m_d->globalSelection = 0;
        m_d->deselectedGlobalSelection = 0;
        setRootLayer(static_cast<KisGroupLayer*>(rhs.m_d->rootLayer->clone().data()));
        m_d->annotations = rhs.m_d->annotations; // XXX the annotations would probably need to be deep-copied
        m_d->nserver = new KisNameServer(*rhs.m_d->nserver);
        m_d->startProjection = rhs.m_d->startProjection;
        Q_CHECK_PTR(m_d->nserver);

        m_d->projection = 0;
        if (m_d->startProjection) {
            m_d->projection = createUpdateScheduler(this);
        }
    }
}

KisImage::~KisImage()
{
    dbgImage << "deleting kisimage" << objectName();

    delete m_d->projection;
    delete m_d->perspectiveGrid;
    delete m_d->nserver;
    delete m_d;

    disconnect(); // in case Qt gets confused
}

void KisImage::aboutToAddANode(KisNode *parent, int index)
{
    SANITY_CHECK_LOCKED("aboutToAddANode");
    emit sigAboutToAddANode(parent, index);
}

void KisImage::nodeHasBeenAdded(KisNode *parent, int index)
{
    emit sigNodeHasBeenAdded(parent, index);
}

void KisImage::aboutToRemoveANode(KisNode *parent, int index)
{
    SANITY_CHECK_LOCKED("aboutToRemoveANode");
    emit sigAboutToRemoveANode(parent, index);
}

void KisImage::nodeHasBeenRemoved(KisNode *parent, int index)
{
    // XXX: Temporarily for compatibility
    emit sigNodeHasBeenRemoved(parent, index);
}

void KisImage::aboutToMoveNode(KisNode *parent, int oldIndex, int newIndex)
{
    SANITY_CHECK_LOCKED("aboutToMoveNode");
    emit sigAboutToMoveNode(parent, oldIndex, newIndex);
}

void KisImage::nodeHasBeenMoved(KisNode *parent, int oldIndex, int newIndex)
{
    emit sigNodeHasBeenMoved(parent, oldIndex, newIndex);
}

void KisImage::nodeChanged(KisNode* node)
{
    emit sigNodeChanged(node);
}

KisSelectionSP KisImage::globalSelection() const
{
    return m_d->globalSelection;
}

void KisImage::setGlobalSelection(KisSelectionSP globalSelection)
{
    if (globalSelection == 0)
        m_d->globalSelection = new KisSelection(new KisDefaultBounds(this));
    else
        m_d->globalSelection = globalSelection;
}

void KisImage::removeGlobalSelection()
{
    m_d->globalSelection = 0;
}

KisSelectionSP KisImage::deselectedGlobalSelection()
{
    return m_d->deselectedGlobalSelection;
}

void KisImage::setDeleselectedGlobalSelection(KisSelectionSP selection)
{
    m_d->deselectedGlobalSelection = selection;
}

KisBackgroundSP KisImage::backgroundPattern() const
{
    return m_d->backgroundPattern;
}

void KisImage::setBackgroundPattern(KisBackgroundSP background)
{
    if (background != m_d->backgroundPattern) {
        m_d->backgroundPattern = background;
        emit sigImageUpdated(bounds());
    }
}

QString KisImage::nextLayerName() const
{
    if (m_d->nserver->currentSeed() == 0) {
        m_d->nserver->number();
        return i18n("background");
    }

    return i18n("Layer %1", m_d->nserver->number());
}

void KisImage::rollBackLayerName()
{
    m_d->nserver->rollback();
}

void KisImage::init(KisUndoAdapter *adapter, qint32 width, qint32 height, const KoColorSpace *colorSpace)
{
    if (colorSpace == 0) {
        colorSpace = KoColorSpaceRegistry::instance()->rgb8();
    }

    m_d->lockCount = 0;
    m_d->sizeChangedWhileLocked = false;
    m_d->perspectiveGrid = 0;

    m_d->adapter = adapter;

    m_d->nserver = new KisNameServer(1);

    m_d->colorSpace = colorSpace;

    setRootLayer(new KisGroupLayer(this, "root", OPACITY_OPAQUE_U8));

    m_d->xres = 1.0;
    m_d->yres = 1.0;
    m_d->unit = KoUnit::Point;
    m_d->width = width;
    m_d->height = height;

    m_d->recorder = new KisActionRecorder(this);

    m_d->projection = 0;
    if (m_d->startProjection) {
        m_d->projection = createUpdateScheduler(this);
    }
}

bool KisImage::locked() const
{
    return m_d->lockCount != 0;
}

void KisImage::lock()
{
//  blockSignals(true);
    if (!locked()) {
        if (m_d->projection) {
            m_d->projection->lock();
        }
        m_d->sizeChangedWhileLocked = false;
    }
    m_d->lockCount++;
}

void KisImage::unlock()
{
    Q_ASSERT(locked());

    if (locked()) {
        m_d->lockCount--;

        if (m_d->lockCount == 0) {
            if (m_d->sizeChangedWhileLocked) {
                emit sigSizeChanged(m_d->width, m_d->height);
            }

            if (m_d->projection) {
                m_d->projection->unlock();
            }
        }
//      blockSignals(false);
    }
}

void KisImage::notifyLayerUpdated(KisLayerSP layer)
{
    // Add the layer to the list of layers that need to be
    // rescanned for the thumbnails in the layerbox
    KisLayer *l = layer.data();
    while (l) {
        if (!m_d->dirtyLayers.contains(l))
            m_d->dirtyLayers.append(l);
        l = dynamic_cast<KisLayer*>(l->parent().data());
    }
}

void KisImage::setSize(const QSize& size)
{
    m_d->width = size.width();
    m_d->height = size.height();
    emitSizeChanged();
}

void KisImage::resize(const QRect& newRect, bool cropLayers)
{
    if(newRect == bounds())
        return;

    QString macroName = cropLayers ? i18n("Crop Image") : i18n("Resize Image");
    m_d->adapter->beginMacro(macroName);
    m_d->adapter->addCommand(new KisImageLockCommand(KisImageWSP(this), true));
    m_d->adapter->addCommand(new KisImageResizeCommand(KisImageWSP(this), newRect.size()));

    if(cropLayers) {
        KisCropVisitor visitor(newRect, m_d->adapter);
        m_d->rootLayer->accept(visitor);
    }

    m_d->adapter->addCommand(new KisImageLockCommand(KisImageWSP(this), false));
    m_d->adapter->endMacro();

    setModified();
}

void KisImage::resize(qint32 w, qint32 h, qint32 x, qint32 y, bool cropLayers)
{
    resize(QRect(x, y, w, h), cropLayers);
}

void KisImage::resizeWithOffset(qint32 w, qint32 h, qint32 xOffset, qint32 yOffset)
{
    resize(QRect(-xOffset, -yOffset, w, h), true);
}

void KisImage::emitSizeChanged()
{
    if (!locked()) {
        emit sigSizeChanged(m_d->width, m_d->height);
    } else {
        m_d->sizeChangedWhileLocked = true;
    }
}


void KisImage::scale(double sx, double sy, KoUpdater *progress, KisFilterStrategy *filterStrategy, bool scaleOnlyShapes)
{
    // New image size. XXX: Pass along to discourage rounding errors?
    qint32 w, h;
    w = (qint32)((width() * sx) + 0.5);
    h = (qint32)((height() * sy) + 0.5);

    QSize newSize(w, h);
    if(newSize == size()) return;

    m_d->adapter->beginMacro(i18n("Scale Image"));
    m_d->adapter->addCommand(new KisImageLockCommand(KisImageWSP(this), true));

    if(!scaleOnlyShapes) {
        m_d->adapter->addCommand(new KisImageResizeCommand(KisImageWSP(this), newSize));
    }

    KisTransformVisitor visitor(KisImageWSP(this), sx, sy, 0.0, 0.0, 0.0, 0, 0, progress, filterStrategy, scaleOnlyShapes);
    m_d->rootLayer->accept(visitor);

    m_d->adapter->addCommand(new KisImageLockCommand(KisImageWSP(this), false));
    m_d->adapter->endMacro();

    setModified();
}
void KisImage::rotate(double radians, KoUpdater *progress)
{
    qint32 w = width();
    qint32 h = height();
    qint32 tx = qint32((w * cos(radians) - h * sin(radians) - w) / 2 + 0.5);
    qint32 ty = qint32((h * cos(radians) + w * sin(radians) - h) / 2 + 0.5);
    w = (qint32)(width() * qAbs(cos(radians)) + height() * qAbs(sin(radians)) + 0.5);
    h = (qint32)(height() * qAbs(cos(radians)) + width() * qAbs(sin(radians)) + 0.5);

    tx -= (w - width()) / 2;
    ty -= (h - height()) / 2;

    m_d->adapter->beginMacro(i18n("Rotate Image"));
    m_d->adapter->addCommand(new KisImageLockCommand(KisImageWSP(this), true));
    m_d->adapter->addCommand(new KisImageResizeCommand(KisImageWSP(this), QSize(w,h)));

    KisFilterStrategy *filter = KisFilterStrategyRegistry::instance()->value("Triangle");

    KisTransformVisitor visitor(KisImageWSP(this), 1.0, 1.0, 0, 0, radians, -tx, -ty, progress, filter);
    m_d->rootLayer->accept(visitor);

    m_d->adapter->addCommand(new KisImageLockCommand(KisImageWSP(this), false));
    m_d->adapter->endMacro();

    setModified();
}

void KisImage::shear(double angleX, double angleY, KoUpdater *progress)
{
    const double pi = 3.1415926535897932385;

    //new image size
    qint32 w = width();
    qint32 h = height();


    if (angleX != 0 || angleY != 0) {
        double deltaY = height() * qAbs(tan(angleX * pi / 180) * tan(angleY * pi / 180));
        w = (qint32)(width() + qAbs(height() * tan(angleX * pi / 180)));
        //ugly fix for the problem of having two extra pixels if only a shear along one
        //axis is done.
        if (angleX == 0 || angleY == 0)
            h = (qint32)(height() + qAbs(w * tan(angleY * pi / 180)));
        else if (angleX > 0 && angleY > 0)
            h = (qint32)(height() + qAbs(w * tan(angleY * pi / 180)) - 2 * deltaY + 2);
        else if (angleX < 0 && angleY < 0)
            h = (qint32)(height() + qAbs(w * tan(angleY * pi / 180)) - 2 * deltaY + 2);
        else
            h = (qint32)(height() + qAbs(w * tan(angleY * pi / 180)));
    }

    QSize newSize(w, h);
    if(newSize == size()) return;

    m_d->adapter->beginMacro(i18n("Shear Image"));
    m_d->adapter->addCommand(new KisImageLockCommand(KisImageWSP(this), true));
    m_d->adapter->addCommand(new KisImageResizeCommand(KisImageWSP(this), newSize));

    KisShearVisitor visitor(angleX, angleY, progress);
    visitor.setUndoAdapter(m_d->adapter);
    rootLayer()->accept(visitor);

    m_d->adapter->addCommand(new KisImageLockCommand(KisImageWSP(this), false));
    m_d->adapter->endMacro();

    setModified();
}

void KisImage::convertImageColorSpace(const KoColorSpace *dstColorSpace, KoColorConversionTransformation::Intent renderingIntent)
{
    if (*m_d->colorSpace == *dstColorSpace) return;

    m_d->adapter->beginMacro(i18n("Convert Image Color Space"));
    m_d->adapter->addCommand(new KisImageLockCommand(KisImageWSP(this), true));
    m_d->adapter->addCommand(new KisImageSetProjectionColorSpaceCommand(KisImageWSP(this), dstColorSpace));

    KisColorSpaceConvertVisitor visitor(this, dstColorSpace, renderingIntent);
    m_d->rootLayer->accept(visitor);

    m_d->adapter->addCommand(new KisImageLockCommand(KisImageWSP(this), false));
    m_d->adapter->endMacro();

    setModified();
}

void KisImage::assignImageProfile(const KoColorProfile *profile)
{
    if(!profile) return;

    m_d->adapter->beginMacro(i18n("Assign Profile"));
    m_d->adapter->addCommand(new KisImageLockCommand(KisImageWSP(this), true));

    const KoColorSpace *dstCs = KoColorSpaceRegistry::instance()->colorSpace(colorSpace()->colorModelId().id(), colorSpace()->colorDepthId().id(), profile);
    const KoColorSpace *srcCs = colorSpace();

    KisChangeProfileVisitor visitor(srcCs, dstCs);
    m_d->rootLayer->accept(visitor);

    m_d->adapter->addCommand(new KisImageSetProjectionColorSpaceCommand(KisImageWSP(this), dstCs));
    m_d->adapter->addCommand(new KisImageLockCommand(KisImageWSP(this), false));
    m_d->adapter->endMacro();

    setModified();
    emit sigProfileChanged(profile);
}

void KisImage::convertProjectionColorSpace(const KoColorSpace *dstColorSpace)
{
    if (*m_d->colorSpace == *dstColorSpace) return;

    m_d->adapter->beginMacro(i18n("Convert Projection Color Space"));
    m_d->adapter->addCommand(new KisImageLockCommand(KisImageWSP(this), true));
    m_d->adapter->addCommand(new KisImageSetProjectionColorSpaceCommand(KisImageWSP(this), dstColorSpace));
    m_d->adapter->addCommand(new KisImageLockCommand(KisImageWSP(this), false));
    m_d->adapter->endMacro();

    setModified();
}

void KisImage::setProjectionColorSpace(const KoColorSpace * colorSpace)
{
    m_d->colorSpace = colorSpace;
    m_d->rootLayer->resetCache();
    emit sigColorSpaceChanged(colorSpace);
}

const KoColorSpace * KisImage::colorSpace() const
{
    return m_d->colorSpace;
}

const KoColorProfile * KisImage::profile() const
{
    return colorSpace()->profile();
}

double KisImage::xRes() const
{
    return m_d->xres;
}

double KisImage::yRes() const
{
    return m_d->yres;
}

void KisImage::setResolution(double xres, double yres)
{
    m_d->xres = xres;
    m_d->yres = yres;
    emit(sigResolutionChanged(xres, yres));
}

QPointF KisImage::documentToPixel(const QPointF &documentCoord) const
{
    return QPointF(documentCoord.x() * xRes(), documentCoord.y() * yRes());
}

QPoint KisImage::documentToIntPixel(const QPointF &documentCoord) const
{
    QPointF pixelCoord = documentToPixel(documentCoord);
    return QPoint((int)pixelCoord.x(), (int)pixelCoord.y());
}

QRectF KisImage::documentToPixel(const QRectF &documentRect) const
{
    return QRectF(documentToPixel(documentRect.topLeft()), documentToPixel(documentRect.bottomRight()));
}

QRect KisImage::documentToIntPixel(const QRectF &documentRect) const
{
    return documentToPixel(documentRect).toAlignedRect();
}

QPointF KisImage::pixelToDocument(const QPointF &pixelCoord) const
{
    return QPointF(pixelCoord.x() / xRes(), pixelCoord.y() / yRes());
}

QPointF KisImage::pixelToDocument(const QPoint &pixelCoord) const
{
    return QPointF((pixelCoord.x() + 0.5) / xRes(), (pixelCoord.y() + 0.5) / yRes());
}

QRectF KisImage::pixelToDocument(const QRectF &pixelCoord) const
{
    return QRectF(pixelToDocument(pixelCoord.topLeft()), pixelToDocument(pixelCoord.bottomRight()));
}

qint32 KisImage::width() const
{
    return m_d->width;
}

qint32 KisImage::height() const
{
    return m_d->height;
}

KisGroupLayerSP KisImage::rootLayer() const
{
    Q_ASSERT(m_d->rootLayer);
    return m_d->rootLayer;
}

KisPaintDeviceSP KisImage::projection()
{
    Q_ASSERT(m_d->rootLayer);
    KisPaintDeviceSP projection = m_d->rootLayer->projection();
    Q_ASSERT(projection);
    return projection;
}

qint32 KisImage::nlayers() const
{
    QStringList list;
    list << "KisLayer";

    KisCountVisitor visitor(list, KoProperties());
    m_d->rootLayer->accept(visitor);
    return visitor.count();
}

qint32 KisImage::nHiddenLayers() const
{
    QStringList list;
    list << "KisLayer";
    KoProperties properties;
    properties.setProperty("visible", false);
    KisCountVisitor visitor(list, properties);
    m_d->rootLayer->accept(visitor);

    return visitor.count();
}

QRect KisImage::realNodeExtent(KisNodeSP rootNode, QRect currentRect)
{
    KisNodeSP node = rootNode->firstChild();

    while(node) {
        currentRect |= realNodeExtent(node, currentRect);
        node = node->nextSibling();
    }

    // TODO: it would be better to count up changeRect inside
    // node's extent() method
    currentRect |= rootNode->changeRect(rootNode->extent());

    return currentRect;
}

void KisImage::refreshHiddenArea(KisNodeSP rootNode, const QRect &preparedArea)
{
    QRect realNodeRect = realNodeExtent(rootNode);
    if(!preparedArea.contains(realNodeRect)) {

        QRegion dirtyRegion = realNodeRect;
        dirtyRegion -= preparedArea;

        foreach(const QRect &rc, dirtyRegion.rects()) {
            refreshGraph(rootNode, rc, realNodeRect);
        }
    }
}

void KisImage::flatten()
{
    KisGroupLayerSP oldRootLayer = m_d->rootLayer;
    KisGroupLayerSP newRootLayer =
        new KisGroupLayer(this, "root", OPACITY_OPAQUE_U8);

    refreshHiddenArea(oldRootLayer, bounds());

    lock();
    KisPaintDeviceSP projectionCopy =
        new KisPaintDevice(*oldRootLayer->projection());
    unlock();

    KisPaintLayerSP flattenLayer =
        new KisPaintLayer(this, nextLayerName(), OPACITY_OPAQUE_U8, projectionCopy);
    Q_CHECK_PTR(flattenLayer);

    addNode(flattenLayer, newRootLayer, 0);

    m_d->adapter->beginMacro(i18n("Flatten Image"));
    // NOTE: KisImageChangeLayersCommand performs all the locking for us
    m_d->adapter->addCommand(new KisImageChangeLayersCommand(KisImageWSP(this), oldRootLayer, newRootLayer, ""));
    m_d->adapter->endMacro();

    setModified();
}

KisLayerSP KisImage::mergeDown(KisLayerSP layer, const KisMetaData::MergeStrategy* strategy)
{
    if(!layer->prevSibling()) return 0;

    // XXX: this breaks if we allow free mixing of masks and layers
    KisLayerSP prevLayer = dynamic_cast<KisLayer*>(layer->prevSibling().data());
    if (!prevLayer) return 0;


    refreshHiddenArea(layer, bounds());
    refreshHiddenArea(prevLayer, bounds());

    QRect layerProjectionExtent = layer->projection()->extent();
    QRect prevLayerProjectionExtent = prevLayer->projection()->extent();

    lock();
    KisPaintDeviceSP mergedDevice = new KisPaintDevice(*prevLayer->projection());
    unlock();

    KisPainter gc(mergedDevice);
    gc.setChannelFlags(layer->channelFlags());
    gc.setCompositeOp(mergedDevice->colorSpace()->compositeOp(layer->compositeOpId()));
    gc.setOpacity(layer->opacity());
    gc.bitBlt(layerProjectionExtent.topLeft(), layer->projection(), layerProjectionExtent);

    KisPaintLayerSP mergedLayer = new KisPaintLayer(this, prevLayer->name(), OPACITY_OPAQUE_U8, mergedDevice);
    Q_CHECK_PTR(mergedLayer);


    // Merge meta data
    QList<const KisMetaData::Store*> srcs;
    srcs.append(prevLayer->metaData());
    srcs.append(layer->metaData());
    QList<double> scores;
    int prevLayerArea = prevLayerProjectionExtent.width() * prevLayerProjectionExtent.height();
    int layerArea = layerProjectionExtent.width() * layerProjectionExtent.height();
    double norm = qMax(prevLayerArea, layerArea);
    scores.append(prevLayerArea / norm);
    scores.append(layerArea / norm);
    strategy->merge(mergedLayer->metaData(), srcs, scores);

    KisNodeSP parent = layer->parent(); // parent is set to null when the layer is removed from the node
    dbgImage << ppVar(parent);

    // XXX: merge the masks!
    // AAA: do you really think you need it? ;)

    // FIXME: "Merge Down"?
    undoAdapter()->beginMacro(i18n("Merge with Layer Below"));

    undoAdapter()->addCommand(new KisImageLayerAddCommand(this, mergedLayer, parent, layer));
    undoAdapter()->addCommand(new KisImageLayerRemoveCommand(this, prevLayer));
    undoAdapter()->addCommand(new KisImageLayerRemoveCommand(this, layer));

    undoAdapter()->endMacro();

    return mergedLayer;
}

KisLayerSP KisImage::flattenLayer(KisLayerSP layer)
{
    if (!layer->firstChild()) return layer;

    refreshHiddenArea(layer, bounds());

    lock();
    KisPaintDeviceSP mergedDevice = new KisPaintDevice(*layer->projection());
    unlock();

    KisPaintLayerSP newLayer = new KisPaintLayer(this, layer->name(), layer->opacity(), mergedDevice);
    newLayer->setCompositeOp(layer->compositeOp()->id());


    undoAdapter()->beginMacro(i18n("Flatten Layer"));
    undoAdapter()->addCommand(new KisImageLayerAddCommand(this, newLayer, layer->parent(), layer));

    KisNodeSP node = layer->firstChild();
    while (node) {
        undoAdapter()->addCommand(new KisImageLayerRemoveCommand(this, node));
        node = node->nextSibling();
    }
    undoAdapter()->addCommand(new KisImageLayerRemoveCommand(this, layer));


    QList<const KisMetaData::Store*> srcs;
    srcs.append(layer->metaData());

    const KisMetaData::MergeStrategy* strategy = KisMetaData::MergeStrategyRegistry::instance()->get("Smart");
    QList<double> scores;
    scores.append(1.0); //Just give some score, there only is one layer
    strategy->merge(newLayer->metaData(), srcs, scores);

    undoAdapter()->endMacro();

    return newLayer;
}


void KisImage::setModified()
{
    emit sigImageModified();
}

void KisImage::renderToPainter(qint32 srcX,
                               qint32 srcY,
                               qint32 dstX,
                               qint32 dstY,
                               qint32 width,
                               qint32 height,
                               QPainter &painter,
                               const KoColorProfile *  monitorProfile)
{
    QImage image = convertToQImage(srcX, srcY, width, height, monitorProfile);
    painter.drawImage(dstX, dstY, image, 0, 0, width, height);
}

QImage KisImage::convertToQImage(QRect imageRect,
                                 const KoColorProfile * profile)
{
    qint32 x;
    qint32 y;
    qint32 w;
    qint32 h;
    imageRect.getRect(&x, &y, &w, &h);
    return convertToQImage(x, y, w, h, profile);
}

QImage KisImage::convertToQImage(qint32 x,
                                 qint32 y,
                                 qint32 w,
                                 qint32 h,
                                 const KoColorProfile * profile)
{
    KisPaintDeviceSP dev = m_d->rootLayer->projection();
    if (!dev) return QImage();
    QImage image = dev->convertToQImage(const_cast<KoColorProfile*>(profile), x, y, w, h);

    if (m_d->backgroundPattern) {
        m_d->backgroundPattern->paintBackground(image, QRect(x, y, w, h));
    }
    if (!image.isNull()) {
#ifdef WORDS_BIGENDIAN
        uchar * data = image.bits();
        for (int i = 0; i < w * h; ++i) {
            uchar r, g, b, a;
            a = data[0];
            b = data[1];
            g = data[2];
            r = data[3];
            data[0] = r;
            data[1] = g;
            data[2] = b;
            data[3] = a;
            data += 4;
        }
#endif

        return image;
    }

    return QImage();
}



QImage KisImage::convertToQImage(const QRect& scaledRect, const QSize& scaledImageSize, const KoColorProfile *profile)
{

    if (scaledRect.isEmpty() || scaledImageSize.isEmpty()) {
        return QImage();
    }

    qint32 imageWidth = width();
    qint32 imageHeight = height();
    quint32 pixelSize = colorSpace()->pixelSize();

    double xScale = static_cast<double>(imageWidth) / scaledImageSize.width();
    double yScale = static_cast<double>(imageHeight) / scaledImageSize.height();

    QRect srcRect;

    srcRect.setLeft(static_cast<int>(scaledRect.left() * xScale));
    srcRect.setRight(static_cast<int>(ceil((scaledRect.right() + 1) * xScale)) - 1);
    srcRect.setTop(static_cast<int>(scaledRect.top() * yScale));
    srcRect.setBottom(static_cast<int>(ceil((scaledRect.bottom() + 1) * yScale)) - 1);

    KisPaintDeviceSP mergedImage = m_d->rootLayer->projection();
    quint8 *scaledImageData = new quint8[scaledRect.width() * scaledRect.height() * pixelSize];

    quint8 *imageRow = new quint8[srcRect.width() * pixelSize];
    const qint32 imageRowX = srcRect.x();

    for (qint32 y = 0; y < scaledRect.height(); ++y) {

        qint32 dstY = scaledRect.y() + y;
        qint32 dstX = scaledRect.x();
        qint32 srcY = (dstY * imageHeight) / scaledImageSize.height();

        mergedImage->readBytes(imageRow, imageRowX, srcY, srcRect.width(), 1);

        quint8 *dstPixel = scaledImageData + (y * scaledRect.width() * pixelSize);
        quint32 columnsRemaining = scaledRect.width();

        while (columnsRemaining > 0) {

            qint32 srcX = (dstX * imageWidth) / scaledImageSize.width();

            memcpy(dstPixel, imageRow + ((srcX - imageRowX) * pixelSize), pixelSize);

            ++dstX;
            dstPixel += pixelSize;
            --columnsRemaining;
        }
    }
    delete [] imageRow;

    QImage image = colorSpace()->convertToQImage(scaledImageData, scaledRect.width(), scaledRect.height(), const_cast<KoColorProfile*>(profile), KoColorConversionTransformation::IntentPerceptual);

    if (m_d->backgroundPattern) {
        m_d->backgroundPattern->paintBackground(image, scaledRect, scaledImageSize, QSize(imageWidth, imageHeight));
    }

    delete [] scaledImageData;

#ifdef __BIG_ENDIAN__
    uchar * data = image.bits();
    for (int i = 0; i < image.width() * image.height(); ++i) {
        uchar r, g, b, a;
        a = data[0];
        b = data[1];
        g = data[2];
        r = data[3];
        data[0] = r;
        data[1] = g;
        data[2] = b;
        data[3] = a;
        data += 4;
    }
#endif

    return image;
}


KisPaintDeviceSP KisImage::mergedImage()
{
    refreshGraph();
    return m_d->rootLayer->projection();
}

void KisImage::notifyLayersChanged()
{
    emit sigLayersChanged(rootLayer());
    emit sigPostLayersChanged(rootLayer());
}

QRect KisImage::bounds() const
{
    return QRect(0, 0, width(), height());
}


void KisImage::setUndoAdapter(KisUndoAdapter * adapter)
{
    m_d->adapter = adapter;
}


KisUndoAdapter* KisImage::undoAdapter() const
{
    return m_d->adapter;
}

KisActionRecorder* KisImage::actionRecorder() const
{
    return m_d->recorder;
}

void KisImage::setRootLayer(KisGroupLayerSP rootLayer)
{
    if(m_d->rootLayer)
        m_d->rootLayer->disconnect();

    m_d->rootLayer = rootLayer;
    m_d->rootLayer->disconnect();
    m_d->rootLayer->setGraphListener(this);
    setRoot(m_d->rootLayer.data());
}

void KisImage::addAnnotation(KisAnnotationSP annotation)
{
    // Find the icc annotation, if there is one
    vKisAnnotationSP_it it = m_d->annotations.begin();
    while (it != m_d->annotations.end()) {
        if ((*it)->type() == annotation->type()) {
            *it = annotation;
            return;
        }
        ++it;
    }
    m_d->annotations.push_back(annotation);
}

KisAnnotationSP KisImage::annotation(const QString& type)
{
    vKisAnnotationSP_it it = m_d->annotations.begin();
    while (it != m_d->annotations.end()) {
        if ((*it)->type() == type) {
            return *it;
        }
        ++it;
    }
    return KisAnnotationSP(0);
}

void KisImage::removeAnnotation(const QString& type)
{
    vKisAnnotationSP_it it = m_d->annotations.begin();
    while (it != m_d->annotations.end()) {
        if ((*it)->type() == type) {
            m_d->annotations.erase(it);
            return;
        }
        ++it;
    }
}

vKisAnnotationSP_it KisImage::beginAnnotations()
{
    const KoColorProfile * profile = colorSpace()->profile();
    KisAnnotationSP annotation;

    if (profile) {
#ifdef __GNUC__
#warning "KisImage::beginAnnotations: make it possible to save any profile, not just icc profiles."
#endif
#if 0
        // XXX we hardcode icc, this is correct for icc?
        // XXX productName(), or just "ICC Profile"?
        if (profile->valid() && profile->type() == "icc" && !profile->rawData().isEmpty()) {
                annotation = new  KisAnnotation("icc", profile->name(), profile->rawData());
            }
        }
#endif
    }

    if (annotation)
        addAnnotation(annotation);
    else
        removeAnnotation("icc");

    return m_d->annotations.begin();
}

vKisAnnotationSP_it KisImage::endAnnotations()
{
    return m_d->annotations.end();
}

void KisImage::notifyAboutToBeDeleted()
{
    emit sigAboutToBeDeleted();
}

KisPerspectiveGrid* KisImage::perspectiveGrid()
{
    if (m_d->perspectiveGrid == 0)
        m_d->perspectiveGrid = new KisPerspectiveGrid();
    return m_d->perspectiveGrid;
}

void KisImage::refreshGraph(KisNodeSP root)
{
    refreshGraph(root, bounds(), bounds());
}

void KisImage::refreshGraph(KisNodeSP root, const QRect &rc, const QRect &cropRect)
{
    if (!root) root = m_d->rootLayer;

    if (!locked() && m_d->projection) {
        m_d->projection->fullRefresh(root, rc, cropRect);
    }
}

void KisImage::slotProjectionUpdated(const QRect & rc)
{
    emit sigImageUpdated(rc);
}

void KisImage::updateProjection(KisNodeSP node, const QRect& rc)
{
    if (!locked() && m_d->projection) {
        dbgImage << "KisImage: requested and update for" << node->name() << rc;
        m_d->projection->updateProjection(node, rc, bounds());
    }
}

#include "kis_image.moc"

