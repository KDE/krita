/*
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
 *  Copyright (c) 2005 C. Boemann <cbo@boemann.dk>
 *  Copyright (c) 2009 Dmitry Kazakov <dimula73@gmail.com>
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

#include "kis_layer.h"


#include <klocalizedstring.h>
#include <QImage>
#include <QBitArray>
#include <QStack>
#include <QMutex>
#include <QMutexLocker>
#include <QReadWriteLock>
#include <QReadLocker>
#include <QWriteLocker>

#include <KoIcon.h>
#include <kis_icon.h>
#include <KoProperties.h>
#include <KoCompositeOpRegistry.h>
#include <KoColorSpace.h>

#include "kis_debug.h"
#include "kis_image.h"

#include "kis_painter.h"
#include "kis_mask.h"
#include "kis_effect_mask.h"
#include "kis_selection_mask.h"
#include "kis_meta_data_store.h"
#include "kis_selection.h"
#include "kis_paint_layer.h"
#include "kis_raster_keyframe_channel.h"

#include "kis_clone_layer.h"

#include "kis_psd_layer_style.h"
#include "kis_layer_projection_plane.h"
#include "layerstyles/kis_layer_style_projection_plane.h"

#include "krita_utils.h"
#include "kis_layer_properties_icons.h"
#include "kis_layer_utils.h"
#include "kis_projection_leaf.h"
#include "KisRecycleProjectionsJob.h"


class KisSafeProjection {
public:
    KisPaintDeviceSP getDeviceLazy(KisPaintDeviceSP prototype) {
        QMutexLocker locker(&m_lock);

        if(!m_projection ||
           *m_projection->colorSpace() != *prototype->colorSpace()) {

            if (!m_cleanProjections.isEmpty()) {
                m_projection = m_cleanProjections.takeLast();
                m_projection->makeCloneFromRough(prototype, prototype->extent());
            } else {
                m_projection = new KisPaintDevice(*prototype);
            }

            m_projection->setProjectionDevice(true);
        }

        return m_projection;
    }

    void tryCopyFrom(const KisSafeProjection &rhs) {
        QMutexLocker rhsLocker(&rhs.m_lock);
        if (!rhs.m_projection) return;

        getDeviceLazy(rhs.m_projection);
    }

    bool releaseDevice() {
        QMutexLocker locker(&m_lock);

        bool result = false;

        if (m_projection) {
            m_dirtyProjections.append(m_projection);
            m_projection = 0;
            result = true;
        }

        return result;
    }

    void recycleProjections() {
        QMutexLocker locker(&m_lock);

        Q_FOREACH (KisPaintDeviceSP dev, m_dirtyProjections) {
            dev->clear();
            m_cleanProjections.append(dev);
        }
        m_dirtyProjections.clear();
    }

    void discardCaches() {
        QMutexLocker locker(&m_lock);
        m_dirtyProjections.clear();
    }

private:
    mutable QMutex m_lock;
    KisPaintDeviceSP m_projection;
    QVector<KisPaintDeviceSP> m_dirtyProjections;
    QVector<KisPaintDeviceSP> m_cleanProjections;
};

class KisCloneLayersList {
public:
    void addClone(KisCloneLayerWSP cloneLayer) {
        m_clonesList.append(cloneLayer);
    }

    void removeClone(KisCloneLayerWSP cloneLayer) {
        m_clonesList.removeOne(cloneLayer);
    }

    void setDirty(const QRect &rect) {
        Q_FOREACH (KisCloneLayerSP clone, m_clonesList) {
            if (clone) {
                clone->setDirtyOriginal(rect);
            }
        }
    }

    const QList<KisCloneLayerWSP> registeredClones() const {
        return m_clonesList;
    }

    bool hasClones() const {
        return !m_clonesList.isEmpty();
    }

private:
    QList<KisCloneLayerWSP> m_clonesList;
};

class KisLayerMasksCache {
public:
    KisLayerMasksCache(KisLayer *parent)
        : m_parent(parent)
    {
    }

    KisSelectionMaskSP selectionMask() {
        QReadLocker readLock(&m_lock);

        if (!m_isSelectionMaskValid) {
            readLock.unlock();

            QWriteLocker writeLock(&m_lock);
            if (!m_isSelectionMaskValid) {
                KoProperties properties;
                properties.setProperty("active", true);
                properties.setProperty("visible", true);
                QList<KisNodeSP> masks = m_parent->childNodes(QStringList("KisSelectionMask"), properties);

                // return the first visible mask
                Q_FOREACH (KisNodeSP mask, masks) {
                    if (mask) {
                        m_selectionMask = dynamic_cast<KisSelectionMask*>(mask.data());
                        break;
                    }
                }
                m_isSelectionMaskValid = true;
            }

            // return under write lock
            return m_selectionMask;
        }

        // return under read lock
        return m_selectionMask;
    }

    QList<KisEffectMaskSP> effectMasks() {
        QReadLocker readLock(&m_lock);

        if (!m_isEffectMasksValid) {
            readLock.unlock();

            QWriteLocker writeLock(&m_lock);
            if (!m_isEffectMasksValid) {
                m_effectMasks = m_parent->searchEffectMasks(0);
                m_isEffectMasksValid = true;
            }

            // return under write lock
            return m_effectMasks;
        }

        // return under read lock
        return m_effectMasks;
    }

    void setDirty()
    {
        QWriteLocker l(&m_lock);
        m_isSelectionMaskValid = false;
        m_isEffectMasksValid = false;
        m_selectionMask = 0;
        m_effectMasks.clear();
    }

private:
    KisLayer *m_parent;

    QReadWriteLock m_lock;

    bool m_isSelectionMaskValid = false;
    bool m_isEffectMasksValid = false;
    KisSelectionMaskSP m_selectionMask;
    QList<KisEffectMaskSP> m_effectMasks;
};

struct Q_DECL_HIDDEN KisLayer::Private
{
    Private(KisLayer *q) : masksCache(q) {}

    KisImageWSP image;
    QBitArray channelFlags;
    KisMetaData::Store* metaDataStore;
    KisSafeProjection safeProjection;
    KisCloneLayersList clonesList;

    KisPSDLayerStyleSP layerStyle;
    KisLayerStyleProjectionPlaneSP layerStyleProjectionPlane;

    KisAbstractProjectionPlaneSP projectionPlane;

    KisLayerMasksCache masksCache;
};


KisLayer::KisLayer(KisImageWSP image, const QString &name, quint8 opacity)
        : KisNode()
        , m_d(new Private(this))
{
    setName(name);
    setOpacity(opacity);
    m_d->image = image;
    m_d->metaDataStore = new KisMetaData::Store();
    m_d->projectionPlane = toQShared(new KisLayerProjectionPlane(this));
    connect(this, SIGNAL(internalInitiateProjectionsCleanup()), this, SLOT(slotInitiateProjectionsCleanup()));
}

KisLayer::KisLayer(const KisLayer& rhs)
        : KisNode(rhs)
        , m_d(new Private(this))
{
    if (this != &rhs) {
        m_d->image = rhs.m_d->image;
        m_d->metaDataStore = new KisMetaData::Store(*rhs.m_d->metaDataStore);
        m_d->channelFlags = rhs.m_d->channelFlags;

        m_d->safeProjection.tryCopyFrom(rhs.m_d->safeProjection);

        setName(rhs.name());
        m_d->projectionPlane = toQShared(new KisLayerProjectionPlane(this));

        if (rhs.m_d->layerStyle) {
            m_d->layerStyle = rhs.m_d->layerStyle->clone();

            if (rhs.m_d->layerStyleProjectionPlane) {
                m_d->layerStyleProjectionPlane = toQShared(
                    new KisLayerStyleProjectionPlane(*rhs.m_d->layerStyleProjectionPlane,
                                                     this,
                                                     m_d->layerStyle));
            }
        }
    }
    connect(this, SIGNAL(internalInitiateProjectionsCleanup()), this, SLOT(slotInitiateProjectionsCleanup()));
}

KisLayer::~KisLayer()
{
    delete m_d->metaDataStore;
    delete m_d;
}

const KoColorSpace * KisLayer::colorSpace() const
{
    KisImageSP image = m_d->image.toStrongRef();
    if (!image) {
        return nullptr;
    }
    return image->colorSpace();
}

const KoCompositeOp * KisLayer::compositeOp() const
{
    /**
     * FIXME: This function duplicates the same function from
     * KisMask. We can't move it to KisBaseNode as it doesn't
     * know anything about parent() method of KisNode
     * Please think it over...
     */

    KisNodeSP parentNode = parent();
    if (!parentNode) return 0;

    if (!parentNode->colorSpace()) return 0;
    const KoCompositeOp* op = parentNode->colorSpace()->compositeOp(compositeOpId());
    return op ? op : parentNode->colorSpace()->compositeOp(COMPOSITE_OVER);
}

KisPSDLayerStyleSP KisLayer::layerStyle() const
{
    return m_d->layerStyle;
}

void KisLayer::setLayerStyle(KisPSDLayerStyleSP layerStyle)
{
    if (layerStyle) {
        m_d->layerStyle = layerStyle;

        KisLayerStyleProjectionPlaneSP plane = !layerStyle->isEmpty() ?
            KisLayerStyleProjectionPlaneSP(new KisLayerStyleProjectionPlane(this)) :
            KisLayerStyleProjectionPlaneSP(0);

        m_d->layerStyleProjectionPlane = plane;
    } else {
        m_d->layerStyleProjectionPlane.clear();
        m_d->layerStyle.clear();
    }
}

KisBaseNode::PropertyList KisLayer::sectionModelProperties() const
{
    KisBaseNode::PropertyList l = KisBaseNode::sectionModelProperties();
    l << KisBaseNode::Property(KoID("opacity", i18n("Opacity")), i18n("%1%", percentOpacity()));

    const KoCompositeOp * compositeOp = this->compositeOp();

    if (compositeOp) {
        l << KisBaseNode::Property(KoID("compositeop", i18n("Blending Mode")), compositeOp->description());
    }

    if (m_d->layerStyle && !m_d->layerStyle->isEmpty()) {
        l << KisLayerPropertiesIcons::getProperty(KisLayerPropertiesIcons::layerStyle, m_d->layerStyle->isEnabled());
    }

    l << KisLayerPropertiesIcons::getProperty(KisLayerPropertiesIcons::inheritAlpha, alphaChannelDisabled());

    return l;
}

void KisLayer::setSectionModelProperties(const KisBaseNode::PropertyList &properties)
{
    KisBaseNode::setSectionModelProperties(properties);

    Q_FOREACH (const KisBaseNode::Property &property, properties) {
        if (property.id == KisLayerPropertiesIcons::inheritAlpha.id()) {
            disableAlphaChannel(property.state.toBool());
        }

        if (property.id == KisLayerPropertiesIcons::layerStyle.id()) {
            if (m_d->layerStyle &&
                m_d->layerStyle->isEnabled() != property.state.toBool()) {

                m_d->layerStyle->setEnabled(property.state.toBool());

                baseNodeChangedCallback();
                baseNodeInvalidateAllFramesCallback();
            }
        }
    }
}

void KisLayer::disableAlphaChannel(bool disable)
{
    QBitArray newChannelFlags = m_d->channelFlags;

    if(newChannelFlags.isEmpty())
        newChannelFlags = colorSpace()->channelFlags(true, true);

    if(disable)
        newChannelFlags &= colorSpace()->channelFlags(true, false);
    else
        newChannelFlags |= colorSpace()->channelFlags(false, true);

    setChannelFlags(newChannelFlags);
}

bool KisLayer::alphaChannelDisabled() const
{
    QBitArray flags = colorSpace()->channelFlags(false, true) & m_d->channelFlags;
    return flags.count(true) == 0 && !m_d->channelFlags.isEmpty();
}


void KisLayer::setChannelFlags(const QBitArray & channelFlags)
{
    Q_ASSERT(channelFlags.isEmpty() ||((quint32)channelFlags.count() == colorSpace()->channelCount()));

    if (KritaUtils::compareChannelFlags(channelFlags,
                                        this->channelFlags())) {
        return;
    }

    if (!channelFlags.isEmpty() &&
        channelFlags == QBitArray(channelFlags.size(), true)) {

        m_d->channelFlags.clear();
    } else {
        m_d->channelFlags = channelFlags;
    }

    baseNodeChangedCallback();
    baseNodeInvalidateAllFramesCallback();
}

QBitArray & KisLayer::channelFlags() const
{
    return m_d->channelFlags;
}

bool KisLayer::temporary() const
{
    return nodeProperties().boolProperty("temporary", false);
}

void KisLayer::setTemporary(bool t)
{
    setNodeProperty("temporary", t);
}

KisImageWSP KisLayer::image() const
{
    return m_d->image;
}

void KisLayer::setImage(KisImageWSP image)
{
    m_d->image = image;

    // we own the projection device, so we should take care about it
    KisPaintDeviceSP projection = this->projection();
    if (projection && projection != original()) {
        projection->setDefaultBounds(new KisDefaultBounds(image));
    }

    KisNodeSP node = firstChild();
    while (node) {
        KisLayerUtils::recursiveApplyNodes(node,
                                           [image] (KisNodeSP node) {
                                               node->setImage(image);
                                           });

        node = node->nextSibling();
    }
}

bool KisLayer::canMergeAndKeepBlendOptions(KisLayerSP otherLayer)
{
    return
        this->compositeOpId() == otherLayer->compositeOpId() &&
        this->opacity() == otherLayer->opacity() &&
        this->channelFlags() == otherLayer->channelFlags() &&
        !this->layerStyle() && !otherLayer->layerStyle() &&
        (this->colorSpace() == otherLayer->colorSpace() ||
         *this->colorSpace() == *otherLayer->colorSpace());
}

KisLayerSP KisLayer::createMergedLayerTemplate(KisLayerSP prevLayer)
{
    const bool keepBlendingOptions = canMergeAndKeepBlendOptions(prevLayer);

    KisLayerSP newLayer = new KisPaintLayer(image(), prevLayer->name(), OPACITY_OPAQUE_U8);

    if (keepBlendingOptions) {
        newLayer->setCompositeOpId(compositeOpId());
        newLayer->setOpacity(opacity());
        newLayer->setChannelFlags(channelFlags());
    }

    return newLayer;
}

void KisLayer::fillMergedLayerTemplate(KisLayerSP dstLayer, KisLayerSP prevLayer)
{
    const bool keepBlendingOptions = canMergeAndKeepBlendOptions(prevLayer);

    QRect layerProjectionExtent = this->projection()->extent();
    QRect prevLayerProjectionExtent = prevLayer->projection()->extent();
    bool alphaDisabled = this->alphaChannelDisabled();
    bool prevAlphaDisabled = prevLayer->alphaChannelDisabled();

    KisPaintDeviceSP mergedDevice = dstLayer->paintDevice();

    if (!keepBlendingOptions) {
        KisPainter gc(mergedDevice);

        KisImageSP imageSP = image().toStrongRef();
        if (!imageSP) {
            return;
        }

        //Copy the pixels of previous layer with their actual alpha value
        prevLayer->disableAlphaChannel(false);

        prevLayer->projectionPlane()->apply(&gc, prevLayerProjectionExtent | imageSP->bounds());

        //Restore the previous prevLayer disableAlpha status for correct undo/redo
        prevLayer->disableAlphaChannel(prevAlphaDisabled);

        //Paint the pixels of the current layer, using their actual alpha value
        if (alphaDisabled == prevAlphaDisabled) {
            this->disableAlphaChannel(false);
        }

        this->projectionPlane()->apply(&gc, layerProjectionExtent | imageSP->bounds());

        //Restore the layer disableAlpha status for correct undo/redo
        this->disableAlphaChannel(alphaDisabled);
    }
    else {
        //Copy prevLayer
        KisPaintDeviceSP srcDev = prevLayer->projection();
        mergedDevice->makeCloneFrom(srcDev, srcDev->extent());

        //Paint layer on the copy
        KisPainter gc(mergedDevice);
        gc.bitBlt(layerProjectionExtent.topLeft(), this->projection(), layerProjectionExtent);
    }
}

void KisLayer::registerClone(KisCloneLayerWSP clone)
{
    m_d->clonesList.addClone(clone);
}

void KisLayer::unregisterClone(KisCloneLayerWSP clone)
{
    m_d->clonesList.removeClone(clone);
}

const QList<KisCloneLayerWSP> KisLayer::registeredClones() const
{
    return m_d->clonesList.registeredClones();
}

bool KisLayer::hasClones() const
{
    return m_d->clonesList.hasClones();
}

void KisLayer::updateClones(const QRect &rect)
{
    m_d->clonesList.setDirty(rect);
}

void KisLayer::notifyChildMaskChanged()
{
    m_d->masksCache.setDirty();
}

KisSelectionMaskSP KisLayer::selectionMask() const
{
    return m_d->masksCache.selectionMask();
}

KisSelectionSP KisLayer::selection() const
{
    KisSelectionMaskSP mask = selectionMask();

    if (mask) {
        return mask->selection();
    }

    KisImageSP image = m_d->image.toStrongRef();
    if (image) {
        return image->globalSelection();
    }
    return KisSelectionSP();
}

///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////

QList<KisEffectMaskSP> KisLayer::effectMasks() const
{
    return m_d->masksCache.effectMasks();
}

QList<KisEffectMaskSP> KisLayer::effectMasks(KisNodeSP lastNode) const
{
    if (lastNode.isNull()) {
        return effectMasks();
    } else {
        // happens rarely.
        return searchEffectMasks(lastNode);
    }
}

QList<KisEffectMaskSP> KisLayer::searchEffectMasks(KisNodeSP lastNode) const
{
    QList<KisEffectMaskSP> masks;

    KIS_SAFE_ASSERT_RECOVER_NOOP(projectionLeaf());

    KisProjectionLeafSP child = projectionLeaf()->firstChild();
    while (child) {
        if (child->node() == lastNode) break;

        KIS_SAFE_ASSERT_RECOVER_NOOP(child);
        KIS_SAFE_ASSERT_RECOVER_NOOP(child->node());

        if (child->visible()) {
            KisEffectMaskSP mask = dynamic_cast<KisEffectMask*>(const_cast<KisNode*>(child->node().data()));
            if (mask) {
                masks.append(mask);
            }
        }

        child = child->nextSibling();
    }

    return masks;
}

void KisLayer::recycleProjectionsInSafety()
{
    m_d->safeProjection.recycleProjections();
}

void KisLayer::slotInitiateProjectionsCleanup()
{
    /**
     * After the projection has been used, we should clean it. But we cannot
     * clean it until all the workers accessing it have completed their job.
     *
     * Therefore we just schedule an exclusive job that will execute the
     * recycling action in an exclusive context, when no jobs are running.
     */

    if (m_d->image) {
        m_d->image->addSpontaneousJob(new KisRecycleProjectionsJob(this));
    } else {
        m_d->safeProjection.discardCaches();
    }
}

bool KisLayer::hasEffectMasks() const
{
    return  !m_d->masksCache.effectMasks().isEmpty();
}

QRect KisLayer::masksChangeRect(const QList<KisEffectMaskSP> &masks,
                                const QRect &requestedRect,
                                bool &rectVariesFlag) const
{
    rectVariesFlag = false;

    QRect prevChangeRect = requestedRect;

    /**
     * We set default value of the change rect for the case
     * when there is no mask at all
     */
    QRect changeRect = requestedRect;

    Q_FOREACH (const KisEffectMaskSP& mask, masks) {
        changeRect = mask->changeRect(prevChangeRect);

        if (changeRect != prevChangeRect)
            rectVariesFlag = true;

        prevChangeRect = changeRect;
    }

    return changeRect;
}

QRect KisLayer::masksNeedRect(const QList<KisEffectMaskSP> &masks,
                              const QRect &changeRect,
                              QStack<QRect> &applyRects,
                              bool &rectVariesFlag) const
{
    rectVariesFlag = false;

    QRect prevNeedRect = changeRect;
    QRect needRect;

    for (qint32 i = masks.size() - 1; i >= 0; i--) {
        applyRects.push(prevNeedRect);

        needRect = masks[i]->needRect(prevNeedRect);

        if (prevNeedRect != needRect)
            rectVariesFlag = true;

        prevNeedRect = needRect;
    }

    return needRect;
}

KisNode::PositionToFilthy calculatePositionToFilthy(KisNodeSP nodeInQuestion,
                                           KisNodeSP filthy,
                                           KisNodeSP parent)
{
    if (parent == filthy || parent != filthy->parent()) {
        return KisNode::N_ABOVE_FILTHY;
    }

    if (nodeInQuestion == filthy) {
        return KisNode::N_FILTHY;
    }

    KisNodeSP node = nodeInQuestion->prevSibling();
    while (node) {
        if (node == filthy) {
            return KisNode::N_ABOVE_FILTHY;
        }
        node = node->prevSibling();
    }

    return KisNode::N_BELOW_FILTHY;
}

QRect KisLayer::applyMasks(const KisPaintDeviceSP source,
                           KisPaintDeviceSP destination,
                           const QRect &requestedRect,
                           KisNodeSP filthyNode,
                           KisNodeSP lastNode) const
{
    Q_ASSERT(source);
    Q_ASSERT(destination);

    QList<KisEffectMaskSP> masks = effectMasks(lastNode);
    QRect changeRect;
    QRect needRect;

    if (masks.isEmpty()) {
        changeRect = requestedRect;
        if (source != destination) {
            copyOriginalToProjection(source, destination, requestedRect);
        }
    } else {
        QStack<QRect> applyRects;
        bool changeRectVaries;
        bool needRectVaries;

        /**
         * FIXME: Assume that varying of the changeRect has already
         * been taken into account while preparing walkers
         */
        changeRectVaries = false;
        changeRect = requestedRect;
        //changeRect = masksChangeRect(masks, requestedRect,
        //                             changeRectVaries);

        needRect = masksNeedRect(masks, changeRect,
                                 applyRects, needRectVaries);

        if (!changeRectVaries && !needRectVaries) {
            /**
             * A bit of optimization:
             * All filters will read/write exactly from/to the requested
             * rect so we needn't create temporary paint device,
             * just apply it onto destination
             */
            Q_ASSERT(needRect == requestedRect);

            if (source != destination) {
                copyOriginalToProjection(source, destination, needRect);
            }

            Q_FOREACH (const KisEffectMaskSP& mask, masks) {
                const QRect maskApplyRect = applyRects.pop();
                const QRect maskNeedRect =
                    applyRects.isEmpty() ? needRect : applyRects.top();

                PositionToFilthy maskPosition = calculatePositionToFilthy(mask, filthyNode, const_cast<KisLayer*>(this));
                mask->apply(destination, maskApplyRect, maskNeedRect, maskPosition);
            }
            Q_ASSERT(applyRects.isEmpty());
        } else {
            /**
             * We can't eliminate additional copy-op
             * as filters' behaviour may be quite insane here,
             * so let them work on their own paintDevice =)
             */

            KisPaintDeviceSP tempDevice = new KisPaintDevice(colorSpace());
            tempDevice->prepareClone(source);
            copyOriginalToProjection(source, tempDevice, needRect);

            QRect maskApplyRect = applyRects.pop();
            QRect maskNeedRect = needRect;

            Q_FOREACH (const KisEffectMaskSP& mask, masks) {
                PositionToFilthy maskPosition = calculatePositionToFilthy(mask, filthyNode, const_cast<KisLayer*>(this));
                mask->apply(tempDevice, maskApplyRect, maskNeedRect, maskPosition);

                if (!applyRects.isEmpty()) {
                    maskNeedRect = maskApplyRect;
                    maskApplyRect = applyRects.pop();
                }
            }
            Q_ASSERT(applyRects.isEmpty());

            KisPainter::copyAreaOptimized(changeRect.topLeft(), tempDevice, destination, changeRect);
        }
    }

    return changeRect;
}

QRect KisLayer::updateProjection(const QRect& rect, KisNodeSP filthyNode)
{
    QRect updatedRect = rect;
    KisPaintDeviceSP originalDevice = original();
    if (!rect.isValid() ||
            !visible() ||
            !originalDevice) return QRect();

    if (!needProjection() && !hasEffectMasks()) {
        if (m_d->safeProjection.releaseDevice()) {
            emit internalInitiateProjectionsCleanup();
        }
    } else {

        if (!updatedRect.isEmpty()) {
            KisPaintDeviceSP projection =
                m_d->safeProjection.getDeviceLazy(originalDevice);

            updatedRect = applyMasks(originalDevice, projection,
                                     updatedRect, filthyNode, 0);
        }
    }

    return updatedRect;
}

QRect KisLayer::partialChangeRect(KisNodeSP lastNode, const QRect& rect)
{
    bool changeRectVaries = false;
    QRect changeRect = outgoingChangeRect(rect);
    changeRect = masksChangeRect(effectMasks(lastNode), changeRect,
                                 changeRectVaries);

    return changeRect;
}

/**
 * \p rect is a dirty rect in layer's original() coordinates!
 */
void KisLayer::buildProjectionUpToNode(KisPaintDeviceSP projection, KisNodeSP lastNode, const QRect& rect)
{
    QRect changeRect = partialChangeRect(lastNode, rect);

    KisPaintDeviceSP originalDevice = original();

    KIS_ASSERT_RECOVER_RETURN(needProjection() || hasEffectMasks());

    if (!changeRect.isEmpty()) {
        applyMasks(originalDevice, projection,
                   changeRect, this, lastNode);
    }
}

bool KisLayer::needProjection() const
{
    return false;
}

void KisLayer::copyOriginalToProjection(const KisPaintDeviceSP original,
                                        KisPaintDeviceSP projection,
                                        const QRect& rect) const
{
    KisPainter::copyAreaOptimized(rect.topLeft(), original, projection, rect);
}

KisAbstractProjectionPlaneSP KisLayer::projectionPlane() const
{
    return m_d->layerStyleProjectionPlane ?
        KisAbstractProjectionPlaneSP(m_d->layerStyleProjectionPlane) : m_d->projectionPlane;
}

KisAbstractProjectionPlaneSP KisLayer::internalProjectionPlane() const
{
    return m_d->projectionPlane;
}

KisPaintDeviceSP KisLayer::projection() const
{
    KisPaintDeviceSP originalDevice = original();

    return needProjection() || hasEffectMasks() ?
        m_d->safeProjection.getDeviceLazy(originalDevice) : originalDevice;
}

QRect KisLayer::changeRect(const QRect &rect, PositionToFilthy pos) const
{
    QRect changeRect = rect;
    changeRect = incomingChangeRect(changeRect);

    if(pos == KisNode::N_FILTHY) {
        QRect projectionToBeUpdated = projection()->exactBoundsAmortized() & changeRect;

        bool changeRectVaries;
        changeRect = outgoingChangeRect(changeRect);
        changeRect = masksChangeRect(effectMasks(), changeRect, changeRectVaries);

        /**
         * If the projection contains some dirty areas we should also
         * add them to the change rect, because they might have
         * changed. E.g. when a visibility of the mask has chnaged
         * while the parent layer was invinisble.
         */

        if (!projectionToBeUpdated.isEmpty() &&
            !changeRect.contains(projectionToBeUpdated)) {

            changeRect |= projectionToBeUpdated;
        }
    }

    // TODO: string comparizon: optimize!
    if (pos != KisNode::N_FILTHY &&
        pos != KisNode::N_FILTHY_PROJECTION &&
        compositeOpId() != COMPOSITE_COPY) {

        changeRect |= rect;
    }

    return changeRect;
}

void KisLayer::childNodeChanged(KisNodeSP changedChildNode)
{
    if (dynamic_cast<KisMask*>(changedChildNode.data())) {
        notifyChildMaskChanged();
    }
}

QRect KisLayer::incomingChangeRect(const QRect &rect) const
{
    return rect;
}

QRect KisLayer::outgoingChangeRect(const QRect &rect) const
{
    return rect;
}

QImage KisLayer::createThumbnail(qint32 w, qint32 h)
{
    if (w == 0 || h == 0) {
        return QImage();
    }

    KisPaintDeviceSP originalDevice = original();

    return originalDevice ?
           originalDevice->createThumbnail(w, h, 1,
                                           KoColorConversionTransformation::internalRenderingIntent(),
                                           KoColorConversionTransformation::internalConversionFlags()) : QImage();
}

QImage KisLayer::createThumbnailForFrame(qint32 w, qint32 h, int time)
{
    if (w == 0 || h == 0) {
        return QImage();
    }

    KisPaintDeviceSP originalDevice = original();
    if (originalDevice ) {
        KisRasterKeyframeChannel *channel = originalDevice->keyframeChannel();

        if (channel) {
            KisPaintDeviceSP targetDevice = new KisPaintDevice(colorSpace());
            KisKeyframeSP keyframe = channel->activeKeyframeAt(time);
            channel->fetchFrame(keyframe, targetDevice);
            return targetDevice->createThumbnail(w, h, 1,
                                                 KoColorConversionTransformation::internalRenderingIntent(),
                                                 KoColorConversionTransformation::internalConversionFlags());
        }
    }

    return createThumbnail(w, h);
}

qint32 KisLayer::x() const
{
    KisPaintDeviceSP originalDevice = original();
    return originalDevice ? originalDevice->x() : 0;
}
qint32 KisLayer::y() const
{
    KisPaintDeviceSP originalDevice = original();
    return originalDevice ? originalDevice->y() : 0;
}
void KisLayer::setX(qint32 x)
{
    KisPaintDeviceSP originalDevice = original();
    if (originalDevice)
        originalDevice->setX(x);
}
void KisLayer::setY(qint32 y)
{
    KisPaintDeviceSP originalDevice = original();
    if (originalDevice)
        originalDevice->setY(y);
}

QRect KisLayer::layerExtentImpl(bool needExactBounds) const
{
    QRect additionalMaskExtent = QRect();
    QList<KisEffectMaskSP> effectMasks = this->effectMasks();

    Q_FOREACH(KisEffectMaskSP mask, effectMasks) {
        additionalMaskExtent |= mask->nonDependentExtent();
    }

    KisPaintDeviceSP originalDevice = original();
    QRect layerExtent;

    if (originalDevice) {
        layerExtent = needExactBounds ?
            originalDevice->exactBounds() :
            originalDevice->extent();
    }

    QRect additionalCompositeOpExtent;
    if (compositeOpId() == COMPOSITE_DESTINATION_IN ||
        compositeOpId() == COMPOSITE_DESTINATION_ATOP) {

        additionalCompositeOpExtent = originalDevice->defaultBounds()->bounds();
    }

    return layerExtent | additionalMaskExtent | additionalCompositeOpExtent;
}

QRect KisLayer::extent() const
{
    return layerExtentImpl(false);
}

QRect KisLayer::exactBounds() const
{
    return layerExtentImpl(true);
}

KisLayerSP KisLayer::parentLayer() const
{
    return qobject_cast<KisLayer*>(parent().data());
}

KisMetaData::Store* KisLayer::metaData()
{
    return m_d->metaDataStore;
}

