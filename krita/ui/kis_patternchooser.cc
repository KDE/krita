/*
 *  kis_patternchooser.cc - part of Krayon
 *
 *  A chooser for KisPatterns. Makes use of the KoIconChooser class and maintains
 *  all available brushes for KIS.
 *
 *  Copyright (c) 1999 Carsten Pfeiffer <pfeiffer@kde.org>
 *  Copyright (c) 2000 Matthias Elter   <elter@kde.org>
 *  Copyright (c) 2002 Patrick Julein <freak@codepimps.org>
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
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <qhbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qptrlist.h>

#include <kinstance.h>
#include <klocale.h>
#include <kstandarddirs.h>

#include <koIconChooser.h>

#include "kis_factory.h"
#include "kis_resourceserver.h"
#include "integerwidget.h"

#include "kis_patternchooser.h"

// X11 headers
#undef Below
#undef Above

KisPatternChooser::KisPatternChooser( QWidget *parent, const char *name )
 : QWidget( parent, name )
{
    lbSpacing = new QLabel( i18n("Spacing:"), this );
    slSpacing = new IntegerWidget( 1, 100, this, "int widget" );

    slSpacing->setTickmarks( QSlider::Below );
    slSpacing->setTickInterval( 10 );
    QObject::connect( slSpacing, SIGNAL( valueChanged(int) ),
	    this, SLOT( slotSetPatternSpacing(int) ));

    // only serves as beautifier for the iconchooser
    frame = new QHBox( this );
    frame->setFrameStyle( QFrame::Panel | QFrame::Sunken );
    chooser = new KoIconChooser( QSize(30,30), frame, "pattern chooser" );

    QPtrList<KisPattern> bList = KisFactory::rServer()->patterns();

    for (KisPattern *pattern = bList.first(); pattern != 0; pattern = bList.next())
    {
        if ( pattern->isValid() )
	        chooser->addItem(pattern );
    }

    QObject::connect( chooser, SIGNAL(selected( KoIconItem * ) ),
		    this, SLOT( slotItemSelected( KoIconItem * )));

    initGUI();

    KisPattern *pattern = currentPattern();

    if ( pattern )
        slSpacing->setValue(pattern->spacing());
}


KisPatternChooser::~KisPatternChooser()
{
  delete lbSpacing;
  delete slSpacing;
  // delete container;
  delete chooser;
  delete frame;
}


// set the active pattern in the chooser - does NOT emit selected() (should it?)
void KisPatternChooser::setCurrentPattern( KisPattern *pattern )
{
    chooser->setCurrentItem(  pattern );
    slSpacing->setValue( pattern->spacing() );
}


// return the active pattern
KisPattern * KisPatternChooser::currentPattern()
{
    return (KisPattern *) chooser->currentItem();
}


void KisPatternChooser::initGUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout( this, 2, -1, "main layout" );
    QHBoxLayout *spacingLayout = new QHBoxLayout( -1, "spacing layout" );

    mainLayout->addWidget( frame, 10 );
    mainLayout->addLayout( spacingLayout, 1 );

    spacingLayout->addWidget( lbSpacing, 0 );
    spacingLayout->addStretch();
    spacingLayout->addWidget( slSpacing, 1 );
}


// called when an item is selected in the chooser
// set the slider to the correct position
void KisPatternChooser::slotItemSelected( KoIconItem *item )
{
    KisPattern *pattern = (KisPattern *) item;
    slSpacing->setValue( pattern->spacing());
    emit selected( pattern );
}


// sliderposition (spacing) changed, apply that to the current pattern
void KisPatternChooser::slotSetPatternSpacing( int spacing )
{
    KisPattern *pattern = (KisPattern *) currentPattern();
        if ( pattern ) pattern->setSpacing( spacing );
}

#include "kis_patternchooser.moc"





