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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * Ported from the CImg Gimp plugin by Victor Stinner and David Tschumperlé.
 */
#ifndef _KIS_CIMGCONFIG_WIDGET_
#define _KIS_CIMGCONFIG_WIDGET_

#include "wdg_cimg.h"
#include "kis_cimg_filter.h"
#include "kis_filter.h"
#include "kis_filter_config_widget.h"

class KisCImgconfigWidget : public KisFilterConfigWidget {

    Q_OBJECT

public:

    KisCImgconfigWidget(KisFilter* nfilter, QWidget * parent = 0, const char * name = 0, WFlags f = 0 );
    virtual ~KisCImgconfigWidget() {};

public:

    KisCImgFilterConfiguration * config();
    void setConfiguration(KisFilterConfiguration * config);

private:
    WdgCImg * m_page;

};

#endif // _KIS_CIMGCONFIG_WIDGET_
