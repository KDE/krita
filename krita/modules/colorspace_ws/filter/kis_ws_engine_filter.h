/*
 *  Copyright (c) 2005 Boudewijn Rempt (boud@valdyas.org)
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
#ifndef _KIS_WS_ENGINE_FILTER_H_
#define _KIS_WS_ENGINE_FILTER_H_

#include <kdebug.h>

#include "kis_view.h"
#include "kis_filter.h"


class KisWSEngineFilterConfiguration : public KisFilterConfiguration
{

public:

	KisWSEngineFilterConfiguration() { m_pixels = 10000; }

	KisWSEngineFilterConfiguration(Q_UINT32 pixels = 0) { m_pixels = pixels; }

	Q_UINT32 pixels() { return m_pixels; }

private:

	Q_UINT32 m_pixels; // The number of pixels the filter should
			   // move. 0 means keep running indefinitely



};

class KisWSEngineFilter : public KisFilter
{

public:

	KisWSEngineFilter(KisView * view);

	virtual void process(KisPaintDeviceSP src, KisPaintDeviceSP dst, KisFilterConfiguration* cfg, const QRect& rc);
	static inline QString name() { return "Wet & Sticky Engine"; };
	virtual bool supportsPainting() { return false; }


public:
	virtual KisFilterConfigurationWidget* createConfigurationWidget(QWidget* parent);
	virtual KisFilterConfiguration* configuration(KisFilterConfigurationWidget*);


private:

private:

	KisWSEngineFilterConfiguration * m_cfg;
	KisPaintDeviceSP m_src;
	KisPaintDeviceSP m_dst;
	QRect m_rect;
	KisTransaction * m_ktc;

};

#endif // _KIS_WS_ENGINE_FILTER_H_
