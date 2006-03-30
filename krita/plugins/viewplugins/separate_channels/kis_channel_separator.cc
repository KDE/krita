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
#include <limits.h>

#include <stdlib.h>
#include <vector>

#include <klocale.h>
#include <kiconloader.h>
#include <kinstance.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <ktempfile.h>
#include <kdebug.h>
#include <kgenericfactory.h>
#include <knuminput.h>
#include <kfiledialog.h>


#include <KoFilterManager.h>

#include <kis_doc.h>
#include <kis_image.h>
#include "kis_meta_registry.h"
#include <kis_iterators_pixel.h>
#include <kis_layer.h>
#include <kis_paint_layer.h>
#include <kis_group_layer.h>
#include <kis_transaction.h>
#include <kis_undo_adapter.h>
#include <kis_global.h>
#include <kis_types.h>
#include <kis_progress_subject.h>
#include <kis_progress_display_interface.h>
#include <kis_colorspace.h>
#include <kis_colorspace_factory_registry.h>
#include <kis_view.h>
#include <kis_paint_device.h>
#include <kis_channelinfo.h>

#include "kis_channel_separator.h"

KisChannelSeparator::KisChannelSeparator(KisView * view)
    : m_view(view)
{
}

void KisChannelSeparator::separate(KisProgressDisplayInterface * progress, enumSepAlphaOptions alphaOps, enumSepSource sourceOps, enumSepOutput outputOps, bool downscale, bool toColor)
{
    KisImageSP image = m_view->canvasSubject()->currentImg();
    if (!image) return;

    KisLayerSP layer = image->activeLayer();
    if (!layer) return;

    KisPaintDeviceSP src = image->activeDevice();
    if (!src) return;

    m_cancelRequested = false;
    if ( progress )
        progress->setSubject(this, true, true);
    emit notifyProgressStage(i18n("Separating image..."), 0);

    KisColorSpace * dstCs = 0;

    quint32 numberOfChannels = src->nChannels();
    KisColorSpace * srcCs  = src->colorSpace();
    Q3ValueVector<KisChannelInfo *> channels = srcCs->channels();

    // Use the flattened image, if required
    switch(sourceOps) {

        case(ALL_LAYERS):
            src = image->mergedImage();
            break;
        default:
            break;
    }

    vKisPaintDeviceSP layers;

    Q3ValueVector<KisChannelInfo *>::const_iterator begin = channels.begin();
    Q3ValueVector<KisChannelInfo *>::const_iterator end = channels.end();


    QRect rect = src->exactBounds();

    int i = 0;
    quint32 channelIndex = 0;
    for (Q3ValueVector<KisChannelInfo *>::const_iterator it = begin; it != end; ++it, ++channelIndex)
    {

        KisChannelInfo * ch = (*it);

        if (ch->channelType() == KisChannelInfo::ALPHA && alphaOps != CREATE_ALPHA_SEPARATION) {
            continue;
        }

        qint32 channelSize = ch->size();
        qint32 channelPos = ch->pos();
        qint32 destSize = 1;

        KisPaintDeviceSP dev;
        if (toColor) {
            // We don't downscale if we separate to color channels
            dev = new KisPaintDevice(srcCs, "color separations");
        }
        else {
            if (channelSize == 1 || downscale) {
                dev = new KisPaintDevice( KisMetaRegistry::instance()->csRegistry()->getColorSpace(KisID("GRAYA",""),"" ), "8 bit grayscale sep");
            }
            else {
                dev = new KisPaintDevice( KisMetaRegistry::instance()->csRegistry()->getColorSpace(KisID("GRAYA16",""),"" ), "16 bit grayscale sep");
                destSize = 2;
            }
        }

        dstCs = dev->colorSpace();

        layers.push_back(dev);

        for (qint32 row = 0; row < rect.height(); ++row) {

            KisHLineIteratorPixel srcIt = src->createHLineIterator(rect.x(), rect.y() + row, rect.width(), false);
            KisHLineIteratorPixel dstIt = dev->createHLineIterator(rect.x(), rect.y() + row, rect.width(), true);

            while( ! srcIt.isDone() )
            {
                if (srcIt.isSelected())
                {
                    if (toColor) {
                        dstCs->getSingleChannelPixel(dstIt.rawData(), srcIt.rawData(), channelIndex);

                        if (alphaOps == COPY_ALPHA_TO_SEPARATIONS) {
                            //dstCs->setAlpha(dstIt.rawData(), srcIt.rawData()[srcAlphaPos], 1);
                            dstCs->setAlpha(dstIt.rawData(), srcCs->getAlpha(srcIt.rawData()), 1);
                        }
                        else {
                            dstCs->setAlpha(dstIt.rawData(), OPACITY_OPAQUE, 1);
                        }
                    }
                    else {

                        // To grayscale

                        // Decide wether we need downscaling
                        if (channelSize == 1 && destSize == 1) {

                            // Both 8-bit channels
                            dstIt.rawData()[0] = srcIt.rawData()[channelPos];

                            if (alphaOps == COPY_ALPHA_TO_SEPARATIONS) {
                                dstCs->setAlpha(dstIt.rawData(), srcCs->getAlpha(srcIt.rawData()), 1);
                            }
                            else {
                                dstCs->setAlpha(dstIt.rawData(), OPACITY_OPAQUE, 1);
                            }
                        }
                        else if (channelSize == 2 && destSize == 2) {

                            // Both 16-bit
                            dstIt.rawData()[0] = srcIt.rawData()[channelPos];
                            dstIt.rawData()[1] = srcIt.rawData()[channelPos + 1];

                            if (alphaOps == COPY_ALPHA_TO_SEPARATIONS) {
                                dstCs->setAlpha(dstIt.rawData(), srcCs->getAlpha(srcIt.rawData()), 1);
                            }
                            else {
                                dstCs->setAlpha(dstIt.rawData(), OPACITY_OPAQUE, 1);
                            }
                        }
                        else if (channelSize != 1 && destSize == 1) {
                            // Downscale
                            memset(dstIt.rawData(), srcCs->scaleToU8(srcIt.rawData(), channelPos), 1);

                            // XXX: Do alpha
                            dstCs->setAlpha(dstIt.rawData(), OPACITY_OPAQUE, 1);
                        }
                        else if (channelSize != 2 && destSize == 2) {
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
        }
        ++i;

        emit notifyProgress((i * 100) / numberOfChannels);
        if (m_cancelRequested) {
            break;
        }
    }

    vKisPaintDeviceSP_it deviceIt = layers.begin();

    emit notifyProgressDone();

    if (!m_cancelRequested) {

        KisUndoAdapter * undo = 0;
        if ((undo = image->undoAdapter()) && undo->undo()) {
            undo->beginMacro(i18n("Separate Image"));
        }

        // Flatten the image if required
        switch(sourceOps) {
            case(ALL_LAYERS):
                image->flatten();
                break;
            default:
                break;
        }

        for (Q3ValueVector<KisChannelInfo *>::const_iterator it = begin; it != end; ++it)
        {

            KisChannelInfo * ch = (*it);

            if (ch->channelType() == KisChannelInfo::ALPHA && alphaOps != CREATE_ALPHA_SEPARATION) {
            // Don't make an separate separation of the alpha channel if the user didn't ask for it.
                continue;
            }

            if (outputOps == TO_LAYERS) {
                KisPaintLayerSP l = new KisPaintLayer( image, ch->name(), OPACITY_OPAQUE, *deviceIt);
                image->addLayer( dynamic_cast<KisLayer*>(l.data()), image->rootLayer(), 0);
            }
            else {
                QStringList listMimeFilter = KoFilterManager::mimeFilter("application/x-krita", KoFilterManager::Export);
                QString mimelist = listMimeFilter.join(" ");

                KFileDialog fd (QString::null, mimelist, m_view, "Export Layer", true);
                fd.setCaption(i18n("Export Layer") + "(" + ch->name() + ")");
                fd.setMimeFilter(listMimeFilter);
                fd.setOperationMode(KFileDialog::Saving);
                fd.setURL(KUrl(ch->name()));
                if (!fd.exec()) return;

                KUrl url = fd.selectedURL();
                QString mimefilter = fd.currentMimeFilter();

                if (url.isEmpty())
                    return;

                KisPaintLayerSP l = new KisPaintLayer( image, ch->name(), OPACITY_OPAQUE, *deviceIt);
                QRect r = l->exactBounds();

                KisDoc d;
                d.prepareForImport();

                KisImageSP dst = new KisImage(d.undoAdapter(), r.width(), r.height(), (*deviceIt)->colorSpace(), l->name());
                d.setCurrentImage( dst );
                dst->addLayer(l->clone(), dst->rootLayer(), 0);

                d.setOutputMimeType(mimefilter.latin1());
                d.exp0rt(url);

            }

            ++deviceIt;
        }

        if (undo && undo->undo()) {
            undo->endMacro();
        }

        m_view->canvasSubject()->document()->setModified(true);
    }
}




#include "kis_channel_separator.moc"
