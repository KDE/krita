 /* This file is part of the KDE project
   Copyright (c) 2004 Cyrille Berger <cberger@cberger.net>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#ifndef BRIGHTNESSCONTRAST_H
#define BRIGHTNESSCONTRAST_H

#include <kparts/plugin.h>
#include "kis_perchannel_filter.h"


class KisView;


class KisDesaturateFilter : public KisFilter {
public:
	KisDesaturateFilter(KisView * view);
public:
	virtual void process(KisPaintDeviceSP, KisFilterConfiguration* , const QRect&, KisTileCommand* );
	static inline QString name() { return "Desaturate"; };
};

class KisGammaCorrectionFilter : public KisPerChannelFilter {
public:
	KisGammaCorrectionFilter(KisView * view);
public:
	virtual void process(KisPaintDeviceSP, KisFilterConfiguration* , const QRect&, KisTileCommand* );
	static inline QString name() { return "Gamma adjustment"; };
};

class KisColorAdjustmentFilter : public KisPerChannelFilter {
public:
	KisColorAdjustmentFilter(KisView * view);
public:
	virtual void process(KisPaintDeviceSP, KisFilterConfiguration* , const QRect&, KisTileCommand* );
	static inline QString name() { return "Color adjustment"; };
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
