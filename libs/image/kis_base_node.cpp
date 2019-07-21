/*
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

#include "kis_base_node.h"
#include <klocalizedstring.h>

#include <kis_image.h>
#include <kis_icon.h>
#include <KoProperties.h>
#include <KoColorSpace.h>
#include <KoCompositeOpRegistry.h>
#include "kis_paint_device.h"
#include "kis_layer_properties_icons.h"

#include "kis_scalar_keyframe_channel.h"

struct Q_DECL_HIDDEN KisBaseNode::Private
{
    QString compositeOp;
    KoProperties properties;
    KisBaseNode::Property hack_visible; //HACK
    QUuid id;
    QMap<QString, KisKeyframeChannel*> keyframeChannels;
    QScopedPointer<KisScalarKeyframeChannel> opacityChannel;

    bool systemLocked;
    bool collapsed;
    bool supportsLodMoves;
    bool animated;
    bool useInTimeline;
    KisImageWSP image;

    Private(KisImageWSP image)
        : id(QUuid::createUuid())
        , systemLocked(false)
        , collapsed(false)
        , supportsLodMoves(false)
        , animated(false)
        , useInTimeline(false)
        , image(image)
    {
    }

    Private(const Private &rhs)
        : compositeOp(rhs.compositeOp),
          id(QUuid::createUuid()),
          systemLocked(false),
          collapsed(rhs.collapsed),
          supportsLodMoves(rhs.supportsLodMoves),
          animated(rhs.animated),
          useInTimeline(rhs.useInTimeline),
          image(rhs.image)
    {
        QMapIterator<QString, QVariant> iter = rhs.properties.propertyIterator();
        while (iter.hasNext()) {
            iter.next();
            properties.setProperty(iter.key(), iter.value());
        }
    }
};

KisBaseNode::KisBaseNode(KisImageWSP image)
    : m_d(new Private(image))
{
    /**
     * Be cautious! These two calls are vital to warm-up KoProperties.
     * We use it and its QMap in a threaded environment. This is not
     * officially supported by Qt, but our environment guarantees, that
     * there will be the only writer and several readers. Whilst the
     * value of the QMap is boolean and there are no implicit-sharing
     * calls provocated, it is safe to work with it in such an
     * environment.
     */
    setVisible(true, true);
    setUserLocked(false);
    setCollapsed(false);
    setSupportsLodMoves(true);

    m_d->compositeOp = COMPOSITE_OVER;
}


KisBaseNode::KisBaseNode(const KisBaseNode & rhs)
    : QObject()
    , KisShared()
    , m_d(new Private(*rhs.m_d))
{
    if (rhs.m_d->keyframeChannels.size() > 0) {
        Q_FOREACH(QString key, rhs.m_d->keyframeChannels.keys()) {
            KisKeyframeChannel* channel = rhs.m_d->keyframeChannels.value(key);
            if (!channel) {
                continue;
            }

            if (channel->inherits("KisScalarKeyframeChannel")) {
                KisScalarKeyframeChannel* pchannel = qobject_cast<KisScalarKeyframeChannel*>(channel);
                KIS_ASSERT_RECOVER(pchannel) { continue; }

                KisScalarKeyframeChannel* channelNew = new KisScalarKeyframeChannel(*pchannel, 0);
                KIS_ASSERT(channelNew);

                m_d->keyframeChannels.insert(channelNew->id(), channelNew);

                if (KoID(key) == KisKeyframeChannel::Opacity) {
                    m_d->opacityChannel.reset(channelNew);
                }
            }

        }
    }
}

KisBaseNode::~KisBaseNode()
{
    delete m_d;
}

KisPaintDeviceSP KisBaseNode::colorPickSourceDevice() const
{
    return projection();
}

quint8 KisBaseNode::opacity() const
{
    if (m_d->opacityChannel) {
        qreal value = m_d->opacityChannel->currentValue();

        if (!qIsNaN(value)) {
            return value;
        }
    }

    return nodeProperties().intProperty("opacity", OPACITY_OPAQUE_U8);
}

void KisBaseNode::setOpacity(quint8 val)
{
    if (m_d->opacityChannel) {
        KisKeyframeSP activeKeyframe = m_d->opacityChannel->currentlyActiveKeyframe();

        if (activeKeyframe) {
            m_d->opacityChannel->setScalarValue(activeKeyframe, val);
        }
    }

    if (opacity() == val) return;

    setNodeProperty("opacity", val);

    baseNodeInvalidateAllFramesCallback();
}

quint8 KisBaseNode::percentOpacity() const
{
    return int(float(opacity() * 100) / 255 + 0.5);
}

void KisBaseNode::setPercentOpacity(quint8 val)
{
    setOpacity(int(float(val * 255) / 100 + 0.5));
}

const QString& KisBaseNode::compositeOpId() const
{
    return m_d->compositeOp;
}

void KisBaseNode::setCompositeOpId(const QString& compositeOp)
{
    if (m_d->compositeOp == compositeOp) return;

    m_d->compositeOp = compositeOp;
    baseNodeChangedCallback();
    baseNodeInvalidateAllFramesCallback();
}

KisBaseNode::PropertyList KisBaseNode::sectionModelProperties() const
{
    KisBaseNode::PropertyList l;
    l << KisLayerPropertiesIcons::getProperty(KisLayerPropertiesIcons::visible, visible(), m_d->hack_visible.isInStasis, m_d->hack_visible.stateInStasis);
    l << KisLayerPropertiesIcons::getProperty(KisLayerPropertiesIcons::locked, userLocked());
    return l;
}

void KisBaseNode::setSectionModelProperties(const KisBaseNode::PropertyList &properties)
{
    setVisible(properties.at(0).state.toBool());
    m_d->hack_visible = properties.at(0);
    setUserLocked(properties.at(1).state.toBool());
}

const KoProperties & KisBaseNode::nodeProperties() const
{
    return m_d->properties;
}

void KisBaseNode::setNodeProperty(const QString & name, const QVariant & value)
{
    m_d->properties.setProperty(name, value);
    baseNodeChangedCallback();
}

void KisBaseNode::mergeNodeProperties(const KoProperties & properties)
{
    QMapIterator<QString, QVariant> iter = properties.propertyIterator();
    while (iter.hasNext()) {
        iter.next();
        m_d->properties.setProperty(iter.key(), iter.value());
    }
    baseNodeChangedCallback();
    baseNodeInvalidateAllFramesCallback();
}

bool KisBaseNode::check(const KoProperties & properties) const
{
    QMapIterator<QString, QVariant> iter = properties.propertyIterator();
    while (iter.hasNext()) {
        iter.next();
        if (m_d->properties.contains(iter.key())) {
            if (m_d->properties.value(iter.key()) != iter.value())
                return false;
        }
    }
    return true;
}


QImage KisBaseNode::createThumbnail(qint32 w, qint32 h)
{
    try {
        QImage image(w, h, QImage::Format_ARGB32);
        image.fill(0);
        return image;
    } catch (const std::bad_alloc&) {
        return QImage();
    }

}

QImage KisBaseNode::createThumbnailForFrame(qint32 w, qint32 h, int time)
{
    Q_UNUSED(time)
    return createThumbnail(w, h);
}

bool KisBaseNode::visible(bool recursive) const
{
    bool isVisible = m_d->properties.boolProperty(KisLayerPropertiesIcons::visible.id(), true);
    KisBaseNodeSP parentNode = parentCallback();

    return recursive && isVisible && parentNode ?
        parentNode->visible(recursive) : isVisible;
}

void KisBaseNode::setVisible(bool visible, bool loading)
{
    const bool isVisible = m_d->properties.boolProperty(KisLayerPropertiesIcons::visible.id(), true);
    if (!loading && isVisible == visible) return;

    m_d->properties.setProperty(KisLayerPropertiesIcons::visible.id(), visible);
    notifyParentVisibilityChanged(visible);

    if (!loading) {
        baseNodeChangedCallback();
        baseNodeInvalidateAllFramesCallback();
    }
}

bool KisBaseNode::userLocked() const
{
    return m_d->properties.boolProperty(KisLayerPropertiesIcons::locked.id(), false);
}

void KisBaseNode::setUserLocked(bool locked)
{
    const bool isLocked = m_d->properties.boolProperty(KisLayerPropertiesIcons::locked.id(), true);
    if (isLocked == locked) return;

    m_d->properties.setProperty(KisLayerPropertiesIcons::locked.id(), locked);
    baseNodeChangedCallback();
}

bool KisBaseNode::isEditable(bool checkVisibility) const
{
    bool editable = true;
    if (checkVisibility) {
        editable = (visible(false) && !userLocked());
    }
    else {
        editable = (!userLocked());
    }

    if (editable) {
        KisBaseNodeSP parentNode = parentCallback();
        if (parentNode && parentNode != this) {
            editable = parentNode->isEditable(checkVisibility);
        }
    }
    return editable;
}

bool KisBaseNode::hasEditablePaintDevice() const
{
    return paintDevice() && isEditable();
}

void KisBaseNode::setCollapsed(bool collapsed)
{
    m_d->collapsed = collapsed;
}

bool KisBaseNode::collapsed() const
{
    return m_d->collapsed;
}

void KisBaseNode::setColorLabelIndex(int index)
{
    const int currentLabel = colorLabelIndex();

    if (currentLabel == index) return;

    m_d->properties.setProperty(KisLayerPropertiesIcons::colorLabelIndex.id(), index);
    baseNodeChangedCallback();
}

int KisBaseNode::colorLabelIndex() const
{
    return m_d->properties.intProperty(KisLayerPropertiesIcons::colorLabelIndex.id(), 0);
}

QUuid KisBaseNode::uuid() const
{
    return m_d->id;
}

void KisBaseNode::setUuid(const QUuid& id)
{
    m_d->id = id;
    baseNodeChangedCallback();
}

bool KisBaseNode::supportsLodMoves() const
{
    return m_d->supportsLodMoves;
}

void KisBaseNode::setImage(KisImageWSP image)
{
    m_d->image = image;
}

KisImageWSP KisBaseNode::image() const
{
    return m_d->image;
}

bool KisBaseNode::isFakeNode() const
{
    return false;
}

void KisBaseNode::setSupportsLodMoves(bool value)
{
    m_d->supportsLodMoves = value;
}


QMap<QString, KisKeyframeChannel*> KisBaseNode::keyframeChannels() const
{
    return m_d->keyframeChannels;
}

KisKeyframeChannel * KisBaseNode::getKeyframeChannel(const QString &id) const
{
    QMap<QString, KisKeyframeChannel*>::const_iterator i = m_d->keyframeChannels.constFind(id);
    if (i == m_d->keyframeChannels.constEnd()) {
        return 0;
    }
    return i.value();
}

KisKeyframeChannel * KisBaseNode::getKeyframeChannel(const QString &id, bool create)
{
    KisKeyframeChannel *channel = getKeyframeChannel(id);

    if (!channel && create) {
        channel = requestKeyframeChannel(id);

        if (channel) {
            addKeyframeChannel(channel);
        }
    }

    return channel;
}

bool KisBaseNode::isAnimated() const
{
    return m_d->animated;
}

void KisBaseNode::enableAnimation()
{
    m_d->animated = true;
    baseNodeChangedCallback();
}

bool KisBaseNode::useInTimeline() const
{
    return m_d->useInTimeline;
}

void KisBaseNode::setUseInTimeline(bool value)
{
    if (value == m_d->useInTimeline) return;

    m_d->useInTimeline = value;
    baseNodeChangedCallback();
}

void KisBaseNode::addKeyframeChannel(KisKeyframeChannel *channel)
{
    m_d->keyframeChannels.insert(channel->id(), channel);
    emit keyframeChannelAdded(channel);
}

KisKeyframeChannel *KisBaseNode::requestKeyframeChannel(const QString &id)
{
    if (id == KisKeyframeChannel::Opacity.id()) {
        Q_ASSERT(m_d->opacityChannel.isNull());

        KisPaintDeviceSP device = original();

        if (device) {
            KisScalarKeyframeChannel * channel = new KisScalarKeyframeChannel(
                KisKeyframeChannel::Opacity,
                0, 255,
                device->defaultBounds(),
                KisKeyframe::Linear
            );

            m_d->opacityChannel.reset(channel);

            return channel;
        }
    }

    return 0;
}
