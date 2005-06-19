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

#include "kis_bc_filter_widget.h"
#include "wdg_brightness_contrast.h"

#include <klocale.h>
#include <qlayout.h>

#include "kis_filter.h"

KisBCFilterWidget::KisBCFilterWidget( KisFilter* nfilter, QWidget * parent, const char * name, const char * caption) : QWidget( parent, name )
{
	this->setCaption(caption);

	QGridLayout *widgetLayout = new QGridLayout(this, 1, 1);
	widgetLayout -> setColStretch ( 1, 1 );

	m_w = new WdgBrightnessContrast(this);
	widgetLayout -> addWidget( m_w, 0 , 0);
}

#include "kis_bc_filter_widget.moc"
