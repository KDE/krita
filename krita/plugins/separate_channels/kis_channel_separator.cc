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
#include <kis_abstract_colorspace.h>
#include <kis_colorspace_registry.h>
#include <kis_view.h>
#include <kis_paint_device.h>

#include "kis_channel_separator.h"

KisChannelSeparator::KisChannelSeparator(KisView * view)
	: m_view(view)
{
}

void KisChannelSeparator::separate(KisProgressDisplayInterface * progress)
{
	KisImageSP image = m_view->currentImg();
	if (!image) return;

	KisLayerSP src = image->activeLayer();
	if (!src) return;
		
	m_cancelRequested = false;
	if ( progress )
		progress -> setSubject(this, true, true);
	emit notifyProgressStage(this, i18n("Separating image..."), 0);

	KisUndoAdapter * undo = 0;
	KisTransaction * t = 0;
	if ((undo = image->undoAdapter())) {
		t = new KisTransaction(i18n("Separate Image"), src.data());
	}

	Q_UINT32 numberOfChannels = src -> nChannels();
	KisAbstractColorSpace * colorStrategy = src -> colorStrategy();
	vKisChannelInfoSP channels = colorStrategy -> channels();

	vKisLayerSP layers;

	vKisChannelInfoSP_cit begin = channels.begin();
	vKisChannelInfoSP_cit end = channels.end();


	QRect rect = image->bounds();

	int i = 0;
	for (vKisChannelInfoSP_cit it = begin; it != end; ++it)
	{

		KisChannelInfoSP ch = (*it);

		KisLayerSP dev = new KisLayer( KisColorSpaceRegistry::instance() -> get( "GRAYA" ), ch->name());
		layers.push_back(dev);

		KisRectIteratorPixel srcIt = src->createRectIterator(rect.x(), rect.y(), rect.width(), rect.height(), false);
		// XXX: Casper is going to make sure that these iterators align!
		KisRectIteratorPixel dstIt = dev->createRectIterator(rect.x(), rect.y(), rect.width(), rect.height(), true);

		while( ! srcIt.isDone() )
		{
			if(srcIt.isSelected())
			{
				dstIt.rawData()[0] = srcIt.oldRawData()[i];
				dstIt.rawData()[1] = OPACITY_OPAQUE;
			}
			++dstIt;
			++srcIt;
		}
		++i;

		emit notifyProgress(this, (i * 100) / numberOfChannels);
		if (m_cancelRequested) {
			break;
		}
	}

	vKisLayerSP_it it;

	if (!m_cancelRequested) {
		for ( it = layers.begin(); it != layers.end(); ++it ) {
			KisLayerSP layer = (*it);
			image->add( layer, -1);
		}
		if (undo) undo -> addCommand(t);

		m_view->getDocument()->setModified(true);
		m_view->layersUpdated();
	}

	emit notifyProgressDone(this);
	
}

#include "kis_channel_separator.moc"
