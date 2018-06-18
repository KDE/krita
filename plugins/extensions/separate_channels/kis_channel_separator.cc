/*
 * This file is part of Krita
 *
 * Copyright (c) 2005 Michael Thaler <michael.thaler@physik.tu-muenchen.de>
 *
 * ported from Gimp, Copyright (C) 1997 Eiichi Takamori <taka@ma1.seikyou.ne.jp>
 * original pixelize.c for GIMP 0.54 by Tracy Scott
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

KisChannelSeparator::KisChannelSeparator(KisViewManager * view)
        : m_viewManager(view)
{
}

void KisChannelSeparator::separate(KoUpdater * progressUpdater, enumSepAlphaOptions alphaOps, enumSepSource sourceOps, enumSepOutput outputOps, bool downscale, bool toColor)
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
    vKisPaintDeviceSP layers;

    QList<KoChannelInfo *>::const_iterator begin = channels.constBegin();
    QList<KoChannelInfo *>::const_iterator end = channels.constEnd();

    QRect rect = src->exactBounds();

    image->lock();
    int i = 0;
    for (QList<KoChannelInfo *>::const_iterator it = begin; it != end; ++it) {

        KoChannelInfo * ch = (*it);

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
        } else {
            if (channelSize == 1 || downscale) {
                dev = new KisPaintDevice(KoColorSpaceRegistry::instance()->colorSpace(GrayAColorModelID.id(), Integer8BitsColorDepthID.id(), 0));
            } else {
                dev = new KisPaintDevice(KoColorSpaceRegistry::instance()->colorSpace(GrayAColorModelID.id(), Integer16BitsColorDepthID.id(), 0));
                destSize = 2;
            }
        }

        dstCs = dev->colorSpace();

        layers.push_back(dev);

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

    vKisPaintDeviceSP_it deviceIt = layers.begin();

    progressUpdater->setProgress(100);

    if (!progressUpdater->interrupted()) {

        KisUndoAdapter * undo = image->undoAdapter();
        if (outputOps == TO_LAYERS) {
            undo->beginMacro(kundo2_i18n("Separate Image"));
        }

        // Flatten the image if required
        switch (sourceOps) {
        case(ALL_LAYERS):
            image->flatten();
            break;
        default:
            break;
        }
        KisNodeCommandsAdapter adapter(m_viewManager);

        for (QList<KoChannelInfo *>::const_iterator it = begin; it != end; ++it) {

            KoChannelInfo * ch = (*it);

            if (ch->channelType() == KoChannelInfo::ALPHA && alphaOps != CREATE_ALPHA_SEPARATION) {
                // Don't make an separate separation of the alpha channel if the user didn't ask for it.
                continue;
            }

            if (outputOps == TO_LAYERS) {
                KisPaintLayerSP l = KisPaintLayerSP(new KisPaintLayer(image.data(), ch->name(), OPACITY_OPAQUE_U8, *deviceIt));
                adapter.addNode(l.data(), image->rootLayer(), 0);
            }
            else {
                KoFileDialog dialog(m_viewManager->mainWindow(), KoFileDialog::SaveFile, "OpenDocument");
                dialog.setCaption(i18n("Export Layer") + '(' + ch->name() + ')');
                dialog.setDefaultDir(QStandardPaths::writableLocation(QStandardPaths::PicturesLocation));
                dialog.setMimeTypeFilters(KisImportExportManager::supportedMimeTypes(KisImportExportManager::Export));
                QUrl url = QUrl::fromUserInput(dialog.filename());

                if (url.isEmpty())
                    return;

                const QString mimeType = KisMimeDatabase::mimeTypeForFile(url.toLocalFile(), false);

                KisPaintLayerSP l = KisPaintLayerSP(new KisPaintLayer(image.data(), ch->name(), OPACITY_OPAQUE_U8, *deviceIt));
                QRect r = l->exactBounds();

                KisDocument *d = KisPart::instance()->createDocument();

                KisImageWSP dst = KisImageWSP(new KisImage(d->createUndoStore(), r.width(), r.height(), (*deviceIt)->colorSpace(), l->name()));
                d->setCurrentImage(dst);
                dst->addNode(l->clone().data(), dst->rootLayer());

                d->exportDocumentSync(url, mimeType.toLatin1());

                delete d;

            }

            ++deviceIt;
        }

        if (outputOps == TO_LAYERS) {
            undo->endMacro();
        }
        image->unlock();
        image->setModified();
    }
}
