/* This file is part of the KDE project
   Copyright (c) 2004 Cyrille Berger <cberger@cberger.net>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include <math.h>

#include <stdlib.h>

#include <qslider.h>
#include <qpoint.h>

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
#include <kis_layer.h>
#include <kis_global.h>
#include <kis_tile_command.h>
#include <kis_types.h>
#include <kis_view.h>
#include <kistile.h>
#include <kistilemgr.h>
#include <kis_iterators_pixel.h>

// #include <kmessagebox.h>

#include "formbcdialog.h"
#include "formrgbsliders.h"
#include "formcmybsliders.h"

#include "colorsfilters.moc"

#define min(x,y) ((x)<(y)?(x):(y))

typedef KGenericFactory<ColorsFilters> ColorsFiltersFactory;
K_EXPORT_COMPONENT_FACTORY( colorsfilters, ColorsFiltersFactory( "krita" ) )

namespace {
	inline QUANTUM processColor( QUANTUM d, int s)
	{
		if( d < -s  ) return 0;
		else if( d > QUANTUM_MAX - s) return QUANTUM_MAX;
		else return d + s;
	}
}

ColorsFilters::ColorsFilters(QObject *parent, const char *name, const QStringList &)
		: KParts::Plugin(parent, name)
{
	setInstance(ColorsFiltersFactory::instance());

	kdDebug() << "ColorsFilters plugin. Class: " 
		  << className() 
		  << ", Parent: " 
		  << parent -> className()
		  << "\n";


	(void) new KAction(i18n("&Brightness / Contrast..."), 0, 0, this, SLOT(slotBrightnessContrastActivated()), actionCollection(), "brightnesscontrast");
	(void) new KAction(i18n("&Gamma Correction..."), 0, 0, this, SLOT(slotGammaActivated()), actionCollection(), "gammacorrection");
	(void) new KAction(i18n("&Color Adjustment..."), 0, 0, this, SLOT(slotColorActivated()), actionCollection(), "coloradjustment");
	(void) new KAction(i18n("&Desaturate"), 0, 0, this, SLOT(slotDesaturate()), actionCollection(), "desaturate");
	if ( !parent->inherits("KisView") )
	{
		m_view = 0;
	} else {
		m_view = (KisView*) parent;
	}
}

ColorsFilters::~ColorsFilters()
{
}

void ColorsFilters::slotColorActivated()
{
	KisLayerSP lay = m_view->currentImg()->activeLayer();
	KisTileCommand* ktc = new KisTileCommand(i18n("Color adjustment"), (KisPaintDeviceSP)lay ); // Create a command
	KisTileMgrSP ktm = lay->data();
	KisTileSP tile;
	if( lay->colorStrategy()->name() == "RGBA")
	{
		FormRGBSliders* frsd = new FormRGBSliders( m_view, "Color adjustment", TRUE);

		frsd->setCaption(i18n("Color adjustment"));

		frsd->setMinValue(-255);
		frsd->setMaxValue(255);
		frsd->setPrecision(1);
		frsd->setInitValue(0);
		if( frsd->exec() == QDialog::Rejected )
			return;
		int red = (int)frsd->getRedValue();
		int green = (int)frsd->getGreenValue();
		int blue = (int)frsd->getBlueValue();
		for(unsigned int i = 0; i < ktm->nrows() * ktm->ncols(); i++)
		{
			if( (tile = ktm->tile( i , TILEMODE_NONE)) )
			{
				ktc->addTile( i , tile);
			}
			if (!(tile = ktm->tile( i, TILEMODE_RW)))
				continue;
			QUANTUM *data = tile->data(0, 0);
			// we compute the color inversion
			// kdDebug() << lay->alpha() << (int)(lay->alpha()) << (1+lay->alpha()) << endl;
			for( int j = 0; j < tile->size(); j += 1 ) //lay->alpha() )
			{
				data[j] = processColor( data[j], blue );
				j ++;
				data[j] = processColor( data[j], green );
				j ++;
				data[j] = processColor( data[j], red );
				j ++;
			}
		}
		m_view->currentImg()->undoAdapter()->addCommand( ktc );
		m_view->currentImg()->notify();
	} else if( lay->colorStrategy()->name() == "CMYKA") {
		FormCMYBSliders* frsd = new FormCMYBSliders( m_view, "Color adjustment", TRUE);
		frsd->setCaption(i18n("Color adjustment"));
		frsd->setMinValue(-255);
		frsd->setMaxValue(255);
		frsd->setPrecision(1);
		frsd->setInitValue(0);
		if( frsd->exec() == QDialog::Rejected )
			return;
		QUANTUM cyan = (QUANTUM)frsd->getCyanValue();
		QUANTUM magenta = (QUANTUM)frsd->getMagentaValue();
		QUANTUM yellow = (QUANTUM)frsd->getYellowValue();
		QUANTUM black = (QUANTUM)frsd->getBlackValue();
		for(unsigned int i = 0; i < ktm->nrows() * ktm->ncols(); i++)
		{
			if( (tile = ktm->tile( i , TILEMODE_NONE)) )
			{
				ktc->addTile( i , tile);
			}
			if (!(tile = ktm->tile( i, TILEMODE_RW)))
				continue;
			QUANTUM *data = tile->data(0, 0);
			// we compute the color inversion
			// kdDebug() << lay->alpha() << (int)(lay->alpha()) << (1+lay->alpha()) << endl;
			for( int j = 0; j < tile->size(); j += 1 ) //lay->alpha() )
			{
				data[j] = processColor( data[j], cyan );
				j ++;
				data[j] = processColor( data[j], magenta );
				j ++;
				data[j] = processColor( data[j], yellow );
				j ++;
				data[j] = processColor( data[j], black );
				j ++;
			}
		}
		m_view->currentImg()->undoAdapter()->addCommand( ktc );
		m_view->currentImg()->notify();
	}
}

void ColorsFilters::slotDesaturate()
{
	KisLayerSP lay = m_view->currentImg()->activeLayer();
	KisTileCommand* ktc = new KisTileCommand("Desaturate", (KisPaintDeviceSP)lay ); // Create a command
	if( lay->colorStrategy()->name() == "RGBA") {
		KisIteratorLinePixel lineIt = lay->iteratorPixelSelectionBegin(ktc);
		KisIteratorLinePixel lastLine = lay->iteratorPixelSelectionEnd(ktc);
		while( lineIt <= lastLine )
		{
			KisIteratorPixel pixelIt = *lineIt;
			KisIteratorPixel lastPixel = lineIt.end();
			while( pixelIt <= lastPixel )
			{
				KisPixelRepresentation data = pixelIt;
				/* I thought of using the HSV model, but GIMP seems to use
				   HSL for desaturating. Better use the gimp model for now 
				   (HSV produces a lighter image than HSL) */
				Q_INT32 lightness = ( QMAX(QMAX(data[0], data[1]), data[2])
				                    +QMIN(QMIN(data[0], data[1]), data[2])) / 2; 
				data[0] = lightness;
				data[1] = lightness;
				data[2] = lightness;
				++pixelIt;
			}
			++lineIt;
		}
		m_view->currentImg()->undoAdapter()->addCommand( ktc );
		m_view->currentImg()->notify();
	} else {
		kdDebug() << "Colorsfilters: desaturate not yet supported for this type" << endl;
	}
}

void ColorsFilters::slotGammaActivated()
{
	KisLayerSP lay = m_view->currentImg()->activeLayer();
	KisTileCommand* ktc = new KisTileCommand(i18n("Gamma Correction"), (KisPaintDeviceSP)lay ); // Create a command
	KisTileMgrSP ktm = lay->data();
	KisTileSP tile;
	if( lay->colorStrategy()->name() == "RGBA")
	{
		FormRGBSliders* frsd = new FormRGBSliders( m_view, "Gamma Correction", TRUE);
		frsd->setCaption(i18n("Gamma Correction"));
		frsd->setMinValue(1);
		frsd->setMaxValue(600);
		frsd->setPrecision(100);
		frsd->setInitValue(1);
		if( frsd->exec() == QDialog::Rejected )
			return;
		float red = frsd->getRedValue();
		float green = frsd->getGreenValue();
		float blue = frsd->getBlueValue();
//		kdDebug() << "RGB" << red << " " << green << " " << blue << endl;
		for(unsigned int i = 0; i < ktm->nrows() * ktm->ncols(); i++)
		{
			if( (tile = ktm->tile( i , TILEMODE_NONE)) )
			{
				ktc->addTile( i , tile);
			}
			if (!(tile = ktm->tile( i, TILEMODE_RW)))
				continue;
			QUANTUM *data = tile->data(0, 0);
			// we compute the color inversion
			// kdDebug() << lay->alpha() << (int)(lay->alpha()) << (1+lay->alpha()) << endl;
			for( int j = 0; j < tile->size(); j += 1 ) //lay->alpha() )
			{
				data[j] = (QUANTUM)( QUANTUM_MAX * pow( ((float)data[j])/QUANTUM_MAX, 1.0 / blue ) );
				j ++;
				data[j] = (QUANTUM)( QUANTUM_MAX * pow( ((float)data[j])/QUANTUM_MAX, 1.0 / green ) );
				j ++;
				data[j] = (QUANTUM)( QUANTUM_MAX * pow( ((float)data[j])/QUANTUM_MAX, 1.0 / red ) );
				j ++;
			}
		}
		m_view->currentImg()->undoAdapter()->addCommand( ktc );
		m_view->currentImg()->notify();
	} else if( lay->colorStrategy()->name() == "CMYA") {
		FormCMYBSliders* frsd = new FormCMYBSliders( m_view, "Gamma Correction", TRUE);
		frsd->setCaption(i18n("Gamma Correction"));
		frsd->setMinValue(1);
		frsd->setMaxValue(600);
		frsd->setPrecision(100);
		frsd->setInitValue(1);
		if( frsd->exec() == QDialog::Rejected )
			return;
		float cyan = frsd->getCyanValue();
		float magenta = frsd->getMagentaValue();
		float yellow = frsd->getYellowValue();
		float black = frsd->getBlackValue();
		for(unsigned int i = 0; i < ktm->nrows() * ktm->ncols(); i++)
		{
			if( (tile = ktm->tile( i , TILEMODE_NONE)) )
			{
				ktc->addTile( i , tile);
			}
			if (!(tile = ktm->tile( i, TILEMODE_RW)))
				continue;
			QUANTUM *data = tile->data(0, 0);
			// we compute the color inversion
			// kdDebug() << lay->alpha() << (int)(lay->alpha()) << (1+lay->alpha()) << endl;
			for( int j = 0; j < tile->size(); j += 1 ) //lay->alpha() )
			{
				data[j] = (QUANTUM)( QUANTUM_MAX * pow( ((float)data[j])/QUANTUM_MAX, 1.0 / cyan ) );
				j ++;
				data[j] = (QUANTUM)( QUANTUM_MAX * pow( ((float)data[j])/QUANTUM_MAX, 1.0 / magenta ) );
				j ++;
				data[j] = (QUANTUM)( QUANTUM_MAX * pow( ((float)data[j])/QUANTUM_MAX, 1.0 / yellow ) );
				j ++;
				data[j] = (QUANTUM)( QUANTUM_MAX * pow( ((float)data[j])/QUANTUM_MAX, 1.0 / black ) );
				j ++;
			}
		}
		m_view->currentImg()->undoAdapter()->addCommand( ktc );
		m_view->currentImg()->notify();
	}
}
void ColorsFilters::slotBrightnessContrastActivated()
{
	//Get the new values
	FormBCDialog* fbcd = new FormBCDialog( m_view, "Brightness / Contrast", TRUE);
	fbcd->setCaption(i18n("Brightness / Contrast"));
	if( fbcd->exec() ==  QDialog::Rejected )
		return;
	int bright = fbcd->sliderBrightness->value();
	int contrast = 100+fbcd->sliderContrast->value();
	KisLayerSP lay = m_view->currentImg()->activeLayer();
	KisTileCommand* ktc = new KisTileCommand(i18n("Brightness / Contrast"), (KisPaintDeviceSP)lay ); // Create a command
	int nbchannel = lay->depth() - 1; // get the number of channel whithout alpha
	KisTileMgrSP ktm = lay->data();
	KisTileSP tile;
	for(unsigned int i = 0; i < ktm->nrows() * ktm->ncols(); i++)
	{
		if( (tile = ktm->tile( i , TILEMODE_NONE)) )
		{
			ktc->addTile( i , tile);
		}
		if (!(tile = ktm->tile( i, TILEMODE_RW)))
			continue;
		QUANTUM *data = tile->data(0, 0);
//		kdDebug() << lay->alpha() << (int)(lay->alpha()) << (1+lay->alpha()) << endl;
		for( int j = 0; j < tile->size(); j += 1 ) //lay->alpha() )
		{
			int end = j + nbchannel;
			for( ; j < end ; j ++ )
			{
			// change the brightness
				QUANTUM d = data[j];
				if( d < -bright  ) d = 0;
				else if( d > QUANTUM_MAX - bright ) d = QUANTUM_MAX;
				else d += bright;
			// change the contrast
				int nd = d * contrast / 100;
				if( nd > QUANTUM_MAX ) d = QUANTUM_MAX;
				else d = nd;
				data[j] = d ;
			}
		}
	}
	m_view->currentImg()->undoAdapter()->addCommand( ktc );
	m_view->currentImg()->notify();
}
