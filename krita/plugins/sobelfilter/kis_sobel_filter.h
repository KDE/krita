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

#ifndef _KIS_SOBEL_FILTER_H_
#define _KIS_SOBEL_FILTER_H_

#include "kis_filter.h"
#include "kis_view.h"
#include <kdebug.h>

class KisSobelFilterConfiguration : public KisFilterConfiguration
{
	public:
                KisSobelFilterConfiguration(Q_UINT32 pixelWidth, Q_UINT32 pixelHeight) : m_pixelWidth(pixelWidth), m_pixelHeight(pixelHeight) {};
	public:
                inline Q_UINT32 pixelWidth() { return m_pixelWidth; };
                inline Q_UINT32 pixelHeight() {return m_pixelHeight; };
        private:
                Q_UINT32 m_pixelWidth;
                Q_UINT32 m_pixelHeight;
};

class KisSobelFilter : public KisFilter
{
public:
	KisSobelFilter();
public:
	virtual void process(KisPaintDeviceSP,KisPaintDeviceSP, KisFilterConfiguration* , const QRect&);
	static inline KisID id() { return KisID("sobel", i18n("Sobel")); };
	virtual bool supportsPainting() { return true; }
	virtual bool supportsPreview() { return true; }
	virtual std::list<KisFilterConfiguration*> listOfExamplesConfiguration(KisPaintDeviceSP )
	{ std::list<KisFilterConfiguration*> list; list.insert(list.begin(), new KisSobelFilterConfiguration(10,10)); return list; }
public:
	virtual KisFilterConfigWidget * createConfigurationWidget(QWidget* parent, KisPaintDeviceSP dev);
	virtual KisFilterConfiguration* configuration(QWidget*, KisPaintDeviceSP dev);
private:
	void prepareRow (KisPaintDeviceSP src, Q_UINT8* data, Q_UINT32 x, Q_UINT32 y, Q_UINT32 w, Q_UINT32 h);
	void sobel(KisPaintDeviceSP src, KisPaintDeviceSP dst, bool doHorizontal, bool doVertical, bool keepSign);
};

#endif
