/*
 *  integerwidget.cc - part of KImageShop
 *
 *  A convenience widget for setting integer values
 *  Consists of a SpinBox and a slider
 *
 *  Copyright (c) 1999 Carsten Pfeiffer <pfeiffer@kde.org>
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
#include <qlayout.h>
#include <qspinbox.h>
#include <qlineedit.h>

#include "integerwidget.h"

IntegerWidget::IntegerWidget( int min, int max, QWidget* parent, const char* name )
  : QWidget( parent, name )
{
  layout = 0L;

  spinBox = new KSpinBox( min, max, 1, this, "spinbox" );
  connect( spinBox, SIGNAL( valueChanged( int ) ), this, SLOT( setSliderValue( int ) ) );

  slider = new QSlider( min, max, 1, min, QSlider::Horizontal, this, "sld" );
  connect( slider, SIGNAL( valueChanged( int ) ), spinBox, SLOT( setValue( int ) ) );

  initGUI();
}

IntegerWidget::~IntegerWidget()
{
  delete spinBox;
  delete slider;
  delete layout;
}

// the currently set value
int IntegerWidget::value()
{
  return spinBox->value();
}

// set the value, both widgets will be updated
void IntegerWidget::setValue( int value )
{
  slider->setValue( value );
}

// set the range, the widget should cover
void IntegerWidget::setRange( int min, int max )
{
  spinBox->setRange( min, max );
  slider->setRange( min, max );
}

// important - there must be an easy way the get straight focus to the
// editwidget, to quickly set the value via keyboard
void IntegerWidget::setEditFocus( bool mark )
{
  spinBox->setEditFocus( mark );
}

// set where to paint tickmarks for the slider
void IntegerWidget::setTickmarks( QSlider::TickSetting s )
{
  slider->setTickmarks( s );
}

void IntegerWidget::setTickInterval( int value )
{
  slider->setTickInterval( value );
}

int IntegerWidget::tickInterval() const
{
  return slider->tickInterval();
}

void IntegerWidget::initGUI()
{
  if ( layout )
    return;

  layout = new QHBoxLayout( this, 0, -1, "hbox layout" );
  layout->addWidget( spinBox );
  layout->addSpacing( 5 );
  layout->addWidget( slider );
}

// update the slider position when the spinbox value has changed and emit
// the new value
void IntegerWidget::setSliderValue( int value )
{
  slider->setValue( value );
  emit valueChanged( value );
}

//////////////////////
//// a quick wrapper around QSpinBox to be able to set focus to the LineEdit
//

KSpinBox::KSpinBox( QWidget *parent, const char *name )
  : QSpinBox( parent, name )
{
}

KSpinBox::KSpinBox( int minValue, int maxValue, int step, QWidget *parent,
		    const char *name )
  : QSpinBox( minValue, maxValue, step, parent, name )
{
}

void KSpinBox::setEditFocus( bool mark )
{
  QLineEdit *edit = editor();
  edit->setFocus();
  if ( mark )
    edit->selectAll();
}

#include "integerwidget.moc"
