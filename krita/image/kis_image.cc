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
#include "kis_meta_data_merge_strategy.h"

#include "kis_image_config.h"
#include "kis_update_scheduler.h"
#include "kis_image_signal_router.h"

#include "kis_undo_stores.h"
#include "kis_legacy_undo_adapter.h"
#include "kis_post_execution_undo_adapter.h"

#include "kis_processing_applicator.h"
#include "processing/kis_crop_processing_visitor.h"
#include "processing/kis_transform_processing_visitor.h"
#include "commands_new/kis_image_resize_command.h"
#include "commands_new/kis_image_set_resolution_command.h"
#include "kis_composite_progress_proxy.h"


// #define SANITY_CHECKS

#ifdef SANITY_CHECKS
#define SANITY_CHECK_LOCKED(name)                                       \
    if(!locked()) qDebug() << "Locking policy failed:" << name          \
                           << "has been called without the image"       \
                              "being locked";
#else
#define SANITY_CHECK_LOCKED(name)
#endif


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

    KisUndoStore *undoStore;
    KisUndoAdapter *legacyUndoAdapter;
    KisPostExecutionUndoAdapter *postExecutionUndoAdapter;

    KisActionRecorder *recorder;

    vKisAnnotationSP annotations;

    KisSelectionSP globalSelection;
    KisSelectionSP deselectedGlobalSelection;

    QAtomicInt disableUIUpdateSignals;
    KisImageSignalRouter *signalRouter;
    KisUpdateScheduler *scheduler;

    KisCompositeProgressProxy *compositeProgressProxy;

    bool startProjection;
};

KisImage::KisImage(KisUndoStore *undoStore, qint32 width, qint32 height, const KoColorSpace * colorSpace, const QString& name, bool startProjection)
        : QObject(0)
        , KisShared()
        , m_d(new KisImagePrivate())
{
    setObjectName(name);
    dbgImage << "creating" << name;
    m_d->startProjection = startProjection;
    init(undoStore, width, height, colorSpace);
}

KisImage::~KisImage()
{
    dbgImage << "deleting kisimage" << objectName();

    /**
     * First delete the nodes, while strokes
     * and undo are still alive
     */
    m_d->rootLayer = 0;


    KisUpdateScheduler *scheduler = m_d->scheduler;
    m_d->scheduler = 0;
    delete scheduler;

    delete m_d->postExecutionUndoAdapter;
    delete m_d->legacyUndoAdapter;
    delete m_d->undoStore;
    delete m_d->compositeProgressProxy;

    delete m_d->signalRouter;
    delete m_d->perspectiveGrid;
    delete m_d->nserver;
    delete m_d;

    disconnect(); // in case Qt gets confused
}

void KisImage::aboutToAddANode(KisNode *parent, int index)
{
    SANITY_CHECK_LOCKED("aboutToAddANode");
    m_d->signalRouter->emitAboutToAddANode(parent, index);
}

void KisImage::nodeHasBeenAdded(KisNode *parent, int index)
{
    m_d->signalRouter->emitNodeHasBeenAdded(parent, index);
}

void KisImage::aboutToRemoveANode(KisNode *parent, int index)
{
    SANITY_CHECK_LOCKED("aboutToRemoveANode");
    m_d->signalRouter->emitAboutToRemoveANode(parent, index);
}

void KisImage::nodeHasBeenRemoved(KisNode *parent, int index)
{
    // XXX: Temporarily for compatibility
    m_d->signalRouter->emitNodeHasBeenRemoved(parent, index);
}

void KisImage::aboutToMoveNode(KisNode *parent, int oldIndex, int newIndex)
{
    SANITY_CHECK_LOCKED("aboutToMoveNode");
    m_d->signalRouter->emitAboutToMoveNode(parent, oldIndex, newIndex);
}

void KisImage::nodeHasBeenMoved(KisNode *parent, int oldIndex, int newIndex)
{
    m_d->signalRouter->emitNodeHasBeenMoved(parent, oldIndex, newIndex);
}

void KisImage::nodeChanged(KisNode* node)
{
    m_d->signalRouter->emitNodeChanged(node);
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

void KisImage::init(KisUndoStore *undoStore, qint32 width, qint32 height, const KoColorSpace *colorSpace)
{
    if (colorSpace == 0) {
        colorSpace = KoColorSpaceRegistry::instance()->rgb8();
    }

    m_d->lockCount = 0;
    m_d->sizeChangedWhileLocked = false;
    m_d->perspectiveGrid = 0;

    m_d->signalRouter = new KisImageSignalRouter(this);

    if(!undoStore) {
        undoStore = new KisDumbUndoStore();
    }

    m_d->undoStore = undoStore;
    m_d->legacyUndoAdapter = new KisLegacyUndoAdapter(m_d->undoStore, this);
    m_d->postExecutionUndoAdapter = new KisPostExecutionUndoAdapter(m_d->undoStore, this);

    m_d->nserver = new KisNameServer(1);

    m_d->colorSpace = colorSpace;

    setRootLayer(new KisGroupLayer(this, "root", OPACITY_OPAQUE_U8));

    m_d->xres = 1.0;
    m_d->yres = 1.0;
    m_d->unit = KoUnit::Point;
    m_d->width = width;
    m_d->height = height;

    m_d->recorder = new KisActionRecorder(this);

    m_d->compositeProgressProxy = new KisCompositeProgressProxy();

    m_d->scheduler = 0;
    if (m_d->startProjection) {
        m_d->scheduler = new KisUpdateScheduler(this);
        m_d->scheduler->setProgressProxy(m_d->compositeProgressProxy);
    }
}

KisCompositeProgressProxy* KisImage::compositeProgressProxy()
{
    return m_d->compositeProgressProxy;
}

bool KisImage::locked() const
{
    return m_d->lockCount != 0;
}

void KisImage::barrierLock()
{
    if (!locked()) {
        if (m_d->scheduler) {
            m_d->scheduler->barrierLock();
        }
        m_d->sizeChangedWhileLocked = false;
    }
    m_d->lockCount++;
}

bool KisImage::tryBarrierLock()
{
    bool result = true;

    if (!locked()) {
        if (m_d->scheduler) {
            result = m_d->scheduler->tryBarrierLock();
        }

        if(result) {
            m_d->sizeChangedWhileLocked = false;
        }
    }

    if(result) {
        m_d->lockCount++;
    }

    return result;
}

void KisImage::lock()
{
    if (!locked()) {
        if (m_d->scheduler) {
            m_d->scheduler->lock();
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
                m_d->signalRouter->emitNotification(SizeChangedSignal);
            }

            if (m_d->scheduler) {
                m_d->scheduler->unlock();
            }
        }
    }
}

void KisImage::blockUpdates()
{
    m_d->scheduler->blockUpdates();
}

void KisImage::unblockUpdates()
{
    m_d->scheduler->unblockUpdates();
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

void KisImage::resizeImageImpl(const QRect& newRect, bool cropLayers)
{
    if(newRect == bounds()) return;

    QString actionName = cropLayers ? i18n("Crop Image") : i18n("Resize Image");

    KisImageSignalVector emitSignals;
    emitSignals << SizeChangedSignal << ModifiedSignal;

    KisProcessingApplicator applicator(this, m_d->rootLayer,
                                       KisProcessingApplicator::RECURSIVE |
                                       KisProcessingApplicator::NO_UI_UPDATES,
                                       emitSignals, actionName);

    if(cropLayers || !newRect.topLeft().isNull()) {
        KisProcessingVisitorSP visitor =
            new KisCropProcessingVisitor(newRect, cropLayers, true);
        applicator.applyVisitor(visitor, KisStrokeJobData::CONCURRENT);
    }
    applicator.applyCommand(new KisImageResizeCommand(this, newRect.size()));
    applicator.end();
}

void KisImage::resizeImage(const QRect& newRect)
{
    resizeImageImpl(newRect, false);
}

void KisImage::cropImage(const QRect& newRect)
{
    resizeImageImpl(newRect, true);
}


void KisImage::cropNode(KisNodeSP node, const QRect& newRect)
{
    QString actionName = i18n("Crop Node");

    KisImageSignalVector emitSignals;
    emitSignals << ModifiedSignal;

    KisProcessingApplicator applicator(this, node,
                                       KisProcessingApplicator::NONE,
                                       emitSignals, actionName);

    KisProcessingVisitorSP visitor =
        new KisCropProcessingVisitor(newRect, true, false);
    applicator.applyVisitor(visitor, KisStrokeJobData::CONCURRENT);
    applicator.end();
}

void KisImage::emitSizeChanged()
{
    if (!locked()) {
        m_d->signalRouter->emitNotification(SizeChangedSignal);
    } else {
        m_d->sizeChangedWhileLocked = true;
    }
}

void KisImage::scaleImage(const QSize &size, qreal xres, qreal yres, KisFilterStrategy *filterStrategy)
{
    bool resolutionChanged = xres != xRes() && yres != yRes();
    bool sizeChanged = size != this->size();

    if(!resolutionChanged && !sizeChanged) return;

    KisImageSignalVector emitSignals;
    if (resolutionChanged) emitSignals << ResolutionChangedSignal;
    if (sizeChanged) emitSignals << SizeChangedSignal;
    emitSignals << ModifiedSignal;

    // XXX: Translate after 2.4 is released
    QString actionName = sizeChanged ? "Scale Image" : "Change Image Resolution";

    KisProcessingApplicator::ProcessingFlags signalFlags =
        (resolutionChanged || sizeChanged) ?
                KisProcessingApplicator::NO_UI_UPDATES :
                KisProcessingApplicator::NONE;

    KisProcessingApplicator applicator(this, m_d->rootLayer,
                                       KisProcessingApplicator::RECURSIVE | signalFlags,
                                       emitSignals, actionName);

    qreal sx = qreal(size.width()) / this->size().width();
    qreal sy = qreal(size.height()) / this->size().height();

    QTransform shapesCorrection;

    if(resolutionChanged) {
        shapesCorrection = QTransform::fromScale(xRes() / xres, yRes() / yres);
    }

    KisProcessingVisitorSP visitor =
        new KisTransformProcessingVisitor(sx, sy,
                                          0, 0,
                                          QPointF(),
                                          0,
                                          0, 0,
                                          filterStrategy,
                                          shapesCorrection);

    applicator.applyVisitor(visitor, KisStrokeJobData::CONCURRENT);

    if (resolutionChanged) {
        KUndo2Command *parent =
            new KisResetShapesCommand(m_d->rootLayer);
        new KisImageSetResolutionCommand(this, xres, yres, parent);
        applicator.applyCommand(parent);
    }

    if (sizeChanged) {
        applicator.applyCommand(new KisImageResizeCommand(this, size));
    }

    applicator.end();
}

void KisImage::rotate(double radians)
{
    qint32 w = width();
    qint32 h = height();
    qint32 tx = qint32((w * cos(radians) - h * sin(radians) - w) / 2 + 0.5);
    qint32 ty = qint32((h * cos(radians) + w * sin(radians) - h) / 2 + 0.5);
    w = (qint32)(width() * qAbs(cos(radians)) + height() * qAbs(sin(radians)) + 0.5);
    h = (qint32)(height() * qAbs(cos(radians)) + width() * qAbs(sin(radians)) + 0.5);

    tx -= (w - width()) / 2;
    ty -= (h - height()) / 2;

    // These signals will be emitted after processing is done
    KisImageSignalVector emitSignals;
    if (w != width() || h != height()) emitSignals << SizeChangedSignal;
    emitSignals << ModifiedSignal;

    // These flags determine whether updates are transferred to the UI during processing
    KisProcessingApplicator::ProcessingFlags signalFlags =
        (emitSignals.contains(SizeChangedSignal)) ?
                KisProcessingApplicator::NO_UI_UPDATES :
                KisProcessingApplicator::NONE;


    // XXX i18n("Rotate Image") after 2.4
    KisProcessingApplicator applicator(this, m_d->rootLayer,
                                       KisProcessingApplicator::RECURSIVE | signalFlags,
                                       emitSignals, "Rotate Image");

    KisFilterStrategy *filter = KisFilterStrategyRegistry::instance()->value("Triangle");

    KisProcessingVisitorSP visitor =
            new KisTransformProcessingVisitor(1.0, 1.0, 0.0, 0.0,
                                              QPointF(),
                                              radians,
                                              -tx, -ty, filter);

    applicator.applyVisitor(visitor, KisStrokeJobData::CONCURRENT);

    if (w != width() || h != height()) {
        applicator.applyCommand(new KisImageResizeCommand(this, QSize(w,h)));
    }
    applicator.end();
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

    undoAdapter()->beginMacro(i18n("Shear Image"));
    undoAdapter()->addCommand(new KisImageLockCommand(KisImageWSP(this), true));
    undoAdapter()->addCommand(new KisImageResizeCommand(KisImageWSP(this), newSize));

    KisShearVisitor visitor(angleX, angleY, progress);
    visitor.setUndoAdapter(undoAdapter());
    rootLayer()->accept(visitor);

    undoAdapter()->addCommand(new KisImageLockCommand(KisImageWSP(this), false));
    undoAdapter()->endMacro();

    setModified();
}

void KisImage::convertImageColorSpace(const KoColorSpace *dstColorSpace, KoColorConversionTransformation::Intent renderingIntent)
{
    if (*m_d->colorSpace == *dstColorSpace) return;

    const KoColorSpace *srcColorSpace = m_d->colorSpace;

    undoAdapter()->beginMacro(i18n("Convert Image Color Space"));
    undoAdapter()->addCommand(new KisImageLockCommand(KisImageWSP(this), true));
    undoAdapter()->addCommand(new KisImageSetProjectionColorSpaceCommand(KisImageWSP(this), dstColorSpace));

    KisColorSpaceConvertVisitor visitor(this, srcColorSpace, dstColorSpace, renderingIntent);
    m_d->rootLayer->accept(visitor);

    undoAdapter()->addCommand(new KisImageLockCommand(KisImageWSP(this), false));
    undoAdapter()->endMacro();

    setModified();
}

void KisImage::assignImageProfile(const KoColorProfile *profile)
{
    if(!profile) return;

    undoAdapter()->beginMacro(i18n("Assign Profile"));
    undoAdapter()->addCommand(new KisImageLockCommand(KisImageWSP(this), true));

    const KoColorSpace *dstCs = KoColorSpaceRegistry::instance()->colorSpace(colorSpace()->colorModelId().id(), colorSpace()->colorDepthId().id(), profile);
    const KoColorSpace *srcCs = colorSpace();

    KisChangeProfileVisitor visitor(srcCs, dstCs);
    m_d->rootLayer->accept(visitor);

    undoAdapter()->addCommand(new KisImageSetProjectionColorSpaceCommand(KisImageWSP(this), dstCs));
    undoAdapter()->addCommand(new KisImageLockCommand(KisImageWSP(this), false));
    undoAdapter()->endMacro();

    setModified();
    m_d->signalRouter->emitNotification(ProfileChangedSignal);
}

void KisImage::convertProjectionColorSpace(const KoColorSpace *dstColorSpace)
{
    if (*m_d->colorSpace == *dstColorSpace) return;

    undoAdapter()->beginMacro(i18n("Convert Projection Color Space"));
    undoAdapter()->addCommand(new KisImageLockCommand(KisImageWSP(this), true));
    undoAdapter()->addCommand(new KisImageSetProjectionColorSpaceCommand(KisImageWSP(this), dstColorSpace));
    undoAdapter()->addCommand(new KisImageLockCommand(KisImageWSP(this), false));
    undoAdapter()->endMacro();

    setModified();
}

void KisImage::setProjectionColorSpace(const KoColorSpace * colorSpace)
{
    m_d->colorSpace = colorSpace;
    m_d->rootLayer->resetCache();
    m_d->signalRouter->emitNotification(ColorSpaceChangedSignal);
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
    m_d->signalRouter->emitNotification(ResolutionChangedSignal);
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

    undoAdapter()->beginMacro(i18n("Flatten Image"));
    // NOTE: KisImageChangeLayersCommand performs all the locking for us
    undoAdapter()->addCommand(new KisImageChangeLayersCommand(KisImageWSP(this), oldRootLayer, newRootLayer, ""));
    undoAdapter()->endMacro();

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
    mergedLayer->setCompositeOp(prevLayer->compositeOp()->id());
    mergedLayer->setOpacity(prevLayer->opacity());
    mergedLayer->setChannelFlags(prevLayer->channelFlags());

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
    // AAA: do you really think you need it? ;) -- yes, we don't want to lose the masks

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
    m_d->signalRouter->emitNotification(ModifiedSignal);
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
    m_d->signalRouter->emitNotification(LayersChangedSignal);
}

QRect KisImage::bounds() const
{
    return QRect(0, 0, width(), height());
}

KisPostExecutionUndoAdapter* KisImage::postExecutionUndoAdapter() const
{
    return m_d->postExecutionUndoAdapter;
}

void KisImage::setUndoStore(KisUndoStore *undoStore)
{

    m_d->legacyUndoAdapter->setUndoStore(undoStore);
    m_d->postExecutionUndoAdapter->setUndoStore(undoStore);
    delete m_d->undoStore;
    m_d->undoStore = undoStore;
}

KisUndoStore* KisImage::undoStore()
{
    return m_d->undoStore;
}

KisUndoAdapter* KisImage::undoAdapter() const
{
    return m_d->legacyUndoAdapter;
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

KisImageSignalRouter* KisImage::signalRouter()
{
    return m_d->signalRouter;
}

void KisImage::waitForDone()
{
    if (m_d->scheduler) {
        m_d->scheduler->waitForDone();
    }
}

KisStrokeId KisImage::startStroke(KisStrokeStrategy *strokeStrategy)
{
    KisStrokeId id;

    if (m_d->scheduler) {
        id = m_d->scheduler->startStroke(strokeStrategy);
    }

    return id;
}

void KisImage::addJob(KisStrokeId id, KisStrokeJobData *data)
{
    if (m_d->scheduler) {
        m_d->scheduler->addJob(id, data);
    }
}

void KisImage::endStroke(KisStrokeId id)
{
    if (m_d->scheduler) {
        m_d->scheduler->endStroke(id);
    }
}

bool KisImage::cancelStroke(KisStrokeId id)
{
    bool result = false;
    if (m_d->scheduler) {
        result = m_d->scheduler->cancelStroke(id);
    }
    return result;
}

void KisImage::refreshGraph(KisNodeSP root)
{
    refreshGraph(root, bounds(), bounds());
}

void KisImage::refreshGraph(KisNodeSP root, const QRect &rc, const QRect &cropRect)
{
    if (!root) root = m_d->rootLayer;

    if (m_d->scheduler) {
        m_d->scheduler->fullRefresh(root, rc, cropRect);
    }
}

void KisImage::initialRefreshGraph()
{
    /**
     * NOTE: Tricky part. We set crop rect to null, so the clones
     * will not rely on precalculated projections of their sources
     */

    refreshGraphAsync(0, bounds(), QRect());
    waitForDone();
}

void KisImage::refreshGraphAsync(KisNodeSP root)
{
    refreshGraphAsync(root, bounds(), bounds());
}

void KisImage::refreshGraphAsync(KisNodeSP root, const QRect &rc)
{
    refreshGraphAsync(root, rc, bounds());
}

void KisImage::refreshGraphAsync(KisNodeSP root, const QRect &rc, const QRect &cropRect)
{
    if (!root) root = m_d->rootLayer;

    if (m_d->scheduler) {
        m_d->scheduler->fullRefreshAsync(root, rc, cropRect);
    }
}

void KisImage::disableUIUpdates()
{
    m_d->disableUIUpdateSignals.ref();
}

void KisImage::enableUIUpdates()
{
    m_d->disableUIUpdateSignals.deref();
}

void KisImage::notifyProjectionUpdated(const QRect &rc)
{
    if(!m_d->disableUIUpdateSignals) {
        emit sigImageUpdated(rc);
    }
}

void KisImage::requestProjectionUpdate(KisNode *node, const QRect& rect)
{
    if (m_d->scheduler) {
        m_d->scheduler->updateProjection(node, rect, bounds());
    }
}

#include "kis_image.moc"

