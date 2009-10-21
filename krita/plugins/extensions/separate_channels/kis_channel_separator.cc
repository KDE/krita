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

#include <klocale.h>
#include <kiconloader.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <kis_debug.h>
#include <kgenericfactory.h>
#include <knuminput.h>
#include <kfiledialog.h>

#include <KoFilterManager.h>
#include <KoProgressUpdater.h>
#include <KoUpdater.h>

#include <kis_doc2.h>
#include <kis_image.h>

#include <kis_iterators_pixel.h>
#include <kis_layer.h>
#include <kis_paint_layer.h>
#include <kis_group_layer.h>
#include <kis_transaction.h>
#include <kis_undo_adapter.h>
#include <kis_global.h>
#include <kis_types.h>

#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>
#include <kis_view2.h>
#include <kis_paint_device.h>
#include <KoChannelInfo.h>
#include <kis_layer_manager.h>
#include <kis_node_commands_adapter.h>

KisChannelSeparator::KisChannelSeparator(KisView2 * view)
        : m_view(view)
{
}

void KisChannelSeparator::separate(KoUpdater * progressUpdater, enumSepAlphaOptions alphaOps, enumSepSource sourceOps, enumSepOutput outputOps, bool downscale, bool toColor)
{
    KisImageWSP image = m_view->image();
    if (!image) return;

    KisPaintDeviceSP src = m_view->activeDevice();
    if (!src) return;

    progressUpdater->setProgress(1);

    const KoColorSpace * dstCs = 0;

    quint32 numberOfChannels = src->channelCount();
    const KoColorSpace * srcCs  = src->colorSpace();
    QList<KoChannelInfo *> channels = srcCs->channels();
    // Use the flattened image, if required
    switch (sourceOps) {
    case(ALL_LAYERS):
        src = image->mergedImage();
        break;
    default:
        break;
    }
    vKisPaintDeviceSP layers;

    QList<KoChannelInfo *>::const_iterator begin = channels.constBegin();
    QList<KoChannelInfo *>::const_iterator end = channels.constEnd();

    QRect rect = src->exactBounds();

    m_view->image()->lock();
    int i = 0;
    quint32 channelIndex = 0;
    for (QList<KoChannelInfo *>::const_iterator it = begin; it != end; ++it, ++channelIndex) {

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
                dev = new KisPaintDevice(KoColorSpaceRegistry::instance()->colorSpace("GRAYA", 0));
            } else {
                dev = new KisPaintDevice(KoColorSpaceRegistry::instance()->colorSpace("GRAYA16", 0));
                destSize = 2;
            }
        }

        dstCs = dev->colorSpace();

        layers.push_back(dev);

        KisHLineConstIteratorPixel srcIt = src->createHLineConstIterator(rect.x(), rect.y(), rect.width());
        KisHLineIteratorPixel dstIt = dev->createHLineIterator(rect.x(), rect.y(), rect.width());

        for (qint32 row = 0; row < rect.height(); ++row) {


            while (! srcIt.isDone()) {
                if (srcIt.isSelected()) {
                    if (toColor) {
                        dstCs->singleChannelPixel(dstIt.rawData(), srcIt.rawData(), channelIndex);

                        if (alphaOps == COPY_ALPHA_TO_SEPARATIONS) {
                            //dstCs->setAlpha(dstIt.rawData(), srcIt.rawData()[srcAlphaPos], 1);
                            dstCs->setAlpha(dstIt.rawData(), srcCs->alpha(srcIt.rawData()), 1);
                        } else {
                            dstCs->setAlpha(dstIt.rawData(), OPACITY_OPAQUE, 1);
                        }
                    } else {

                        // To grayscale

                        // Decide whether we need downscaling
                        if (channelSize == 1 && destSize == 1) {

                            // Both 8-bit channels
                            dstIt.rawData()[0] = srcIt.rawData()[channelPos];

                            if (alphaOps == COPY_ALPHA_TO_SEPARATIONS) {
                                dstCs->setAlpha(dstIt.rawData(), srcCs->alpha(srcIt.rawData()), 1);
                            } else {
                                dstCs->setAlpha(dstIt.rawData(), OPACITY_OPAQUE, 1);
                            }
                        } else if (channelSize == 2 && destSize == 2) {

                            // Both 16-bit
                            dstIt.rawData()[0] = srcIt.rawData()[channelPos];
                            dstIt.rawData()[1] = srcIt.rawData()[channelPos + 1];

                            if (alphaOps == COPY_ALPHA_TO_SEPARATIONS) {
                                dstCs->setAlpha(dstIt.rawData(), srcCs->alpha(srcIt.rawData()), 1);
                            } else {
                                dstCs->setAlpha(dstIt.rawData(), OPACITY_OPAQUE, 1);
                            }
                        } else if (channelSize != 1 && destSize == 1) {
                            // Downscale
                            memset(dstIt.rawData(), srcCs->scaleToU8(srcIt.rawData(), channelPos), 1);

                            // XXX: Do alpha
                            dstCs->setAlpha(dstIt.rawData(), OPACITY_OPAQUE, 1);
                        } else if (channelSize != 2 && destSize == 2) {
                            // Upscale
                            dstIt.rawData()[0] = srcCs->scaleToU8(srcIt.rawData(), channelPos);

                            // XXX: Do alpha
                            dstCs->setAlpha(dstIt.rawData(), OPACITY_OPAQUE, 1);

                        }
                    }
                }
                ++dstIt;
                ++srcIt;
            }
            dstIt.nextRow();
            srcIt.nextRow();
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

        KisUndoAdapter * undo = 0;
        if (outputOps == TO_LAYERS && (undo = image->undoAdapter()) && undo->undo()) {
            undo->beginMacro(i18n("Separate Image"));
        }

        // Flatten the image if required
        switch (sourceOps) {
        case(ALL_LAYERS):
            image->flatten();
            break;
        default:
            break;
        }
        KisNodeCommandsAdapter adapter(m_view);

        for (QList<KoChannelInfo *>::const_iterator it = begin; it != end; ++it) {

            KoChannelInfo * ch = (*it);

            if (ch->channelType() == KoChannelInfo::ALPHA && alphaOps != CREATE_ALPHA_SEPARATION) {
                // Don't make an separate separation of the alpha channel if the user didn't ask for it.
                continue;
            }

            if (outputOps == TO_LAYERS) {
                KisPaintLayerSP l = KisPaintLayerSP(new KisPaintLayer(image.data(), ch->name(), OPACITY_OPAQUE, *deviceIt));
                if (undo && undo->undo()) {
                    adapter.addNode(l.data(), image->rootLayer(), 0);
                } else {
                    image->addNode(l.data(), image->rootLayer());
                }
            } else {
                QStringList listMimeFilter = KoFilterManager::mimeFilter("application/x-krita", KoFilterManager::Export);
                QString mimelist = listMimeFilter.join(" ");

                KFileDialog fd(QString(), mimelist, m_view);
                fd.setObjectName("Export Layer");
                fd.setCaption(i18n("Export Layer") + '(' + ch->name() + ')');
                fd.setMimeFilter(listMimeFilter);
                fd.setOperationMode(KFileDialog::Saving);
                fd.setUrl(KUrl(ch->name()));
                if (!fd.exec()) return;

                KUrl url = fd.selectedUrl();
                QString mimefilter = fd.currentMimeFilter();

                if (url.isEmpty())
                    return;
                if (mimefilter.isNull()) {
                    KMimeType::Ptr mime = KMimeType::findByUrl(url);
                    mimefilter = mime->name();
                }


                KisPaintLayerSP l = KisPaintLayerSP(new KisPaintLayer(image.data(), ch->name(), OPACITY_OPAQUE, *deviceIt));
                QRect r = l->exactBounds();

                KisDoc2 d;
                d.prepareForImport();

                KisImageWSP dst = KisImageWSP(new KisImage(d.undoAdapter(), r.width(), r.height(), (*deviceIt)->colorSpace(), l->name()));
                d.setCurrentImage(dst);
                dst->addNode(l->clone().data(), dst->rootLayer());

                d.setOutputMimeType(mimefilter.toLatin1());
                d.exportDocument(url);

            }

            ++deviceIt;
        }

        if (outputOps == TO_LAYERS && undo && undo->undo()) {
            undo->endMacro();
        }
        m_view->image()->unlock();
        image->setModified();
    }

}
