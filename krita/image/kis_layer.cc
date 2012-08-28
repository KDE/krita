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


#include <klocale.h>
#include <QImage>
#include <QBitArray>
#include <QStack>
#include <QMutex>
#include <QMutexLocker>

#include <KoProperties.h>
#include <KoCompositeOp.h>
#include <KoColorSpace.h>

#include "kis_debug.h"
#include "kis_image.h"

#include "kis_painter.h"
#include "kis_mask.h"
#include "kis_effect_mask.h"
#include "kis_transparency_mask.h"
#include "kis_selection_mask.h"
#include "kis_meta_data_store.h"
#include "kis_selection.h"

#include "kis_clone_layer.h"


class KisSafeProjection {
public:
    KisPaintDeviceSP getDeviceLazy(KisPaintDeviceSP prototype) {
        QMutexLocker locker(&m_lock);

        if(!m_reusablePaintDevice)
            m_reusablePaintDevice = new KisPaintDevice(*prototype);

        if(!m_projection ||
           !(*m_projection->colorSpace() == *prototype->colorSpace())) {
            m_projection = m_reusablePaintDevice;
            m_projection->makeCloneFromRough(prototype, prototype->extent());
        }

        return m_projection;
    }

    void freeDevice() {
        QMutexLocker locker(&m_lock);
        m_projection = 0;
        if(m_reusablePaintDevice) {
            m_reusablePaintDevice->clear();
        }
    }

private:
    QMutex m_lock;
    KisPaintDeviceSP m_projection;
    KisPaintDeviceSP m_reusablePaintDevice;
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
        foreach(KisCloneLayerWSP clone, m_clonesList) {
            clone->setDirtyOriginal(rect);
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

struct KisLayer::Private
{

public:

    KisImageWSP image;
    QBitArray channelFlags;
    KisEffectMaskSP previewMask;
    KisMetaData::Store* metaDataStore;
    KisSafeProjection safeProjection;
    KisCloneLayersList clonesList;
};


KisLayer::KisLayer(KisImageWSP image, const QString &name, quint8 opacity)
        : KisNode()
        , m_d(new Private)
{
    setName(name);
    setOpacity(opacity);
    m_d->image = image;
    m_d->metaDataStore = new KisMetaData::Store();
}

KisLayer::KisLayer(const KisLayer& rhs)
        : KisNode(rhs)
        , m_d(new Private())
{
    if (this != &rhs) {
        m_d->image = rhs.m_d->image;
        m_d->metaDataStore = new KisMetaData::Store(*rhs.m_d->metaDataStore);
        setName(i18n("Duplicate of '%1'", rhs.name()));
    }
}

KisLayer::~KisLayer()
{
    delete m_d->metaDataStore;
    delete m_d;
}

const KoColorSpace * KisLayer::colorSpace() const
{
    if (m_d->image)
        return m_d->image->colorSpace();
    return 0;
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

KoDocumentSectionModel::PropertyList KisLayer::sectionModelProperties() const
{
    KoDocumentSectionModel::PropertyList l = KisBaseNode::sectionModelProperties();
    l << KoDocumentSectionModel::Property(i18n("Opacity"), i18n("%1%", percentOpacity()));
    if (compositeOp())
        l << KoDocumentSectionModel::Property(i18n("Composite Mode"), compositeOp()->description());
    return l;
}

void KisLayer::setSectionModelProperties(const KoDocumentSectionModel::PropertyList &properties)
{
    KisBaseNode::setSectionModelProperties(properties);
}

void KisLayer::disableAlphaChannel(bool disable)
{
    if(m_d->channelFlags.isEmpty())
        m_d->channelFlags = colorSpace()->channelFlags(true, true);

    if(disable)
        m_d->channelFlags &= colorSpace()->channelFlags(true, false);
    else
        m_d->channelFlags |= colorSpace()->channelFlags(false, true);
}

bool KisLayer::alphaChannelDisabled() const
{
    QBitArray flags = colorSpace()->channelFlags(false, true) & m_d->channelFlags;
    return flags.count(true) == 0 && !m_d->channelFlags.isEmpty();
}


void KisLayer::setChannelFlags(const QBitArray & channelFlags)
{
    Q_ASSERT(((quint32)channelFlags.count() == colorSpace()->channelCount() || channelFlags.isEmpty()));
    m_d->channelFlags = channelFlags;
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
    nodeProperties().setProperty("temporary", t);
}

KisImageWSP KisLayer::image() const
{
    return m_d->image;
}

void KisLayer::setImage(KisImageWSP image)
{
    m_d->image = image;
    for (uint i = 0; i < childCount(); ++i) {
        // Only layers know about the image
        KisLayer * layer = dynamic_cast<KisLayer*>(at(i).data());
        if (layer)
            layer->setImage(image);
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

KisSelectionMaskSP KisLayer::selectionMask() const
{
    KoProperties properties;
    properties.setProperty("active", true);
    QList<KisNodeSP> masks = childNodes(QStringList("KisSelectionMask"), properties);
    Q_ASSERT(masks.size() <= 1); // only one active mask at a time

    //finds the active selection mask
    if (masks.size() == 1) {
        KisSelectionMaskSP selectionMask = dynamic_cast<KisSelectionMask*>(masks[0].data());
        return selectionMask;
    }
    return 0;
}

KisSelectionSP KisLayer::selection() const
{
    if (selectionMask()) {
        return selectionMask()->selection();
    }
    else if (m_d->image) {
        return m_d->image->globalSelection();
    }
    else {
        return 0;
    }
}

///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////

QList<KisEffectMaskSP> KisLayer::effectMasks() const
{
    QList<KisEffectMaskSP> masks;

    if (m_d->previewMask && m_d->previewMask->visible()) {
        masks.append(m_d->previewMask);
    }

    if (childCount() > 0) {
        KoProperties properties;
        properties.setProperty("visible", true);
        QList<KisNodeSP> nodes = childNodes(QStringList("KisEffectMask"), properties);

        foreach(const KisNodeSP& node,  nodes) {
            KisEffectMaskSP mask = dynamic_cast<KisEffectMask*>(const_cast<KisNode*>(node.data()));
            if (mask)
                masks.append(mask);
        }
    }
    return masks;
}

bool KisLayer::hasEffectMasks() const
{
    if (m_d->previewMask && m_d->previewMask->visible()) return true;
    if (childCount() == 0) return false;

    KoProperties properties;
    properties.setProperty("visible", true);

    QList<KisNodeSP> masks = childNodes(QStringList("KisEffectMask"), properties);
    if (!masks.isEmpty()) return true;

    return false;
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

    foreach(const KisEffectMaskSP& mask, masks) {
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

QRect KisLayer::applyMasks(const KisPaintDeviceSP source,
                           const KisPaintDeviceSP destination,
                           const QRect &requestedRect) const
{
    Q_ASSERT(source);
    Q_ASSERT(destination);

    QList<KisEffectMaskSP> masks = effectMasks();
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

            foreach(const KisEffectMaskSP& mask, masks) {
                mask->apply(destination, applyRects.pop());
            }
            Q_ASSERT(applyRects.isEmpty());
        } else {
            /**
             * We can't eliminate additional copy-op
             * as filters' behaviour may be quite insane here,
             * so let them work on their own paintDevice =)
             */

            KisPaintDeviceSP tempDevice = new KisPaintDevice(colorSpace());
            copyOriginalToProjection(source, tempDevice, needRect);

            foreach(const KisEffectMaskSP& mask, masks) {
                mask->apply(tempDevice, applyRects.pop());
            }
            Q_ASSERT(applyRects.isEmpty());

            KisPainter gc2(destination);
            gc2.setCompositeOp(colorSpace()->compositeOp(COMPOSITE_COPY));
            gc2.bitBlt(changeRect.topLeft(), tempDevice, changeRect);
        }
    }

    return changeRect;
}

QRect KisLayer::updateProjection(const QRect& rect)
{
    QRect updatedRect = rect;
    KisPaintDeviceSP originalDevice = original();

    if (!rect.isValid() ||
            !visible() ||
            !originalDevice) return QRect();

    if (!needProjection() && !hasEffectMasks()) {
        m_d->safeProjection.freeDevice();
    } else {
        if (!updatedRect.isEmpty()) {
            KisPaintDeviceSP projection =
                m_d->safeProjection.getDeviceLazy(originalDevice);

            updatedRect = applyMasks(originalDevice, projection,
                                     updatedRect);
        }
    }

    return updatedRect;
}

bool KisLayer::needProjection() const
{
    return false;
}

void KisLayer::copyOriginalToProjection(const KisPaintDeviceSP original,
                                        KisPaintDeviceSP projection,
                                        const QRect& rect) const
{
    KisPainter gc(projection);
    gc.setCompositeOp(colorSpace()->compositeOp(COMPOSITE_COPY));
    gc.bitBlt(rect.topLeft(), original, rect);
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

    if(pos == KisNode::N_FILTHY) {
        bool changeRectVaries;
        changeRect = masksChangeRect(effectMasks(), rect, changeRectVaries);
    }

    return changeRect;
}

QImage KisLayer::createThumbnail(qint32 w, qint32 h)
{
    KisPaintDeviceSP originalDevice = original();

    return originalDevice ?
           originalDevice->createThumbnail(w, h, KoColorConversionTransformation::IntentPerceptual, KoColorConversionTransformation::BlackpointCompensation) : QImage();
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

QRect KisLayer::extent() const
{
    KisPaintDeviceSP originalDevice = original();
    return originalDevice ? originalDevice->extent() : QRect();
}

QRect KisLayer::exactBounds() const
{
    KisPaintDeviceSP originalDevice = original();
    return originalDevice ? originalDevice->exactBounds() : QRect();
}


void KisLayer::setPreviewMask(KisEffectMaskSP mask)
{
    m_d->previewMask = mask;
    m_d->previewMask->setParent(this);
}

KisEffectMaskSP KisLayer::previewMask() const
{
    return m_d->previewMask;
}

void KisLayer::removePreviewMask()
{
    m_d->previewMask = 0;
}

KisLayerSP KisLayer::parentLayer() const
{
    return dynamic_cast<KisLayer*>(parent().data());
}

KisMetaData::Store* KisLayer::metaData()
{
    return m_d->metaDataStore;
}

#include "kis_layer.moc"
