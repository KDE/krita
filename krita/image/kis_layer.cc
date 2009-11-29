/*
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
 *  Copyright (c) 2005 Casper Boemann <cbr@boemann.dk>
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


#include <kicon.h>
#include <klocale.h>
#include <QIcon>
#include <QImage>
#include <QBitArray>
#include <QStack>

#include <KoProperties.h>
#include <KoCompositeOp.h>
#include <KoColorSpace.h>

#include "kis_debug.h"
//#include "kis_group_layer.h"
#include "kis_image.h"

#include "kis_painter.h"
#include "kis_mask.h"
#include "kis_effect_mask.h"
#include "kis_transparency_mask.h"
#include "kis_selection_mask.h"
#include "kis_meta_data_store.h"
#include "kis_selection.h"

class KisLayer::Private
{

public:

    KisImageWSP image;
    QBitArray channelFlags;
    KisEffectMaskSP previewMask;
    KisMetaData::Store* metaDataStore;
    KisPaintDeviceSP projection;
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

void KisLayer::setDirty()
{
    setDirty(extent());
}

void KisLayer::setDirty(const QRect & rect)
{
    m_d->image->updateProjection(this, rect);
}

void KisLayer::setDirty(const QRegion & region)
{
    if (region.isEmpty()) return;

    foreach(const QRect & rc, region.rects()) {
        setDirty(rc);
    }
}

KisSelectionMaskSP KisLayer::selectionMask() const
{
    QList<KisNodeSP> masks = childNodes(QStringList("KisSelectionMask"), KoProperties());
    Q_ASSERT(masks.size() <= 1); // Or do we allow more than one selection mask to a layer?
    if (masks.size() == 1) {
        KisSelectionMaskSP selection = dynamic_cast<KisSelectionMask*>(masks[0].data());
        return selection;
    }
    return 0;
}

KisSelectionSP KisLayer::selection() const
{
    KisSelectionMaskSP selMask = selectionMask();
    if (selMask && selMask->visible())
        return selMask->selection();
    else if (m_d->image)
        return m_d->image->globalSelection();
    else
        return 0;
}

///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////

QList<KisEffectMaskSP> KisLayer::effectMasks() const
{
    KoProperties properties;
    properties.setProperty("visible", true);
    QList<KisNodeSP> nodes = childNodes(QStringList("KisEffectMask"), properties);
    QList<KisEffectMaskSP> masks;

    if (m_d->previewMask)
        masks.append(m_d->previewMask);

    foreach(const KisNodeSP& node,  nodes) {
        KisEffectMaskSP mask = dynamic_cast<KisEffectMask*>(const_cast<KisNode*>(node.data()));
        if (mask)
            masks.append(mask);
    }
    return masks;
}

bool KisLayer::hasEffectMasks() const
{
    if (m_d->previewMask) return true;

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
    QRect changeRect;

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

        changeRect = masksChangeRect(masks, requestedRect,
                                     changeRectVaries);

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
        updatedRect = repaintOriginal(originalDevice, updatedRect);
        m_d->projection = 0;
    } else {
        updatedRect = repaintOriginal(originalDevice, updatedRect);

        if (!updatedRect.isEmpty()) {

            if (!m_d->projection ||
                    !(*m_d->projection->colorSpace() == *originalDevice->colorSpace())) {

                /**
                 * If it's needed to create a new projection paint device
                 * let's do it now! (content will be shared)
                 */
                m_d->projection = new KisPaintDevice(*originalDevice);

            } else {
                m_d->projection->setX(originalDevice->x());
                m_d->projection->setY(originalDevice->y());
            }
            updatedRect = applyMasks(originalDevice, m_d->projection,
                                     updatedRect);
        }
        /**
         * FIXME: else { m_d->projection = 0;}
         */
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
    return m_d->projection ? m_d->projection : original();
}

QImage KisLayer::createThumbnail(qint32 w, qint32 h)
{
    KisPaintDeviceSP originalDevice = original();

    return originalDevice ?
           originalDevice->createThumbnail(w, h) : QImage();
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

struct KisIndirectPaintingSupport::Private {
    // To simulate the indirect painting
    KisPaintDeviceSP temporaryTarget;
    const KoCompositeOp* compositeOp;
    quint8 compositeOpacity;
};

void KisIndirectPaintingSupport::setTemporaryTarget(KisPaintDeviceSP t)
{
    d->temporaryTarget = t;
}

void KisIndirectPaintingSupport::setTemporaryCompositeOp(const KoCompositeOp* c)
{
    d->compositeOp = c;
}

void KisIndirectPaintingSupport::setTemporaryOpacity(quint8 o)
{
    d->compositeOpacity = o;
}

KisPaintDeviceSP KisIndirectPaintingSupport::temporaryTarget()
{
    return d->temporaryTarget;
}

const KisPaintDeviceSP KisIndirectPaintingSupport::temporaryTarget() const
{
    return d->temporaryTarget;
}

const KoCompositeOp* KisIndirectPaintingSupport::temporaryCompositeOp() const
{
    return d->compositeOp;
}

quint8 KisIndirectPaintingSupport::temporaryOpacity() const
{
    return d->compositeOpacity;
}

bool KisIndirectPaintingSupport::hasTemporaryTarget() const
{
    return d->temporaryTarget;
}


KisIndirectPaintingSupport::KisIndirectPaintingSupport() : d(new Private)
{
    d->compositeOp = 0;
}
KisIndirectPaintingSupport::~KisIndirectPaintingSupport()
{
    delete d;
}

#include "kis_layer.moc"
