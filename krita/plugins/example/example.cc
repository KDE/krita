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
#include <stdlib.h>
#include <vector>

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
#include <kis_iterators.h>
#include <kis_layer.h>
#include <kis_global.h>
#include <kis_tile_command.h>
#include <kis_types.h>
#include <kis_view.h>
#include <kistile.h>
#include <kistilemgr.h>

// #include <kmessagebox.h>

#include "example.moc"

typedef KGenericFactory<KritaExample> KritaExampleFactory;
K_EXPORT_COMPONENT_FACTORY( kritaexample, KritaExampleFactory( "krita" ) )

	KritaExample::KritaExample(QObject *parent, const char *name, const QStringList &)
		: KParts::Plugin(parent, name)
{
       	setInstance(KritaExampleFactory::instance());

	(void) new KAction(i18n("&Invert..."), 0, 0, this, SLOT(slotActivated()), actionCollection(), "krita_example");
	(void) new KAction(i18n("&Invert (iterators)..."), 0, 0, this, SLOT(slotIteratorsActivated()), actionCollection(), "krita_example_iterators");

	if ( !parent->inherits("KisView") )
	{
		//kdError() << "KritaExempleFactory: KisView expected. Parent is " << parent->className() << endl;
		m_view = 0;
	} else {
		this->m_view = (KisView*) parent;
	}
}

KritaExample::~KritaExample()
{
}

void KritaExample::slotIteratorsActivated()
{
	KisDoc* kD = (KisDoc*) this->m_view->koDocument();
	if( kD->imageNum(0) == 0 )
		return;
	KisLayerSP lay = kD->imageNum(0)->activeLayer();
	KisTileCommand* ktc = new KisTileCommand("Invert", (KisPaintDeviceSP)lay ); // Create a command
	KisIteratorLineQuantum lineIt = lay->iteratorQuantumSelectionBegin(ktc);
	KisIteratorLineQuantum lastLine = lay->iteratorQuantumSelectionEnd(ktc);
	Q_INT32 depth = ::imgTypeDepth( lay->typeWithoutAlpha() );

	while( lineIt <= lastLine )
	{
		KisIteratorQuantum quantumIt = *lineIt;
		KisIteratorQuantum lastQuantum = lineIt.end();
		while( quantumIt <= lastQuantum )
		{
			for( int i = 0; i < depth; i++)
			{
				QUANTUM* data = quantumIt;
				*data = QUANTUM_MAX - (*data);
				++quantumIt;
			}
			++quantumIt;
		}
		++lineIt;
	}
	kD->imageNum(0)->undoAdapter()->addCommand( ktc );
	kD->imageNum(0)->notify();
}

void KritaExample::slotActivated()
{
	KisDoc* kD = (KisDoc*) this->m_view->koDocument();
	if( kD->imageNum(0) == 0 )
		return;
	KisLayerSP lay = kD->imageNum(0)->activeLayer();
	KisTileCommand* ktc = new KisTileCommand("Invert", (KisPaintDeviceSP)lay ); // Create a command
	int nbchannel = ::imgTypeDepth( lay->typeWithoutAlpha() ); // get the number of channel whithout alpha
	KisTileMgrSP ktm = lay->data();
	KisTileSP tile;

	for(int i = 0; i < ktm->nrows() * ktm->ncols(); i++)
	{
		if( (tile = ktm->tile( i , TILEMODE_NONE)) )
		{
			ktc->addTile( i , tile);
		}
		if (!(tile = ktm->tile( i, TILEMODE_RW)))
			continue;
		QUANTUM *data = tile->data(0, 0);
		// we compute the color inversion
//		kdDebug() << lay->alpha() << (int)(lay->alpha()) << (1+lay->alpha()) << endl;
		for( int j = 0; j < tile->size(); j += 1 ) //lay->alpha() )
		{
			int end = j + nbchannel;
			for( ; j < end ; j ++ )
			{
				data[j] = QUANTUM_MAX - data[j];
			}
		}
	}
	kD->imageNum(0)->undoAdapter()->addCommand( ktc );
	kD->imageNum(0)->notify();
}
