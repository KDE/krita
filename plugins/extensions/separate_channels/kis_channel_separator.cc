/*
 * This file is part of Krita
 *
 * SPDX-FileCopyrightText: 2005 Michael Thaler <michael.thaler@physik.tu-muenchen.de>
 *
 * ported from Gimp, SPDX-FileCopyrightText: 1997 Eiichi Takamori <taka@ma1.seikyou.ne.jp>
 * original pixelize.c for GIMP 0.54 by Tracy Scott
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_channel_separator.h"

#include <limits.h>
#include <stdlib.h>
#include <vector>

#include <QStandardPaths>

#include <klocalizedstring.h>
#include <kis_debug.h>
#include <kpluginfactory.h>


#include <KisImportExportManager.h>
#include <KoUpdater.h>
#include <KoFileDialog.h>
#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>
#include <KoChannelInfo.h>
#include <KoColorModelStandardIds.h>

#include <KisDocument.h>
#include <kis_image.h>
#include <kis_layer.h>
#include <kis_paint_layer.h>
#include <kis_group_layer.h>
#include <kis_transaction.h>
#include <kis_undo_adapter.h>
#include <kis_global.h>
#include <kis_types.h>
#include "kis_iterator_ng.h"
#include <KisPart.h>
#include <KisViewManager.h>
#include <kis_paint_device.h>
#include <kis_node_manager.h>
#include <kis_node_commands_adapter.h>
#include <KisMimeDatabase.h>
#include "kis_image_barrier_locker.h"

KisChannelSeparator::KisChannelSeparator(KisViewManager * view)
    : m_viewManager(view)
{
}

void KisChannelSeparator::separate(KoUpdater * progressUpdater, enumSepAlphaOptions alphaOps, enumSepSource sourceOps, bool downscale, bool toColor, bool activateCurrentChannel)
{
    KisImageSP image = m_viewManager->image();
    if (!image) return;

    KisPaintDeviceSP src;

    // Use the flattened image, if required
    switch (sourceOps) {
    case ALL_LAYERS:
        // the content will be locked later
        src = image->projection();
        break;
    case CURRENT_LAYER:
        src = m_viewManager->activeDevice();
        break;
    default:
        break;
    }

    if (!src) return;

    progressUpdater->setProgress(1);

    const KoColorSpace * dstCs = 0;

    quint32 numberOfChannels = src->channelCount();
    const KoColorSpace * srcCs  = src->colorSpace();
    QList<KoChannelInfo *> channels = srcCs->channels();
    vKisPaintDeviceSP paintDevices;

    QRect rect = src->exactBounds();

    KisImageBarrierLocker locker(image);
    int i = 0;
    for (QList<KoChannelInfo *>::const_iterator it =  channels.constBegin(); it != channels.constEnd(); ++it) {

        KoChannelInfo *ch = (*it);
        if (ch->channelType() == KoChannelInfo::ALPHA && alphaOps != CREATE_ALPHA_SEPARATION) {
            continue;
        }

        qint32 channelSize = ch->size();
        qint32 channelPos = ch->pos();
        qint32 destSize = 1;

        KisPaintDeviceSP dev;
        if (toColor) {
            // We don't downscale if we separate to color channels
            dev = new KisPaintDevice(srcCs);
        }
        else {
            if (channelSize == 1 || downscale) {
                dev = new KisPaintDevice(KoColorSpaceRegistry::instance()->colorSpace(GrayAColorModelID.id(), Integer8BitsColorDepthID.id(), 0));
            } else {
                dev = new KisPaintDevice(KoColorSpaceRegistry::instance()->colorSpace(GrayAColorModelID.id(), Integer16BitsColorDepthID.id(), 0));
                destSize = 2;
            }
        }

        dstCs = dev->colorSpace();

        paintDevices.push_back(dev);

        KisHLineConstIteratorSP srcIt = src->createHLineConstIteratorNG(rect.x(), rect.y(), rect.width());
        KisHLineIteratorSP dstIt = dev->createHLineIteratorNG(rect.x(), rect.y(), rect.width());

        for (qint32 row = 0; row < rect.height(); ++row) {
            do {
                if (toColor) {
                    dstCs->singleChannelPixel(dstIt->rawData(), srcIt->oldRawData(), channelPos);

                    if (alphaOps == COPY_ALPHA_TO_SEPARATIONS) {
                        //dstCs->setAlpha(dstIt->rawData(), srcIt->oldRawData()[srcAlphaPos], 1);
                        dstCs->setOpacity(dstIt->rawData(), srcCs->opacityU8(srcIt->oldRawData()), 1);
                    } else {
                        dstCs->setOpacity(dstIt->rawData(), OPACITY_OPAQUE_U8, 1);
                    }
                } else {

                    // To grayscale

                    // Decide whether we need downscaling
                    if (channelSize == 1 && destSize == 1) {

                        // Both 8-bit channels
                        dstIt->rawData()[0] = srcIt->oldRawData()[channelPos];

                        if (alphaOps == COPY_ALPHA_TO_SEPARATIONS) {
                            dstCs->setOpacity(dstIt->rawData(), srcCs->opacityU8(srcIt->oldRawData()), 1);
                        } else {
                            dstCs->setOpacity(dstIt->rawData(), OPACITY_OPAQUE_U8, 1);
                        }
                    } else if (channelSize == 2 && destSize == 2) {

                        // Both 16-bit
                        dstIt->rawData()[0] = srcIt->oldRawData()[channelPos];
                        dstIt->rawData()[1] = srcIt->oldRawData()[channelPos + 1];

                        if (alphaOps == COPY_ALPHA_TO_SEPARATIONS) {
                            dstCs->setOpacity(dstIt->rawData(), srcCs->opacityU8(srcIt->oldRawData()), 1);
                        } else {
                            dstCs->setOpacity(dstIt->rawData(), OPACITY_OPAQUE_U8, 1);
                        }
                    } else if (channelSize != 1 && destSize == 1) {
                        // Downscale
                        memset(dstIt->rawData(), srcCs->scaleToU8(srcIt->oldRawData(), channelPos), 1);

                        // XXX: Do alpha
                        dstCs->setOpacity(dstIt->rawData(), OPACITY_OPAQUE_U8, 1);
                    } else if (channelSize != 2 && destSize == 2) {
                        // Upscale
                        dstIt->rawData()[0] = srcCs->scaleToU8(srcIt->oldRawData(), channelPos);

                        // XXX: Do alpha
                        dstCs->setOpacity(dstIt->rawData(), OPACITY_OPAQUE_U8, 1);

                    }
                }

            } while (dstIt->nextPixel() && srcIt->nextPixel());
            dstIt->nextRow();
            srcIt->nextRow();
        }
        ++i;

        progressUpdater->setProgress((i * 100) / numberOfChannels);
        if (progressUpdater->interrupted()) {
            break;
        }
    }

    vKisPaintDeviceSP::const_iterator paintDeviceIterator = paintDevices.cbegin();

    if (!progressUpdater->interrupted()) {
        KisNodeCommandsAdapter adapter(m_viewManager);
        adapter.beginMacro(kundo2_i18n("Separate Image"));

        for (QList<KoChannelInfo *>::const_iterator it =  channels.constBegin(); it != channels.constEnd(); ++it) {

            KoChannelInfo *ch = (*it);
            if (ch->channelType() == KoChannelInfo::ALPHA && alphaOps != CREATE_ALPHA_SEPARATION) {
                // Don't make an separate separation of the alpha channel if the user didn't ask for it.
                continue;
            }

            KisPaintLayerSP l = new KisPaintLayer(image, ch->name(), OPACITY_OPAQUE_U8, *paintDeviceIterator);

            if (toColor && activateCurrentChannel) {
                QBitArray channelFlags(channels.count());
                int i = 0;
                for (QList<KoChannelInfo *>::const_iterator it2 =  channels.constBegin(); it2 != channels.constEnd(); ++it2) {
                    channelFlags.setBit(i, (it == it2));
                    i++;
                }
                l->setChannelFlags(channelFlags);
            }

            adapter.addNode(l.data(), image->rootLayer(), 0);
            ++paintDeviceIterator;
        }

        adapter.endMacro();
    }

    progressUpdater->setProgress(100);


}
