/*
 *  integerwidget.h - part of KImageShop
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
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#ifndef INTEGERWIDGET_H
#define INTEGERWIDGET_H

#include <qslider.h>
#include <qspinbox.h>
#include <qwidget.h>

class QHBoxLayout;
class KisSpinBox;

// really strange thing... you can't compile when you call 
// IntegerWidget::setTickSettings( QSlider::Below ) without this...

#ifdef Below
#undef Below
#endif
#ifdef Above
#undef Above
#endif

class IntegerWidget : public QWidget
{
	Q_OBJECT

public:

	IntegerWidget( int min, int max, QWidget* parent = 0, const char* name = 0 );
	~IntegerWidget();

	int value();
	void setRange( int min, int max );

	void setTickmarks( QSlider::TickSetting );
	void setTickInterval ( int );
	int tickInterval() const;

signals:

	void valueChanged( int );

public slots:

	void setValue( int value );
	void setEditFocus( bool mark = true );

protected slots:

	void setSliderValue( int );

protected:

	void initGUI();

	QHBoxLayout* layout;
	QSlider* slider;
	KisSpinBox* spinBox;
};

/**
 * A normal QSpinBox, but with the ability to set the focus to the lineedit
 * and mark text in the spinbox. Does not attempt to be generic, it only does
 * what I need now :o)
 */
class KisSpinBox : public QSpinBox
{
	Q_OBJECT

public:
	KisSpinBox( QWidget *parent=0, const char *name=0 );
	KisSpinBox ( int minValue, int maxValue, int step=1, QWidget*parent=0,
		     const char *name=0 );
	~KisSpinBox() {}


public slots:
	void setEditFocus( bool mark=true );

};

#endif // INTEGERWIDGET_H
