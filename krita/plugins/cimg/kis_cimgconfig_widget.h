/*
 * This file is part of Krita
 *
 * Copyright (c) 2005 Boudewijn Rempt <boud@valdyas.org>
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
 *
 * Ported from the CImg Gimp plugin by Victor Stinner and David Tschumperlé.
 */
#ifndef _KIS_CIMGCONFIG_WIDGET_
#define _KIS_CIMGCONFIG_WIDGET_

#include "kis_filter_configuration_widget.h"
#include "wdg_cimg.h"
#include "kis_cimg_filter.h"

class KisCImgconfigWidget : public QWidget {

	Q_OBJECT

public:

	KisCImgconfigWidget(KisFilter* nfilter, QWidget * parent = 0, const char * name = 0, WFlags f = 0 );
	virtual ~KisCImgconfigWidget() {};

public:

	KisCImgFilterConfiguration * config();

private:
	WdgCImg * m_page;

};

#endif // _KIS_CIMGCONFIG_WIDGET_
