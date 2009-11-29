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

#include <config-endian.h> // WORDS_BIGENDIAN

#include <stdlib.h>
#include <math.h>

#include <lcms.h>

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
#include "colorprofiles/KoIccColorProfile.h"

#include "recorder/kis_action_recorder.h"
#include "kis_adjustment_layer.h"
#include "kis_annotation.h"
#include "kis_projection.h"
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
#include "kis_paint_device_action.h"
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

#include "kis_refresh_visitor.h"

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

    KisProjection* projection;

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
        , KisShared(rhs)
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
            m_d->projection = new KisProjection(this);
            m_d->projection->start();
        }
    }
}

KisImage::~KisImage()
{
    dbgImage << "deleting kisimage" << objectName();
    if (m_d->projection) {
        m_d->projection->stop();
        m_d->projection->deleteLater();
    }
    delete m_d->perspectiveGrid;
    delete m_d->nserver;
    delete m_d;
}


void KisImage::aboutToAddANode(KisNode *parent, int index)
{
    emit sigAboutToAddANode(parent, index);
}

void KisImage::nodeHasBeenAdded(KisNode *parent, int index)
{
    KisLayer * layer = dynamic_cast<KisLayer*>(parent->at(index).data());
    if (layer) {
        // The addition of temporary layers is not interesting
        KoProperties props;
        props.setProperty("temporary", false);

        if (!layer->check(props)) {
            emit sigLayerMoved(layer);
        }
    }
    emit sigNodeHasBeenAdded(parent, index);
}

void KisImage::aboutToRemoveANode(KisNode *parent, int index)
{
    emit sigAboutToRemoveANode(parent, index);
}

void KisImage::nodeHasBeenRemoved(KisNode *parent, int index)
{
    // XXX: Temporarily for compatibility
    emit sigNodeHasBeenRemoved(parent, index);

    KisLayer * l = dynamic_cast<KisLayer*>(parent->at(index).data());
    if (l) {
        // The removal of temporary layers is not interesting
        KoProperties props;
        props.setProperty("temporary", false);

        if (!l->check(props)) {
            emit sigLayerRemoved(l);
        }
    }
}

void KisImage::aboutToMoveNode(KisNode *parent, int oldIndex, int newIndex)
{
    Q_UNUSED(parent);
    Q_UNUSED(oldIndex);
    Q_UNUSED(newIndex);
}

void KisImage::nodeHasBeenMoved(KisNode *parent, int oldIndex, int newIndex)
{
    Q_UNUSED(oldIndex);

    KisLayer * l = dynamic_cast<KisLayer*>(parent->at(newIndex).data());
    if (l) {

        // The moving of temporary layers is not interesting

        KoProperties props;
        props.setProperty("temporary", false);

        if (!l->check(props)) {

            emit sigLayerMoved(l);
        }
    }
}


KisSelectionSP KisImage::globalSelection() const
{
    return m_d->globalSelection;
}

void KisImage::setGlobalSelection(KisSelectionSP globalSelection)
{
    if (globalSelection == 0)
        m_d->globalSelection = new KisSelection(m_d->rootLayer->projection());
    else
        m_d->globalSelection = globalSelection;
}

void KisImage::removeGlobalSelection()
{
    m_d->globalSelection = 0;
}

KisSelectionSP KisImage::deleselectedGlobalSelection()
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

    setRootLayer(new KisGroupLayer(this, "root", OPACITY_OPAQUE));

    m_d->xres = 1.0;
    m_d->yres = 1.0;
    m_d->unit = KoUnit::Point;
    m_d->width = width;
    m_d->height = height;

    m_d->recorder = new KisActionRecorder();

    m_d->projection = 0;
    if (m_d->startProjection) {
        m_d->projection = new KisProjection(this);
        m_d->projection->start();
    }
}

bool KisImage::locked() const
{
    return m_d->lockCount != 0;
}

void KisImage::lock()
{
    blockSignals(true);
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
        blockSignals(false);
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


void KisImage::resize(const QRect& rc, bool cropLayers)
{
    resize(rc.width(), rc.height(), rc.x(), rc.y(), cropLayers);
}

void KisImage::resize(qint32 w, qint32 h, qint32 x, qint32 y, bool cropLayers)
{
    if (w != width() || h != height()) {

        lock();

        if (undo()) {
            if (cropLayers)
                m_d->adapter->beginMacro(i18n("Crop Image"));
            else
                m_d->adapter->beginMacro(i18n("Resize Image"));

            m_d->adapter->addCommand(new KisImageLockCommand(KisImageWSP(this), true));
            m_d->adapter->addCommand(new KisImageResizeCommand(KisImageWSP(this), w, h, width(), height()));
        }

        m_d->width = w;
        m_d->height = h;

        if (cropLayers) {
            KisCropVisitor v(QRect(x, y, w, h), m_d->adapter);
            m_d->rootLayer->accept(v);
        }

        emitSizeChanged();

        unlock();

        if (undo()) {
            m_d->adapter->addCommand(new KisImageLockCommand(KisImageWSP(this), false));
            m_d->adapter->endMacro();
        }
    }
}

void KisImage::resizeWithOffset(qint32 w, qint32 h, qint32 xOffset, qint32 yOffset)
{
    if (w == width() && h == height() && xOffset == 0 && yOffset == 0)
        return;

    lock();
    if (undo()) {
        m_d->adapter->beginMacro(i18n("Size Canvas"));
        m_d->adapter->addCommand(new KisImageLockCommand(KisImageWSP(this), true));
        m_d->adapter->addCommand(new KisImageResizeCommand(KisImageWSP(this), w, h, width(), height()));
    }

    KisCropVisitor v(QRect(-xOffset, -yOffset, w, h), m_d->adapter);
    m_d->rootLayer->accept(v);

    emitSizeChanged();

    unlock();

    if (undo()) {
        m_d->adapter->addCommand(new KisImageLockCommand(KisImageWSP(this), false));
        m_d->adapter->endMacro();
    }

}

void KisImage::emitSizeChanged()
{
    if (!locked()) {
        emit sigSizeChanged(m_d->width, m_d->height);
    } else {
        m_d->sizeChangedWhileLocked = true;
    }
}


void KisImage::scale(double sx, double sy, KoUpdater *progress, KisFilterStrategy *filterStrategy)
{
    QStringList list;
    list << "KisLayer";

    KisCountVisitor visitor(list, KoProperties());
    m_d->rootLayer->accept(visitor);

    if (visitor.count() == 0) return; // Nothing to scale

    // New image size. XXX: Pass along to discourage rounding errors?
    qint32 w, h;
    w = (qint32)((width() * sx) + 0.5);
    h = (qint32)((height() * sy) + 0.5);

    if (w != width() || h != height()) {

        lock();

        if (undo()) {
            m_d->adapter->beginMacro(i18n("Scale Image"));
            m_d->adapter->addCommand(new KisImageLockCommand(KisImageWSP(this), true));
        }

        {
            KisTransformVisitor visitor(KisImageWSP(this), sx, sy, 0.0, 0.0, 0.0, 0, 0, progress, filterStrategy);
            m_d->rootLayer->accept(visitor);
        }

        if (undo()) {
            m_d->adapter->addCommand(new KisImageResizeCommand(KisImageWSP(this), w, h, width(), height()));
        }

        m_d->width = w;
        m_d->height = h;

        emitSizeChanged();

        unlock();

        if (undo()) {
            m_d->adapter->addCommand(new KisImageLockCommand(KisImageWSP(this), false));
            m_d->adapter->endMacro();
        }
    }
}
void KisImage::rotate(double radians, KoUpdater *progress)
{
    lock();

    qint32 w = width();
    qint32 h = height();
    qint32 tx = qint32((w * cos(radians) - h * sin(radians) - w) / 2 + 0.5);
    qint32 ty = qint32((h * cos(radians) + w * sin(radians) - h) / 2 + 0.5);
    w = (qint32)(width() * qAbs(cos(radians)) + height() * qAbs(sin(radians)) + 0.5);
    h = (qint32)(height() * qAbs(cos(radians)) + width() * qAbs(sin(radians)) + 0.5);

    tx -= (w - width()) / 2;
    ty -= (h - height()) / 2;

    if (undo()) {
        m_d->adapter->beginMacro(i18n("Rotate Image"));
        m_d->adapter->addCommand(new KisImageLockCommand(KisImageWSP(this), true));
    }

    KisFilterStrategy *filter = KisFilterStrategyRegistry::instance()->value("Triangle");
    KisTransformVisitor visitor(KisImageWSP(this), 1.0, 1.0, 0, 0, radians, -tx, -ty, progress, filter);
    m_d->rootLayer->accept(visitor);

    if (undo()) m_d->adapter->addCommand(new KisImageResizeCommand(KisImageWSP(this), w, h, width(), height()));

    m_d->width = w;
    m_d->height = h;

    emitSizeChanged();

    unlock();

    if (undo()) {
        m_d->adapter->addCommand(new KisImageLockCommand(KisImageWSP(this), false));
        m_d->adapter->endMacro();
    }
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

    if (w != width() || h != height()) {

        lock();

        if (undo()) {
            m_d->adapter->beginMacro(i18n("Shear Image"));
            m_d->adapter->addCommand(new KisImageLockCommand(KisImageWSP(this), true));
        }

        KisShearVisitor v(angleX, angleY, progress);
        v.setUndoAdapter(m_d->adapter);
        rootLayer()->accept(v);

        if (undo()) m_d->adapter->addCommand(new KisImageResizeCommand(KisImageWSP(this), w, h, width(), height()));

        m_d->width = w;
        m_d->height = h;

        emitSizeChanged();

        unlock();

        if (undo()) {
            m_d->adapter->addCommand(new KisImageLockCommand(KisImageWSP(this), false));
            m_d->adapter->endMacro();
        }
    }
}

void KisImage::convertTo(const KoColorSpace *dstColorSpace, KoColorConversionTransformation::Intent renderingIntent)
{
    if (*m_d->colorSpace == *dstColorSpace) {
        return;
    }

    lock();

    const KoColorSpace * oldCs = m_d->colorSpace;

    if (undo()) {
        m_d->adapter->beginMacro(i18n("Convert Image Type"));
        m_d->adapter->addCommand(new KisImageLockCommand(KisImageWSP(this), true));
    }

    KisColorSpaceConvertVisitor visitor(this, dstColorSpace, renderingIntent);
    m_d->rootLayer->accept(visitor);

    if (undo()) {

        m_d->adapter->addCommand(new KisImageConvertTypeCommand(KisImageWSP(this), oldCs, dstColorSpace));
        m_d->adapter->addCommand(new KisImageLockCommand(KisImageWSP(this), false));
        m_d->adapter->endMacro();
    } else {
        setColorSpace(dstColorSpace);
    }

    unlock();
    refreshGraph();
}

const KoColorProfile * KisImage::profile() const
{
    return colorSpace()->profile();
}

void KisImage::setProfile(const KoColorProfile *profile)
{
    if (profile == 0) return;

    kDebug() << profile;

    const KoColorSpace *dstCs = KoColorSpaceRegistry::instance()->colorSpace(colorSpace()->id(), profile);
    if (dstCs) {

        lock();

        const KoColorSpace *oldCs = colorSpace();
        setColorSpace(dstCs);
        emit(sigProfileChanged(const_cast<KoColorProfile*>(profile)));

        KisChangeProfileVisitor visitor(oldCs, dstCs);
        m_d->rootLayer->accept(visitor);

        unlock();
    }
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

void KisImage::flatten()
{
    KisGroupLayerSP oldRootLayer = m_d->rootLayer;

    KisPaintLayer *dst = new KisPaintLayer(this, nextLayerName(), OPACITY_OPAQUE, colorSpace());
    Q_CHECK_PTR(dst);

    QRect rc = mergedImage()->extent();

    KisPainter gc(dst->paintDevice());
    gc.setCompositeOp(COMPOSITE_OVER);
    gc.bitBlt(rc.x(), rc.y(), mergedImage(), rc.left(), rc.top(), rc.width(), rc.height());

    setRootLayer(new KisGroupLayer(this, "root", OPACITY_OPAQUE));

    if (undo()) {
        m_d->adapter->beginMacro(i18n("Flatten Image"));
        m_d->adapter->addCommand(new KisImageLockCommand(KisImageWSP(this), true));
        m_d->adapter->addCommand(new KisImageChangeLayersCommand(KisImageWSP(this), oldRootLayer, m_d->rootLayer, ""));
    }

    lock();

    addNode(dst, m_d->rootLayer.data(), 0);

    unlock();

    notifyLayersChanged();

    if (undo()) {
        m_d->adapter->addCommand(new KisImageLockCommand(KisImageWSP(this), false));
        m_d->adapter->endMacro();
    }
}


KisLayerSP KisImage::mergeLayer(KisLayerSP layer, const KisMetaData::MergeStrategy* strategy)
{
    if (!layer->prevSibling()) return 0;
    // XXX: this breaks if we allow free mixing of masks and layers
    KisLayerSP layer2 = dynamic_cast<KisLayer*>(layer->prevSibling().data());
    dbgImage << "Merge " << layer << " with " << layer2;
    if (!layer2) return 0;

    KisPaintLayerSP newLayer = new KisPaintLayer(this, layer->name(), OPACITY_OPAQUE, colorSpace());
    Q_CHECK_PTR(newLayer);

    QRect layerExtent = layer->extent();
    QRect layerPrevSiblingExtent = layer->prevSibling()->extent();

    QRect rc = layerExtent | layerPrevSiblingExtent;

    undoAdapter()->beginMacro(i18n("Merge with Layer Below"));

    KisPainter gc(newLayer->paintDevice());
    gc.setCompositeOp(newLayer->colorSpace()->compositeOp(COMPOSITE_COPY));
    gc.bitBlt(rc.topLeft(), layer2->projection(), rc);

    gc.setCompositeOp(layer->compositeOp());
    gc.setOpacity(layer->opacity());
    gc.bitBlt(rc.topLeft(), layer->projection(), rc);

    // Merge meta data
    QList<const KisMetaData::Store*> srcs;
    srcs.append(static_cast<KisLayer*>(layer->prevSibling().data())->metaData());
    srcs.append(layer->metaData());
    QList<double> scores;
    int layerPrevSiblingArea = layerPrevSiblingExtent.width() * layerPrevSiblingExtent.height();
    int layerArea = layerExtent.width() * layerExtent.height();
    double norm = qMax(layerPrevSiblingArea, layerArea);
    scores.append(layerPrevSiblingArea / norm);
    scores.append(layerArea / norm);
    strategy->merge(newLayer->metaData(), srcs, scores);

    KisNodeSP parent = layer->parent(); // parent is set to null when the layer is removed from the node
    dbgImage << ppVar(parent);

    // XXX: merge the masks!

    undoAdapter()->addCommand(new KisImageLayerAddCommand(this, newLayer, parent, layer));
    undoAdapter()->addCommand(new KisImageLayerRemoveCommand(this, layer->prevSibling()));
    undoAdapter()->addCommand(new KisImageLayerRemoveCommand(this, layer));

    undoAdapter()->endMacro();

    return newLayer;
}

KisLayerSP KisImage::flattenLayer(KisLayerSP layer)
{
    if (!layer->firstChild()) return layer;

    undoAdapter()->beginMacro(i18n("Flatten Layer"));
    KisPaintLayerSP newLayer = new KisPaintLayer(this, layer->name(), layer->opacity(), colorSpace());
    newLayer->setCompositeOp(layer->compositeOp()->id());
    newLayer->metaData();
    QRect rc = layer->extent();

    KisPainter gc(newLayer->paintDevice());
    gc.setCompositeOp(newLayer->colorSpace()->compositeOp(COMPOSITE_COPY));
    gc.bitBlt(rc.topLeft(), layer->projection(), rc);
    gc.end();
    undoAdapter()->addCommand(new KisImageLayerAddCommand(this, newLayer, layer->parent(), layer->nextSibling()));

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

void KisImage::notifyPropertyChanged(KisLayerSP layer)
{
    emit sigLayerPropertiesChanged(layer);
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

bool KisImage::undo() const
{
    return (m_d->adapter && m_d->adapter->undo());
}

const KoColorSpace * KisImage::colorSpace() const
{
    return m_d->colorSpace;
}

void KisImage::setColorSpace(const KoColorSpace * colorSpace)
{
    m_d->colorSpace = colorSpace;
    m_d->rootLayer->resetCache();
    emit sigColorSpaceChanged(colorSpace);
}

void KisImage::setRootLayer(KisGroupLayerSP rootLayer)
{
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
        // XXX we hardcode icc, this is correct for lcms?
        // XXX productName(), or just "ICC Profile"?
        if (profile->valid()) {
            const KoIccColorProfile* iccprofile = dynamic_cast<const KoIccColorProfile*>(profile);
            if (iccprofile && !iccprofile->rawData().isEmpty()) {
                annotation = new  KisAnnotation("icc", iccprofile->name(), iccprofile->rawData());
            }
        }
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

KisPerspectiveGrid* KisImage::perspectiveGrid()
{
    if (m_d->perspectiveGrid == 0)
        m_d->perspectiveGrid = new KisPerspectiveGrid();
    return m_d->perspectiveGrid;
}

void KisImage::refreshGraph()
{
    KisRefreshVisitor refresher(this);
    rootLayer()->accept(refresher);
}

void KisImage::slotProjectionUpdated(const QRect & rc)
{
    emit sigImageUpdated(rc);
}

void KisImage::updateProjection(KisNodeSP node, const QRect& rc)
{
    if (!locked() && m_d->projection) {
        dbgImage << "Updating node " << node << "rect" << rc;
        m_d->projection->updateProjection(node, rc);
    }
}

#include "kis_image.moc"

