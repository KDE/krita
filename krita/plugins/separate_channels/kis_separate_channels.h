/*
 * This file is part of the KDE project
 *
 * Copyright (c) Michael Thaler <michael.thaler@physik.tu-muenchen.de>
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

#ifndef _KIS_SEPARATE_CHANNELS_H_
#define _KIS_SEPARATE_CHANNELS_H_

#include "kis_filter.h"
#include "kis_view.h"
#include <kdebug.h>

class KisSeparateChannelsConfiguration : public KisFilterConfiguration
{
	public:
                KisSeparateChannelsConfiguration(Q_UINT32 pixelWidth, Q_UINT32 pixelHeight) : m_pixelWidth(pixelWidth), m_pixelHeight(pixelHeight) {};
	public:
                inline Q_UINT32 pixelWidth() { return m_pixelWidth; };
                inline Q_UINT32 pixelHeight() {return m_pixelHeight; };
        private:
                Q_UINT32 m_pixelWidth;
                Q_UINT32 m_pixelHeight;
};

class KisSeparateChannels : public KisFilter
{
public:
	KisSeparateChannels();
public:
	virtual void process(KisPaintDeviceSP,KisPaintDeviceSP, KisFilterConfiguration* , const QRect&);
	static inline KisID id() { return KisID("separatechannels", i18n("Separate channels")); };
	virtual bool supportsPainting() { return false; }
	virtual bool supportsPreview() { return false; }
	virtual std::list<KisFilterConfiguration*> listOfExamplesConfiguration(KisPaintDeviceSP )
	{ std::list<KisFilterConfiguration*> list; list.insert(list.begin(), new KisSeparateChannelsConfiguration(10,10)); return list; }
public:
	virtual KisFilterConfigWidget * createConfigurationWidget(QWidget* parent, KisPaintDeviceSP dev);
	virtual KisFilterConfiguration* configuration(QWidget*, KisPaintDeviceSP dev);
private:
	void pixelize(KisPaintDeviceSP src, KisPaintDeviceSP dst, int x, int y, int w, int h, int pixelWidth, int pixelHeight);
	void separateChannels(KisPaintDeviceSP src, KisPaintDeviceSP dst, const QRect& rec);
};

#endif
