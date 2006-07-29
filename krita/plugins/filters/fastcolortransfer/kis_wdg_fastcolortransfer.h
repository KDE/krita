/*
 * This file is part of Krita
 *
 * Copyright (c) 2006 Cyrille Berger <cberger@cberger.net>
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
 */

#ifndef KIS_WDG_COLORTRANSFER_H
#define KIS_WDG_COLORTRANSFER_H

// TODO: remove that
#define LCMS_HEADER <lcms.h>
// TODO: remove it !

#include <kis_filter_config_widget.h>

class KisFilter;
class WdgFastColorTransfer;

/**
	@author Cyrille Berger <cberger@cberger.net>
*/
class KisWdgFastColorTransfer : public KisFilterConfigWidget
{
    public:
        KisWdgFastColorTransfer(KisFilter* nfilter, QWidget * parent, const char * name);
        ~KisWdgFastColorTransfer();
        virtual void setConfiguration(KisFilterConfiguration*);
        inline WdgFastColorTransfer* widget() { return m_widget; }
    private:
        WdgFastColorTransfer* m_widget;
};

#endif
