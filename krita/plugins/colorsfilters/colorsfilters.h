/*
 * This file is part of Krita
 *
 * Copyright (c) 2004 Cyrille Berger <cberger@cberger.net>
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
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA. */

#ifndef BRIGHTNESSCONTRAST_H
#define BRIGHTNESSCONTRAST_H

#include <kparts/plugin.h>
#include "kis_perchannel_filter.h"


class KisView;


class KisDesaturateFilter : public KisFilter {
public:
	KisDesaturateFilter(KisView * view);
public:
	virtual void process(KisPaintDeviceSP, KisPaintDeviceSP, KisFilterConfiguration* , const QRect&);
	static inline KisID id() { return KisID("desaturate", i18n("Desaturate")); };
	virtual bool supportsPainting() { return true; }
};

class KisGammaCorrectionFilter : public KisPerChannelFilter {
public:
	KisGammaCorrectionFilter(KisView * view);
public:
	virtual void process(KisPaintDeviceSP, KisPaintDeviceSP, KisFilterConfiguration* , const QRect&);
	static inline KisID id() { return KisID("gammadjustment", i18n("Gamma Adjustment")); };
        virtual bool supportsPainting() { return true; }

};

class KisColorAdjustmentFilter : public KisPerChannelFilter {
public:
	KisColorAdjustmentFilter(KisView * view);
public:
	virtual void process(KisPaintDeviceSP, KisPaintDeviceSP, KisFilterConfiguration* , const QRect&);
	static inline KisID id() { return KisID("coloradjustment", i18n("Color Adjustment")); };
        virtual bool supportsPainting() { return true; }

};

class KisAutoContrast : public KisFilter {
public:
	KisAutoContrast(KisView* view);
public:
	virtual void process(KisPaintDeviceSP, KisPaintDeviceSP, KisFilterConfiguration* , const QRect&);
	static inline KisID id() { return KisID("autocontrast", i18n("Auto Contrast")); };
        virtual bool supportsPainting() { return true; }

};

class ColorsFilters : public KParts::Plugin
{
public:
	ColorsFilters(QObject *parent, const char *name, const QStringList &);
	virtual ~ColorsFilters();

private:
	KisView* m_view;
	KisPainter *m_painter;
};

#endif
