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

#ifndef _KIS_FILTER_CONFIGURATION_WIDGET_H_
#define _KIS_FILTER_CONFIGURATION_WIDGET_H_

class KisFilter;
class KisFilterConfiguration;

#include <qwidget.h>

class KisFilterConfigurationWidget : public QWidget {
	Q_OBJECT
public:
	KisFilterConfigurationWidget ( KisFilter* nfilter, QWidget * parent = 0, const char * name = 0, WFlags f = 0 );
public:
	inline KisFilter* filter() { return m_filter; };
private:
	KisFilter* m_filter;
};

#endif
