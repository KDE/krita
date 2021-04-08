/*
 *  SPDX-FileCopyrightText: 2015 Jouni Pentikäinen <joupent@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
    QSet<int> colorLabelFilter;

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


    KisRasterKeyframeSP getNextFrameToComposite(KisKeyframeChannel *channel, int &outFrame, bool backwards) // TODO: Double-check this function... outFrame might be weird?
    {
        while (!channel->keyframeAt(outFrame).isNull()) {
            outFrame = backwards ? channel->previousKeyframeTime(outFrame) : channel->nextKeyframeTime(outFrame);
            if (colorLabelFilter.isEmpty()) {
                return channel->keyframeAt<KisRasterKeyframe>(outFrame);
            } else if (channel->keyframeAt<KisRasterKeyframe>(outFrame)) {
                if (colorLabelFilter.contains(channel->keyframeAt(outFrame)->colorLabel())) {
                    return channel->keyframeAt<KisRasterKeyframe>(outFrame);
                }
            }
        }
        return channel->keyframeAt<KisRasterKeyframe>(outFrame);
    }

    void tryCompositeFrame(KisRasterKeyframeSP keyframe, KisPainter &gcFrame, KisPainter &gcDest, KisPaintDeviceSP tintSource, int opacity, const QRect &rect)
    {
        if (keyframe.isNull() || opacity == OPACITY_TRANSPARENT_U8) return;

        keyframe->writeFrameToDevice(gcFrame.device());

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

void KisOnionSkinCompositor::setColorLabelFilter(QSet<int> colors)
{
    m_d->colorLabelFilter = colors;
}

QSet<int> KisOnionSkinCompositor::colorLabelFilter()
{
    return m_d->colorLabelFilter;
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

    int keyframeTimeBck;
    int keyframeTimeFwd;

    int time = sourceDevice->defaultBounds()->currentTime();

    if (!keyframes) { // it happens when you try to show onion skins on non-animated layer with opacity keyframes
        return;
    }

    keyframeTimeBck = keyframeTimeFwd = keyframes->activeKeyframeTime(time);

    for (int offset = 1; offset <= m_d->numberOfSkins; offset++) {
        KisRasterKeyframeSP backKeyframe = m_d->getNextFrameToComposite(keyframes, keyframeTimeBck, true);
        KisRasterKeyframeSP forwardKeyframe = m_d->getNextFrameToComposite(keyframes, keyframeTimeFwd, false);

        if (!backKeyframe.isNull()) {
            m_d->tryCompositeFrame(backKeyframe, gcFrame, gcDest, backwardTintDevice, m_d->skinOpacity(-offset), rect);
        }

        if (!forwardKeyframe.isNull()) {
            m_d->tryCompositeFrame(forwardKeyframe, gcFrame, gcDest, forwardTintDevice, m_d->skinOpacity(offset), rect);
        }
    }

}

QRect KisOnionSkinCompositor::calculateFullExtent(const KisPaintDeviceSP device)
{
    QRect rect;

    KisRasterKeyframeChannel *channel = device->keyframeChannel();
    if (!channel) return rect;

    int currentKeyTime = channel->firstKeyframeTime();

    while (channel->keyframeAt(currentKeyTime)) {
        rect |= channel->frameExtents(channel->keyframeAt(currentKeyTime));
        currentKeyTime = channel->nextKeyframeTime(currentKeyTime);
    }

    return rect;
}

QRect KisOnionSkinCompositor::calculateExtent(const KisPaintDeviceSP device)
{
    QRect rect;
    int keyframeTimeBack;
    int keyframeTimeFwd;

    KisRasterKeyframeChannel *channel = device->keyframeChannel(); //TODO: take in channel instead of device...?

    if (!channel) { // it happens when you try to show onion skins on non-animated layer with opacity keyframes
        return rect;
    }

    keyframeTimeBack = keyframeTimeFwd = channel->activeKeyframeTime();

    for (int offset = 1; offset <= m_d->numberOfSkins; offset++) {
        if (channel->keyframeAt(keyframeTimeBack)) {
            keyframeTimeBack = channel->previousKeyframeTime(keyframeTimeBack);

            if (channel->keyframeAt(keyframeTimeBack)) {
                rect |= channel->frameExtents(channel->keyframeAt(keyframeTimeBack));
            }
        }

        if (channel->keyframeAt(keyframeTimeFwd)) {
            keyframeTimeFwd = channel->nextKeyframeTime(keyframeTimeFwd);

            if (channel->keyframeAt(keyframeTimeFwd)) {
                rect |= channel->frameExtents(channel->keyframeAt(keyframeTimeFwd));
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
