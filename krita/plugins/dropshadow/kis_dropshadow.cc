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

#include <kis_doc.h>
#include <kis_image.h>
#include <kis_iterators_pixel.h>
#include <kis_layer.h>
#include <kis_transaction.h>
#include <kis_undo_adapter.h>
#include <kis_global.h>
#include <kis_types.h>
#include <kis_progress_subject.h>
#include <kis_progress_display_interface.h>
#include <kis_colorspace.h>
#include <kis_colorspace_factory_registry.h>
#include <kis_view.h>
#include <kis_paint_device_impl.h>
#include <kis_channelinfo.h>

#include "kis_dropshadow.h"

KisDropshadow::KisDropshadow(KisView * view)
    : m_view(view)
{
}

void KisDropshadow::dropshadow(KisProgressDisplayInterface * progress, Q_INT32 xoffset, Q_INT32 yoffset, Q_INT32 blurradius, Q_UINT8 opacity)
{
    KisImageSP image = m_view->getCanvasSubject()->currentImg();
    if (!image) return;

    KisLayerSP src = image->activeLayer();
    if (!src) return;

    m_cancelRequested = false;
    if ( progress )
        progress -> setSubject(this, true, true);
    emit notifyProgressStage(i18n("Add dropshadow..."), 0);

    KisUndoAdapter * undo = 0;
    KisTransaction * t = 0;
    if ((undo = image->undoAdapter())) {
        t = new KisTransaction(i18n("Add dropshadow"), src.data());
    }
    
    KisLayerSP shadowLayer = new KisLayer( KisColorSpaceFactoryRegistry::instance() -> getColorSpace(KisID("RGBA",""),"" ), "Shadow");
 
    QRect rect = src->exactBounds();
    Q_UINT32 pixelSize = src -> pixelSize();
    for (Q_INT32 row = 0; row < rect.height(); ++row) 
    {
        KisHLineIteratorPixel srcIt = src->createHLineIterator(rect.x(), rect.y() + row, rect.width(), false);
        KisHLineIteratorPixel dstIt = shadowLayer->createHLineIterator(rect.x(), rect.y() + row, rect.width(), true);
        while( ! srcIt.isDone() )
        {
            if (srcIt.isSelected())
            {
                for( int i = 0; i < pixelSize -1 ; i++)
                {
                    dstIt.rawData()[i] = 0;
                }
                dstIt.rawData()[pixelSize-1] = srcIt.oldRawData()[pixelSize-1];
            }
            ++srcIt;
            ++dstIt;
        }
        emit notifyProgress((row * 100) / rect.height() );
    }
    
    if (!m_cancelRequested) {
        shadowLayer -> move (xoffset,yoffset);
        shadowLayer -> setOpacity(opacity);
        image -> layerAdd( shadowLayer, -1 );
        //XXX: fix this, the shadow layer should be behind the active layer and not behind all layers
        image -> bottom( shadowLayer );
        image -> notifyLayersChanged();
        
        if (undo) undo -> addCommand(t);

        m_view->getCanvasSubject()->document()->setModified(true);

    }

    emit notifyProgressDone();

}

//#include "kis_dropshadow.moc"
