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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef _KIS_BRIGHTNESS_CONTRAST_FILTER_H_
#define _KIS_BRIGHTNESS_CONTRAST_FILTER_H_

#include "kis_filter.h"
#include <kdebug.h>

class KisBrightnessContrastFilterConfiguration : public KisFilterConfiguration {
public:
	KisBrightnessContrastFilterConfiguration(Q_INT32 nbrightness, Q_INT32 ncontrast);
public:
	inline Q_INT32 brightness() { return m_brightness; };
	inline Q_INT32 contrast() { return m_contrast; };
	Q_UINT16 transferValue(Q_UINT8 i);
private:
	Q_INT32 m_brightness;
	Q_INT32 m_contrast;
};

/** 
 * This class is generic for filters that affect channel separately
 */
class KisBrightnessContrastFilter : public KisFilter
{

public:

	KisBrightnessContrastFilter();

public:

	virtual KisFilterConfigWidget * createConfigurationWidget(QWidget* parent, KisPaintDeviceSP dev);
	virtual KisFilterConfiguration* configuration(QWidget*, KisPaintDeviceSP dev);
	virtual void process(KisPaintDeviceSP, KisPaintDeviceSP, KisFilterConfiguration* , const QRect&);
	static inline KisID id() { return KisID("brightnesscontrast", i18n("Brightness / Contrast")); };
	virtual bool supportsPainting() { return true; }
	virtual bool supportsPreview() { return true; }
	virtual std::list<KisFilterConfiguration*> listOfExamplesConfiguration(KisPaintDeviceSP dev);

};

#endif
