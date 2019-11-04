/*
 *  Copyright (c) 2015 Jouni Pentikäinen <joupent@gmail.com>
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

#include "kis_onion_skin_compositor.h"

#include "kis_paint_device.h"
#include "kis_painter.h"
#include "KoColor.h"
#include "KoColorSpace.h"
#include "KoCompositeOpRegistry.h"
#include "KoColorSpaceConstants.h"

#include "kis_image_config.h"
#include "kis_raster_keyframe_channel.h"

Q_GLOBAL_STATIC(KisOnionSkinCompositor, s_instance)

struct KisOnionSkinCompositor::Private
{
    int numberOfSkins = 0;
    int tintFactor = 0;
    QColor backwardTintColor;
    QColor forwardTintColor;
    QVector<int> backwardOpacities;
    QVector<int> forwardOpacities;
    int configSeqNo = 0;
    QList<int> colorLabelFilter;

    int skinOpacity(int offset)
    {
        const QVector<int> &bo = backwardOpacities;
        const QVector<int> &fo = forwardOpacities;

        return offset > 0 ? fo[qAbs(offset) - 1] : bo[qAbs(offset) - 1];
    }

    KisPaintDeviceSP setUpTintDevice(const QColor &tintColor, const KoColorSpace *colorSpace)
    {
        KisPaintDeviceSP tintDevice = new KisPaintDevice(colorSpace);
        KoColor color = KoColor(tintColor, colorSpace);
        tintDevice->setDefaultPixel(color);
        return tintDevice;
    }


    KisKeyframeSP getNextFrameToComposite(KisKeyframeChannel *channel, KisKeyframeSP keyframe, bool backwards)
    {
        while (!keyframe.isNull()) {
            keyframe = backwards ? channel->previousKeyframe(keyframe) : channel->nextKeyframe(keyframe);
            if (colorLabelFilter.isEmpty()) {
                return keyframe;
            } else if (!keyframe.isNull()) {
                if (colorLabelFilter.contains(keyframe->colorLabel())) {
                    return keyframe;
                }
            }
        }
        return keyframe;
    }

    void tryCompositeFrame(KisRasterKeyframeChannel *keyframes, KisKeyframeSP keyframe, KisPainter &gcFrame, KisPainter &gcDest, KisPaintDeviceSP tintSource, int opacity, const QRect &rect)
    {
        if (keyframe.isNull() || opacity == OPACITY_TRANSPARENT_U8) return;

        keyframes->fetchFrame(keyframe, gcFrame.device());

        gcFrame.bitBlt(rect.topLeft(), tintSource, rect);

        gcDest.setOpacity(opacity);
        gcDest.bitBlt(rect.topLeft(), gcFrame.device(), rect);
    }

    void refreshConfig()
    {
        KisImageConfig config(true);

        numberOfSkins = config.numberOfOnionSkins();
        tintFactor = config.onionSkinTintFactor();
        backwardTintColor = config.onionSkinTintColorBackward();
        forwardTintColor = config.onionSkinTintColorForward();

        backwardOpacities.resize(numberOfSkins);
        forwardOpacities.resize(numberOfSkins);

        const int mainState = (int) config.onionSkinState(0);
        const qreal scaleFactor = mainState * config.onionSkinOpacity(0) / 255.0;

        for (int i = 0; i < numberOfSkins; i++) {
            int backwardState = (int) config.onionSkinState(-(i + 1));
            int forwardState = (int) config.onionSkinState(i + 1);

            backwardOpacities[i] = scaleFactor * backwardState * config.onionSkinOpacity(-(i + 1));
            forwardOpacities[i] = scaleFactor * forwardState * config.onionSkinOpacity(i + 1);
        }

        configSeqNo++;
    }
};

KisOnionSkinCompositor *KisOnionSkinCompositor::instance()
{
    return s_instance;
}

KisOnionSkinCompositor::KisOnionSkinCompositor()
    : m_d(new Private)
{
    m_d->refreshConfig();
}

KisOnionSkinCompositor::~KisOnionSkinCompositor()
{}

int KisOnionSkinCompositor::configSeqNo() const
{
    return m_d->configSeqNo;
}

void KisOnionSkinCompositor::setColorLabelFilter(QList<int> colors)
{
    m_d->colorLabelFilter = colors;
}

void KisOnionSkinCompositor::composite(const KisPaintDeviceSP sourceDevice, KisPaintDeviceSP targetDevice, const QRect& rect)
{
    KisRasterKeyframeChannel *keyframes = sourceDevice->keyframeChannel();

    KisPaintDeviceSP frameDevice = new KisPaintDevice(sourceDevice->colorSpace());
    KisPainter gcFrame(frameDevice);
    QBitArray channelFlags = targetDevice->colorSpace()->channelFlags(true, false);
    gcFrame.setChannelFlags(channelFlags);
    gcFrame.setOpacity(m_d->tintFactor);

    KisPaintDeviceSP backwardTintDevice = m_d->setUpTintDevice(m_d->backwardTintColor, sourceDevice->colorSpace());
    KisPaintDeviceSP forwardTintDevice = m_d->setUpTintDevice(m_d->forwardTintColor, sourceDevice->colorSpace());

    KisPainter gcDest(targetDevice);
    gcDest.setCompositeOp(sourceDevice->colorSpace()->compositeOp(COMPOSITE_BEHIND));

    KisKeyframeSP keyframeBck;
    KisKeyframeSP keyframeFwd;

    int time = sourceDevice->defaultBounds()->currentTime();
    keyframeBck = keyframeFwd = keyframes->activeKeyframeAt(time);

    for (int offset = 1; offset <= m_d->numberOfSkins; offset++) {
        keyframeBck = m_d->getNextFrameToComposite(keyframes, keyframeBck, true);
        keyframeFwd = m_d->getNextFrameToComposite(keyframes, keyframeFwd, false);

        if (!keyframeBck.isNull()) {
            m_d->tryCompositeFrame(keyframes, keyframeBck, gcFrame, gcDest, backwardTintDevice, m_d->skinOpacity(-offset), rect);
        }

        if (!keyframeFwd.isNull()) {
            m_d->tryCompositeFrame(keyframes, keyframeFwd, gcFrame, gcDest, forwardTintDevice, m_d->skinOpacity(offset), rect);
        }
    }

}

QRect KisOnionSkinCompositor::calculateFullExtent(const KisPaintDeviceSP device)
{
    QRect rect;

    KisRasterKeyframeChannel *channel = device->keyframeChannel();
    if (!channel) return rect;

    KisKeyframeSP keyframe = channel->firstKeyframe();

    while (keyframe) {
        rect |= channel->frameExtents(keyframe);
        keyframe = channel->nextKeyframe(keyframe);
    }

    return rect;
}

QRect KisOnionSkinCompositor::calculateExtent(const KisPaintDeviceSP device)
{
    QRect rect;
    KisKeyframeSP keyframeBck;
    KisKeyframeSP keyframeFwd;

    KisRasterKeyframeChannel *channel = device->keyframeChannel();
    keyframeBck = keyframeFwd = channel->activeKeyframeAt(device->defaultBounds()->currentTime());

    for (int offset = 1; offset <= m_d->numberOfSkins; offset++) {
        if (!keyframeBck.isNull()) {
            keyframeBck = channel->previousKeyframe(keyframeBck);

            if (!keyframeBck.isNull()) {
                rect |= channel->frameExtents(keyframeBck);
            }
        }

        if (!keyframeFwd.isNull()) {
            keyframeFwd = channel->nextKeyframe(keyframeFwd);

            if (!keyframeFwd.isNull()) {
                rect |= channel->frameExtents(keyframeFwd);
            }
        }
    }

    return rect;
}

void KisOnionSkinCompositor::configChanged()
{
    m_d->refreshConfig();
    emit sigOnionSkinChanged();
}
