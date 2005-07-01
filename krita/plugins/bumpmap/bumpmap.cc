/*
 * This file is part of the KDE project
 *
 * Copyright (c) 2004 Cyrille Berger <cberger@cberger.net>
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
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 *
 * This implementation completely and utterly based on the gimp's bumpmap.c,
 * copyright:
 * Copyright (C) 1997 Federico Mena Quintero <federico@nuclecu.unam.mx>
 * Copyright (C) 1997-2000 Jens Lautenbacher <jtl@gimp.org>
 * Copyright (C) 2000 Sven Neumann <sven@gimp.org>
 *
 */

#include <stdlib.h>
#include <vector>

#include <qpoint.h>
#include <qlayout.h>
#include <qcombobox.h>

#include <klocale.h>
#include <kiconloader.h>
#include <kinstance.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <ktempfile.h>
#include <kdebug.h>
#include <kgenericfactory.h>

#include <kis_doc.h>
#include <kis_image.h>
#include <kis_iterators_pixel.h>
#include <kis_layer.h>
#include <kis_filter_registry.h>
#include <kis_global.h>
#include <kis_types.h>
#include <kis_view.h>

// #include <kmessagebox.h>
#include "wdgbumpmap.h"
#include "bumpmap.h"

typedef KGenericFactory<KritaBumpmap> KritaBumpmapFactory;
K_EXPORT_COMPONENT_FACTORY( kritabumpmap, KritaBumpmapFactory( "krita" ) )

KritaBumpmap::KritaBumpmap(QObject *parent, const char *name, const QStringList &)
		: KParts::Plugin(parent, name)
{
	setInstance(KritaBumpmapFactory::instance());


	kdDebug(DBG_AREA_PLUGINS) << "Bumpmap plugin. Class: "
		  << className()
		  << ", Parent: "
		  << parent -> className()
		  << "\n";

	if ( !parent->inherits("KisView") )
	{
		return;
	} else {
		m_view = (KisView*) parent;
	}

	KisFilterSP kfi = createFilter<KisFilterBumpmap>(m_view);
	(void) new KAction(i18n("&Bumpmap"), 0, 0, kfi, SLOT(slotActivated()), actionCollection(), "krita_bumpmap");
}

KritaBumpmap::~KritaBumpmap()
{
}

KisFilterBumpmap::KisFilterBumpmap(KisView * view) : KisFilter(id(), view)
{
}



void KisFilterBumpmap::process(KisPaintDeviceSP src, KisPaintDeviceSP dst, KisFilterConfiguration* config, const QRect& rect)
{

#if 0
	Q_INT32 lx, ly;       /* X and Y components of light vector */
	Q_INT32 nz2, nzlz;    /* nz^2, nz*lz */
	Q_INT32 background;   /* Shade for vertical normals */
	double  compensation; /* Background compensation */
	Q_UINT8 lut[256];     /* Look-up table for modes */

	double azimuth;
	double elevation;
	Q_INT32 lz, nz;
	Q_INT32 i;
	double n;
	// ------------------ Prepare parameters

	/* Convert to radians */
	azimuth   = M_PI * config->azimuth / 180.0;
	elevation = M_PI * config->elevation / 180.0;

	/* Calculate the light vector */
	lx = cos(azimuth) * cos(elevation) * 255.0;
	ly = sin(azimuth) * cos(elevation) * 255.0;
	
	lz         = sin(elevation) * 255.0;

	/* Calculate constant Z component of surface normal */
	nz           = (6 * 255) / config->depth;
	nz2  = nz * nz;
	nzlz = nz * lz;

	/* Optimize for vertical normals */
	background = lz;

	/* Calculate darkness compensation factor */
	compensation = sin(elevation);

	/* Create look-up table for map type */
	for (i = 0; i < 256; i++)
	{
		switch (config->type)
		{
		case SPHERICAL:
			n = i / 255.0 - 1.0;
			lut[i] = (int) (255.0 * sqrt(1.0 - n * n) + 0.5);
			break;
			
		case SINUSOIDAL:
			n = i / 255.0;
			lut[i] = (int) (255.0 *
					(sin((-M_PI / 2.0) + M_PI * n) + 1.0) /
					2.0 + 0.5);
			break;
			
		case LINEAR:
		default:
			lut[i] = i;
		}
		
		if (config->invert)
			lut[i] = 255 - params->lut[i];
	}

	// ------------------- Convert the bumpmap layer to grayscale, paying attention to alpha, lookup table and waterlevel

	// 




	KisRectIteratorPixel dstIt = dst->createRectIterator(rect.x(), rect.y(), rect.width(), rect.height(), true );
	KisRectIteratorPixel srcIt = src->createRectIterator(rect.x(), rect.y(), rect.width(), rect.height(), false);
	Q_INT32 depth = src -> colorStrategy() -> nColorChannels();

	while( ! srcIt.isDone() )
	{
		if(srcIt.isSelected())
		{
			for( int i = 0; i < depth; i++)
			{
				dstIt.rawData()[i] = QUANTUM_MAX - srcIt.oldRawData()[i];
			}
		}
		++srcIt;
		++dstIt;
	}
#endif
}

QWidget* KisFilterBumpmap::createConfigurationWidget(QWidget* parent) 
{
	KisBumpmapConfigWidget * w = new KisBumpmapConfigWidget(this, parent);
	// Fill combobox with layers
	KisImageSP img = m_view->currentImg();
	if (img) {
		vKisLayerSP layers = img->layers();
		
		for (vKisLayerSP_cit it = layers.begin(); it != layers.end(); it++) {
			const KisLayerSP& layer = *it;
			w->m_page->cmbLayer->insertItem(layer->name());
		}
	}

	return w;
}

KisFilterConfiguration * KisFilterBumpmap::configuration(QWidget * w) 
{

	if (w == 0) {
		return new KisBumpmapConfiguration();
	}
	else {
		KisBumpmapConfigWidget * w = dynamic_cast<KisBumpmapConfigWidget *>(w);
		return w->config();
	}

}


KisBumpmapConfiguration::KisBumpmapConfiguration()
{
        KisPaintDeviceSP bumpmap = 0; // The layer we use as a bumpmap mask. If zero we'll use the layer we're working on.
        double  azimuth = 135.0;
        double  elevation = 45.0;
        double  depth = 3;
        Q_INT32 xofs = 0;
        Q_INT32 yofs = 0;
        Q_INT32 waterlevel = 0;
        Q_INT32 ambient = 0;
        bool    compensate = true;
        bool    invert = false;
	bool	tile = true;
	krita::enumBumpmapType type = krita::LINEAR;
}


KisBumpmapConfigWidget::KisBumpmapConfigWidget(KisFilter * filter, QWidget * parent, const char * name, WFlags f)
	: QWidget(parent, name, f),
	  m_filter(filter)
{
	m_page = new WdgBumpmap(this);

        QHBoxLayout * l = new QHBoxLayout(this);
        Q_CHECK_PTR(l);

        l -> add(m_page);
        m_filter -> setAutoUpdate(false);
}

KisBumpmapConfiguration * KisBumpmapConfigWidget::config()
{
	return new KisBumpmapConfiguration();
}

#include "bumpmap.moc"
