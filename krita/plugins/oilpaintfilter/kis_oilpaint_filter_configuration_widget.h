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

#ifndef _KIS_OILPAINT_FILTER_CONFIGURATION_WIDGET_
#define _KIS_OILPAINT_FILTER_CONFIGURATION_WIDGET_

#include <vector> 

#include "kis_filter_configuration_widget.h"
#include "kis_oilpaint_filter_configuration_widget.h"
#include "kis_oilpaint_filter_configuration_base_widget.h"

class KisOilPaintFilterConfigurationWidget : public KisFilterConfigurationWidget
{
	Q_OBJECT
        public:
		KisOilPaintFilterConfigurationWidget( KisFilter* nfilter, QWidget * parent, const char * name);
                inline KisOilPaintFilterConfigurationBaseWidget* baseWidget() { return kopfcw; };
        private:
                KisOilPaintFilterConfigurationBaseWidget *kopfcw;
};

#endif
