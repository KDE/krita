/*
 *  kis_dlg_gradient.cc - part of Krayon
 *
 *  Copyright (c) 2001 John Califf <jcaliff@compuzone.net>
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

#include <qlabel.h>
#include <qlayout.h>
#include <qslider.h>
#include <qwidget.h>
#include <qbuttongroup.h>
#include <qpushbutton.h>
#include <qgroupbox.h>
#include <qspinbox.h>
#include <qcheckbox.h>

#include <klocale.h>

#include "integerwidget.h"
#include "blendchooser.h" 
#include "kis_gradient.h"
#include "kis_dlg_gradient.h"

// X11 headers
#undef Below
#undef Above

GradientTab::GradientTab( KisGradient *_gradient, 
    QWidget *_parent,  const char *_name ) 
    : QWidget( _parent, _name )
{
    QWidget *area = (QWidget *)this;
    QGridLayout *layout = new QGridLayout( area, 7, 2, 5 );

    // opacity - relative to layer or absolute ?
    // need checkbox for that
    opacity = new IntegerWidget( 0, 100, area );
    opacity->setValue( 100 );
    opacity->setTickmarks( QSlider::Below );
    opacity->setTickInterval( 10 );
    layout->addWidget( opacity, 0, 1 );

    QLabel *lblOpacity = new QLabel( opacity, 
        i18n( "Opacity" ), area );
    layout->addWidget( lblOpacity, 0, 0 );

    // offset percentage - foreground/background
    offset = new IntegerWidget( 0, 100, 
        area, "offset" );
    offset->setTickmarks( QSlider::Below );
    offset->setTickInterval( 10 );
    layout->addWidget( offset, 1, 1 );

    QLabel *lblOffset = new QLabel( offset, 
        i18n( "Offset" ), area );
    layout->addWidget( lblOffset, 1, 0 );

    // mode - how the gradient blends with the layer
    // equivalent to raster ops
    mode = new BlendChooser( area );
    layout->addWidget( mode, 2, 1 );

    QLabel *lblMode = new QLabel( mode, 
        i18n( "Mode" ), area );
    layout->addWidget( lblMode, 2, 0 );

    // blend - how the foregroud and background
    // gradient colors blend with each other
    blend = new QComboBox( false, area );
    blend->insertItem( i18n( "FG to BG (RGB)" ) );
    blend->insertItem( i18n( "FG to BG (HSV)" ) );
    blend->insertItem( i18n( "FG to Transparent" ) );
    blend->insertItem( i18n( "Custom (from editor)" ) );
    layout->addWidget( blend, 3, 1 );

    QLabel *lblBlend = new QLabel( blend, 
        i18n( "Blend" ), area );
    layout->addWidget( lblBlend, 3, 0 );

    // gradient types - from kimageeffects
        
    /* enum GradientType 
    { VerticalGradient, HorizontalGradient,
      DiagonalGradient, CrossDiagonalGradient,
      PyramidGradient, RectangleGradient,
      PipeCrossGradient, EllipticGradient }; */
    
    gradient = new QComboBox( false, area );
    gradient->insertItem( i18n( "Vertical" ) );
    gradient->insertItem( i18n( "Horizontal" ) );
    gradient->insertItem( i18n( "Diagonal" ) );
    gradient->insertItem( i18n( "CrossDiagonal" ) );
    gradient->insertItem( i18n( "Pyramid" ) );
    gradient->insertItem( i18n( "Rectangle" ) );
    gradient->insertItem( i18n( "PipeCross" ) );
    gradient->insertItem( i18n( "Elliptical" ) );
    //gradient->insertItem( i18n( "Shapeburst (angular)" ) );
    //gradient->insertItem( i18n( "Shapeburst (spherical)" ) );
    //gradient->insertItem( i18n( "Shapeburst (dimpled)" ) );
    
    // set to existing gradient effect type
    gradient->setCurrentItem(static_cast<int>(_gradient->gradientType()));
    
    layout->addWidget( gradient, 4, 1 );

    QLabel *lblGradient = new QLabel( gradient, 
        i18n( "Gradient" ), area );
    layout->addWidget( lblGradient, 4, 0 );

    // repeat - periodicy
    repeat = new QComboBox( false, area );
    repeat->insertItem( i18n( "None" ) );
    repeat->insertItem( i18n( "Sawtooth wave" ) );
    repeat->insertItem( i18n( "Triangular wave" ) );
    layout->addWidget( repeat, 5, 1 );

    QLabel *lblRepeat= new QLabel( repeat, 
        i18n( "Repeat" ), area );
    layout->addWidget( lblRepeat, 5, 0 );

    layout->setColStretch( 1, 1 );
    layout->setRowStretch( 6, 1 );
  
    opacity->setMinimumWidth(gradient->sizeHint().width());
}

int GradientTab::gradientOpacity()   { return opacity->value(); } 
int GradientTab::gradientOffset()    { return offset->value(); } 
int GradientTab::gradientMode()      { return mode->currentItem(); }
int GradientTab::gradientBlend()     { return blend->currentItem(); } 
int GradientTab::gradientType()      { return gradient->currentItem(); } 
int GradientTab::gradientRepeat()    { return repeat->currentItem(); }

GradientDialog::GradientDialog( KisGradient *_gradient,
    QWidget *_parent,  const char *_name, bool _modal) 
    : KDialog( _parent, _name, _modal)
{
    setCaption( i18n( "Gradients Options" ) );
    QVBoxLayout* layout = new QVBoxLayout( this, 4 );

    pGradientTab = new GradientTab(_gradient, static_cast<QWidget *>(this));
    layout->addWidget(pGradientTab);    
     
    QHBoxLayout* buttons = new QHBoxLayout( layout, 3 );
    buttons->addStretch( 3 );

    QPushButton *ok, *cancel, *save;
    ok = new QPushButton( i18n("&OK"), this );
    ok->setDefault( true );
    ok->setMinimumSize( ok->sizeHint() );
    connect( ok, SIGNAL(clicked()), SLOT(accept()) );
    buttons->addWidget( ok );

    cancel = new QPushButton( i18n("&Cancel"), this );
    cancel->setMinimumSize( cancel->sizeHint() );
    connect( cancel, SIGNAL(clicked()), SLOT(reject()) );
    buttons->addWidget( cancel );

    save = new QPushButton( i18n("&Save"), this );
    save->setMinimumSize( save->sizeHint() );
    connect( save, SIGNAL(clicked()), SLOT(reject()) );
    buttons->addWidget( save );
   
    resize(1,1);
}
 
#include "kis_dlg_gradient.moc"
