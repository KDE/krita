/*
 * This file is part of the KDE project
 *
 *  Copyright (c) Michael Thaler <michael.thaler@physik.tu-muenchen.de>
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

#ifndef _KIS_EMBOSS_FILTER_H_
#define _KIS_EMBOSS_FILTER_H_

#include "kis_filter.h"
#include "kis_view.h"
#include <kdebug.h>

class KisEmbossFilterConfiguration : public KisFilterConfiguration
{
public:
	KisEmbossFilterConfiguration(Q_UINT32 depth) : m_depth(depth) {};
public:
	inline Q_UINT32 depth() { return m_depth; };
private:
	Q_UINT32 m_depth;
};

class KisEmbossFilter : public KisFilter
{
public:
	KisEmbossFilter(KisView * view);
public:
	virtual void process(KisPaintDeviceSP,KisPaintDeviceSP, KisFilterConfiguration* , const QRect&, KisTileCommand* );
	static inline QString name() { return "Emboss"; };
	virtual bool supportsPainting() { return true; }
public:
	virtual KisFilterConfigurationWidget* createConfigurationWidget(QWidget* parent);
	virtual KisFilterConfiguration* configuration(KisFilterConfigurationWidget*);
private:
	void Emboss(QUANTUM* data, int Width, int Height, int d, KisProgressDisplayInterface* m_progress);
	inline int Lim_Max (int Now, int Up, int Max);
	inline uchar LimitValues (int ColorValue);
};

#endif
