/*
 *  Copyright (c) 2004 Cyrille Berger <cberger@cberger.net>
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
 
#include "kis_gradient_slider_widget.h"

#include <qpainter.h>

KisGradientSliderWidget::KisGradientSliderWidget(QWidget *parent, const char* name, WFlags f ) : QWidget( parent, name, f)
{
	setMinimumHeight(40);
	setMaximumHeight(40);
}

void KisGradientSliderWidget::paintEvent ( QPaintEvent* pe )
{
	QWidget::paintEvent( pe );
	QPainter painter( this );
	painter.setPen( Qt::black );
	painter.drawRect( 5, 5, width() - 5, 10 );
}
