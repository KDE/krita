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
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#ifndef _KIS_PERCHANNEL_FILTER_H_
#define _KIS_PERCHANNEL_FILTER_H_

#include "kis_filter.h"
#include <kdebug.h>

class KisChannelInfo;

class KisPerChannelFilterConfiguration
	: public KisFilterConfiguration
{
public:
	KisPerChannelFilterConfiguration(Q_INT32 nbchannels, vKisChannelInfoSP ci);

public:

	// This function return the value at index i
	inline Q_INT32& valueFor(Q_INT32 i) { return m_values[i]; };

	// This function return the channel number at index i
	inline Q_INT32 channel(Q_INT32 i) { return m_channels[i]; };

private:
	Q_INT32* m_values;
	Q_INT32* m_channels;
};

/** This class is generic for filters that affect channel separately
	*/
class KisPerChannelFilter
	: public KisFilter
{
public:
	KisPerChannelFilter(KisView * view, const QString& name, Q_INT32 min, Q_INT32 max, Q_INT32 initvalue );
public:
	virtual KisFilterConfigurationWidget* createConfigurationWidget(QWidget* parent);
	virtual KisFilterConfiguration* configuration(KisFilterConfigurationWidget*);
private:
	Q_INT32 m_min;
	Q_INT32 m_max;
	Q_INT32 m_initvalue;
	Q_INT32 m_nbchannels;
};

#endif
