/*
 * This file is part of the KDE project
 *
 * Copyright (c) 2005 Boudewijn <boud@valdyas.org>
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
#include <qcheckbox.h>
#include <qbuttongroup.h>
#include <qstring.h>
#include <qpushbutton.h>

#include <knuminput.h>
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
#include <kis_colorspace_registry.h>

#include "wdgbumpmap.h"
#include "bumpmap.h"

#define MOD(x, y) \
  ((x) < 0 ? ((y) - 1 - ((y) - 1 - (x)) % (y)) : (x) % (y))

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

namespace {
	void convertRow(KisPaintDeviceSP orig, Q_UINT8 * row, Q_INT32 x, Q_INT32 y, Q_INT32 w,  Q_UINT8 * lut, Q_INT32 waterlevel)
	{
		KisStrategyColorSpaceSP cs = KisColorSpaceRegistry::instance()->get("GRAYA");
		KisStrategyColorSpaceSP csOrig = orig->colorStrategy();


		bool convert = (csOrig != cs);
		if (!convert) {
			// Already GRAYA
			orig->readBytes(row, x, y, w, 1);
		}
		
		Q_UINT32 i = 0;
		KisHLineIterator origIt = orig->createHLineIterator(x, y, w, false);
		while (!origIt.isDone()) {
			if (convert) {
				
				QColor c;
				QUANTUM opacity;
					csOrig->toQColor(origIt.rawData(), &c, &opacity);
				row[i] = (c.red() * 0.30
					  + c.green() * 0.59
					  + c.blue() * 0.11) + 0.5;
				row[i + 1] = opacity;
				
			}

			row[i] = lut[waterlevel + ((row[i] -  waterlevel) * row[i + 1]) / 255];
			
			row += 2;
			++origIt;
		}
	}
	
}

void KisFilterBumpmap::process(KisPaintDeviceSP src, KisPaintDeviceSP dst, KisFilterConfiguration* cfg, const QRect& rect)
{
	if (!src) return;
	if (!dst) return;
	if (!cfg) return;
	if (!rect.isValid()) return;
	if (rect.isNull()) return;
	if (rect.isEmpty()) return;
	
	KisBumpmapConfiguration * config = (KisBumpmapConfiguration*)cfg;

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
	lx = (Q_INT32)(cos(azimuth) * cos(elevation) * 255.0);
	ly = (Q_INT32)(sin(azimuth) * cos(elevation) * 255.0);
	
	lz = (Q_INT32)(sin(elevation) * 255.0);

	/* Calculate constant Z component of surface normal */
	nz = (Q_INT32)((6 * 255) / config->depth);
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
			lut[i] = 255 - lut[i];
	}


	// Crate a grayscale layer from the bumpmap layer.
	QRect bmRect;
	KisPaintDeviceSP bumpmap;

	if (!config->bumpmap.isNull()) {
		KisPaintDeviceSP bumplayer = m_view->currentImg()->findLayer(config->bumpmap).data();
		bmRect = bumplayer->exactBounds();
		bumpmap = bumplayer;
	}
	else {
	 	bmRect = rect;
		bumpmap = src;
	}

	setProgressTotalSteps(rect.height());

	// ------------------- Map the bumps
	Q_INT32 yofs1, yofs2, yofs3;

	// ------------------- Initialize offsets
	if (config->tiled) {
		yofs2 = MOD (config->yofs + rect.y(), bmRect.height());
		yofs1 = MOD (yofs2 - 1, bmRect.height());
		yofs3 = MOD (yofs2 + 1,  bmRect.height());
	}
	else {
	      yofs2 = CLAMP (config->yofs + rect.y(), 0, bmRect.height() - 1);
	      yofs1 = yofs2;
	      yofs3 = CLAMP (yofs2 + 1, 0, bmRect.height() - 1);

	}

	// ---------------------- Load initial three bumpmap scanlines
	
	KisStrategyColorSpaceSP srcCs = src->colorStrategy();
	vKisChannelInfoSP channels = srcCs->channels();

	// One byte per pixel, converted from the bumpmap layer.
	Q_UINT8 * bm_row1 = new Q_UINT8[bmRect.width() * 2];
	Q_UINT8 * bm_row2 = new Q_UINT8[bmRect.width() * 2];
	Q_UINT8 * bm_row3 = new Q_UINT8[bmRect.width() * 2];
	Q_UINT8 * tmp_row;

	convertRow(bumpmap, bm_row1, bmRect.x(), yofs1, bmRect.width(), lut, config->waterlevel);
	convertRow(bumpmap, bm_row2, bmRect.x(), yofs2, bmRect.width(), lut, config->waterlevel);
	convertRow(bumpmap, bm_row3, bmRect.x(), yofs3, bmRect.width(), lut, config->waterlevel);
	
	bool row_in_bumpmap;

	Q_INT32 xofs1, xofs2, xofs3, shade, ndotl, nx, ny;
	
	for (int y = rect.y(); y < rect.height(); y++) {

		row_in_bumpmap = (y >= - config->yofs && y < - config->yofs + bmRect.height());

		// Bumpmap
		
		KisHLineIteratorPixel dstIt = dst->createHLineIterator(rect.x(), y, rect.width(), true);
		KisHLineIteratorPixel srcIt = src->createHLineIterator(rect.x(), y, rect.width(), false);

		Q_INT32 tmp = config->xofs + rect.x();
		xofs2 = MOD (tmp, bmRect.width());

		Q_INT32 x = 0;
		while (!srcIt.isDone()) {

			if (srcIt.selectedNess() > MAX_SELECTED / 2) {
				// Calculate surface normal from bumpmap
				if (config->tiled || row_in_bumpmap &&
					x >= - tmp&& x < - tmp + bmRect.width()) {
	
					if (config->tiled) {
						xofs1 = MOD (xofs2 - 1, bmRect.width());
						xofs3 = MOD (xofs2 + 1, bmRect.width());
					}
					else {
						xofs1 = CLAMP (xofs2 - 1, 0, bmRect.width() - 1);
						xofs3 = CLAMP (xofs2 + 1, 0, bmRect.width() - 1);
					}
					#if 0
					kdDebug() << "x: " << x
						<< ", x offset 1: " << xofs1
						<< ", x offset 2: " << xofs2
						<< ", x offset 3: " << xofs3 << "\n";
					#endif
					// We use 8-bit GRAYA, we need only the first byte of every
					// pixel
					nx = (bm_row1[xofs1] + bm_row2[xofs1] + bm_row3[xofs1] -
						bm_row1[xofs3] - bm_row2[xofs3] - bm_row3[xofs3]);
					ny = (bm_row3[xofs1] + bm_row3[xofs2] + bm_row3[xofs3] -
						bm_row1[xofs1] - bm_row1[xofs2] - bm_row1[xofs3]);
	
					
				}
				else {
					nx = 0;
					ny = 0;
				}
	
				// Shade
	
				if ((nx == 0) && (ny == 0)) {
					shade = background;
				}
				else {
					ndotl = (nx * lx) + (ny * ly) + nzlz;
	
					if (ndotl < 0) {
						shade = (Q_INT32)(compensation * config->ambient);
					}
					else {
						shade = (Q_INT32)(ndotl / sqrt(nx * nx + ny * ny + nz2));
						shade = (Q_INT32)(shade + QMAX(0, (255 * compensation - shade)) * config->ambient / 255);
					}
				}
	
				// Paint
				srcCs->darken(srcIt.rawData(), dstIt.rawData(), shade, config->compensate, compensation, 1);
			}
		      if (++xofs2 == bmRect.width())
				xofs2 = 0;
			++srcIt;
			++dstIt;
			++x;
		}


		// Next line
		if (config->tiled || row_in_bumpmap) {
			tmp_row = bm_row1;
			bm_row1 = bm_row2;
			bm_row2 = bm_row3;
			bm_row3 = tmp_row;
			
			if (++yofs2 == bmRect.height()) {
				yofs2 = 0;
			}
			if (config->tiled) {
				yofs3 = MOD(yofs2 + 1, bmRect.height());
			}
			else {
				yofs3 = CLAMP(yofs2 + 1, 0, bmRect.height() - 1);
			}

			convertRow(bumpmap, bm_row3, bmRect.x(), yofs3, bmRect.width(), lut, config->waterlevel);
		}

		incProgress();
	}
	delete [] bm_row1;
	delete [] bm_row2;
	delete [] bm_row3;
	setProgressDone();

}

QWidget* KisFilterBumpmap::createConfigurationWidget(QWidget* parent) 
{
	KisBumpmapConfigWidget * w = new KisBumpmapConfigWidget(this, m_view, parent);


	return w;
}

KisFilterConfiguration * KisFilterBumpmap::configuration(QWidget * w) 
{

	KisBumpmapConfigWidget * widget = dynamic_cast<KisBumpmapConfigWidget *>(w);
	if (widget == 0) {
		return new KisBumpmapConfiguration();
	}
	else {
		return widget->config();
	}

}


KisBumpmapConfiguration::KisBumpmapConfiguration()
{
	bumpmap = QString();
        azimuth = 135.0;
        elevation = 45.0;
        depth = 3;
        xofs = 0;
        yofs = 0;
        waterlevel = 0;
        ambient = 0;
        compensate = true;
        invert = false;
	tiled = true;
	type = krita::LINEAR;
}


KisBumpmapConfigWidget::KisBumpmapConfigWidget(KisFilter * filter, KisView * view, QWidget * parent, const char * name, WFlags f)
	: QWidget(parent, name, f),
	  m_filter(filter),
	  m_view(view)
{
	Q_ASSERT(m_filter);
	Q_ASSERT(m_view);
	
	m_page = new WdgBumpmap(this);
        QHBoxLayout * l = new QHBoxLayout(this);
        Q_CHECK_PTR(l);

        l -> add(m_page);
        m_filter -> setAutoUpdate(false);

	// Fill combobox with layers
	KisImageSP img = m_view->currentImg();
	if (img) {
		vKisLayerSP layers = img->layers();
		
		for (vKisLayerSP_cit it = layers.begin(); it != layers.end(); it++) {
			const KisLayerSP& layer = *it;
			m_page->cmbLayer->insertItem(layer->name());
		}
	}
	connect( m_page->bnRefresh, SIGNAL(clicked()), filter, SLOT(refreshPreview()));
}

KisBumpmapConfiguration * KisBumpmapConfigWidget::config()
{
	KisBumpmapConfiguration * cfg = new KisBumpmapConfiguration();
	cfg->bumpmap = m_page->cmbLayer->currentText();
	cfg->azimuth = m_page->dblAzimuth->value();
        cfg->elevation = m_page->dblElevation->value();
        cfg->depth = m_page->dblDepth->value();
        cfg->xofs = m_page->intXOffset->value();
        cfg->yofs = m_page->intYOffset->value();
        cfg->waterlevel = m_page->intWaterLevel->value();
        cfg->ambient = m_page->intAmbient->value();
        cfg->compensate = m_page->chkCompensate->isChecked();
        cfg->invert = m_page->chkInvert->isChecked();
	cfg->tiled = m_page->chkTiled->isChecked();
	cfg->type = (enumBumpmapType)m_page->grpType->selectedId();
	
	return cfg;
}

#include "bumpmap.moc"
