/*
 *  kis_brushchooser.cc - part of Krayon
 *
 *  A chooser for KisBrushes. Makes use of the IconChooser class and maintains
 *  all available brushes for KIS.
 *
 *  Copyright (c) 1999 Carsten Pfeiffer <pfeiffer@kde.org>
 *  Copyright (c) 2000 Matthias Elter   <elter@kde.org>
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

#include <kinstance.h>
#include <klocale.h>
#include <kstandarddirs.h>

#include "kis_factory.h"
#include "kis_resourceserver.h"
#include "iconchooser.h"
#include "integerwidget.h"
#include "kis_brushchooser.h"

// X11 headers
#undef Below
#undef Above

KisBrushChooser::KisBrushChooser( QWidget *parent, const char *name )
  : KFloatingDialog( parent, name )
{
    lbSpacing = new QLabel( i18n("Spacing:"), this );
    slSpacing = new IntegerWidget( 1, 100, this, "int widget" );

    slSpacing->setTickmarks( QSlider::Below );
    slSpacing->setTickInterval( 10 );
    QObject::connect( slSpacing, SIGNAL( valueChanged(int) ),
		    this, SLOT( slotSetBrushSpacing(int) ));

    // only serves as beautifier for the iconchooser
    frame = new QHBox( this );
    frame->setFrameStyle( QFrame::Panel | QFrame::Sunken );
    chooser = new IconChooser( frame, QSize(30,30), "icon chooser" );

    QPtrList<KisBrush> bList = KisFactory::rServer()->brushes();

    for (KisBrush *brush = bList.first(); brush != 0; brush = bList.next())
    {
        if ( brush->isValid() )
	        chooser->addItem( (IconItem *) brush );
    }

    QObject::connect( chooser, SIGNAL( selected( IconItem * ) ),
		    this, SLOT( slotItemSelected( IconItem * )));

    initGUI();

    KisBrush *brush = currentBrush();
    if ( brush ) slSpacing->setValue( brush->spacing() );
}

void KisBrushChooser::initGUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout( this, 2, -1, "main layout" );
    QHBoxLayout *spacingLayout = new QHBoxLayout( -1, "spacing layout" );

    mainLayout->addWidget( frame, 10 );
    mainLayout->addLayout( spacingLayout, 1 );

    spacingLayout->addWidget( lbSpacing, 0 );
    spacingLayout->addStretch();
    spacingLayout->addWidget( slSpacing, 1 );
}


KisBrushChooser::~KisBrushChooser()
{
    delete lbSpacing;
    delete slSpacing;

    // delete container;
    delete chooser;
    delete frame;
}


// set the active brush in the chooser - does NOT emit selected() (should it?)
void KisBrushChooser::setCurrentBrush( KisBrush *brush )
{
    chooser->setCurrentItem( (IconItem *) brush );
    slSpacing->setValue( brush->spacing() );
}


// return the active brush
KisBrush * KisBrushChooser::currentBrush()
{
    return (KisBrush *) chooser->currentItem();
}

// called when an item is selected in the chooser
// set the slider to the correct position
void KisBrushChooser::slotItemSelected( IconItem *item )
{
    KisBrush *brush = (KisBrush *) item;
    slSpacing->setValue( brush->spacing() );
    emit selected( brush );
}

// sliderposition (spacing) changed, apply that to the current brush
void KisBrushChooser::slotSetBrushSpacing( int spacing )
{
    KisBrush *brush = (KisBrush *) currentBrush();
    if ( brush )
        brush->setSpacing( spacing );
}

#include "kis_brushchooser.moc"
