/*
 *  Copyright (c) 2004 Michael Thaler <michael.thaler@physik.tu-muenchen.de>
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

#include "kis_oilpaint_filter_configuration_widget.h"

#include <qlabel.h>
#include <qlayout.h>
#include <qtabwidget.h>
#include <knuminput.h>
#include "kis_filter.h"

KisOilPaintFilterConfigurationWidget::KisOilPaintFilterConfigurationWidget( KisFilter* nfilter, QWidget * parent, const char * name) : KisFilterConfigurationWidget( nfilter, parent, name )
{       
        QGridLayout *widgetLayout = new QGridLayout(this, 1, 1);

        //this creates a new configuration widget. The configuration widget should actually be
        //created with Qt Designer.
        
        kopfcw = new KisOilPaintFilterConfigurationBaseWidget((QWidget*)this);
	
        //This adds the configuration widget to the dialog
        
        widgetLayout -> addWidget(kopfcw, 0 , 0);

        //refresh the filter preview if the parameters for the filter are changed
        
        connect((QObject*)kopfcw->brushSizeSpinBox, SIGNAL(valueChanged(int)), filter(), SLOT(refreshPreview()));
        connect((QObject*)kopfcw->smoothSpinBox, SIGNAL(valueChanged(int)), filter(), SLOT(refreshPreview()));
}

//#include "kis_raindrops_filter_configuration_widget.moc"

#include "kis_oilpaint_filter_configuration_widget.moc"
