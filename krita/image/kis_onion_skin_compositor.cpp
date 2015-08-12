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

#include <kglobal.h>

#include "kis_paint_device.h"
#include "kis_painter.h"
#include "KoColor.h"
#include "KoColorSpace.h"
#include "KoCompositeOpRegistry.h"

#include "kis_image_config.h"
#include "kis_config_notifier.h"
#include "kis_raster_keyframe_channel.h"


struct KisOnionSkinCompositor::Private
{
    int numberOfSkins;
    int tintFactor;
    QColor backwardTintColor;
    QColor forwardTintColor;
    QVector<int> backwardOpacities;
    QVector<int> forwardOpacities;

    int skinOpacity(int offset)
    {
        KisImageConfig cfg;

        if (offset < 0) {
            while (backwardOpacities.count() < -offset) {
                backwardOpacities.append(cfg.onionSkinOpacity(-(backwardOpacities.count() + 1)));
            }

            return backwardOpacities.at(-offset - 1);
        } else {
           while (forwardOpacities.count() < offset) {
                forwardOpacities.append(cfg.onionSkinOpacity(forwardOpacities.count() + 1));
            }

            return forwardOpacities.at(offset - 1);
        }
    }

    KisPaintDeviceSP setUpTintDevice(const QColor &tintColor, const KoColorSpace *colorSpace)
    {
        KisPaintDeviceSP tintDevice = new KisPaintDevice(colorSpace);
        KoColor color = KoColor(tintColor, colorSpace);
        tintDevice->setDefaultPixel(color.data());
        return tintDevice;
    }

    void compositeFrame(KisRasterKeyframeChannel *keyframes, KisKeyframeSP keyframe, KisPainter &gcFrame, KisPainter &gcDest, KisPaintDeviceSP tintSource, int opacity, const QRect &rect)
    {
        if (keyframe.isNull()) return;

        keyframes->fetchFrame(keyframe, gcFrame.device());

        gcFrame.bitBlt(rect.topLeft(), tintSource, rect);

        gcDest.setOpacity(opacity);
        gcDest.bitBlt(rect.topLeft(), gcFrame.device(), rect);
    }

    void refreshConfig()
    {
        KisImageConfig config;

        numberOfSkins = config.numberOfOnionSkins();
        tintFactor = config.onionSkinTintFactor();
        backwardTintColor = config.onionSkinTintColorBackward();
        forwardTintColor = config.onionSkinTintColorForward();

        backwardOpacities.clear();
        forwardOpacities.clear();
    }
};

KisOnionSkinCompositor *KisOnionSkinCompositor::instance()
{
    K_GLOBAL_STATIC(KisOnionSkinCompositor, s_instance);
    return s_instance;
}

KisOnionSkinCompositor::KisOnionSkinCompositor()
    : m_d(new Private)
{
    m_d->refreshConfig();
}

KisOnionSkinCompositor::~KisOnionSkinCompositor()
{}

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
        if (!keyframeBck.isNull()) {
            keyframeBck = keyframes->previousKeyframe(keyframeBck);
            m_d->compositeFrame(keyframes, keyframeBck, gcFrame, gcDest, backwardTintDevice, m_d->skinOpacity(-offset), rect);
        }

        if (!keyframeFwd.isNull()) {
            keyframeFwd = keyframes->nextKeyframe(keyframeFwd);
            m_d->compositeFrame(keyframes, keyframeFwd, gcFrame, gcDest, forwardTintDevice, m_d->skinOpacity(offset), rect);
        }
    }

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
}
