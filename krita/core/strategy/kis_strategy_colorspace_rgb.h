/*
 *  Copyright (c) 2002 Patrick Julien  <freak@codepimps.org>
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
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */
#if !defined KIS_STRATEGY_COLORSPACE_RGB_H_
#define KIS_STRATEGY_COLORSPACE_RGB_H_

#include <qcolor.h>
#include <kpixmapio.h>
#include <koColor.h>
#include "kis_global.h"
#include "kis_strategy_colorspace.h"

class KisStrategyColorSpaceRGB : public KisStrategyColorSpace {
public:
	KisStrategyColorSpaceRGB();
	virtual ~KisStrategyColorSpaceRGB();

public:
	virtual void nativeColor(const KoColor& c, QUANTUM *dst);
	virtual void nativeColor(const KoColor& c, QUANTUM opacity, QUANTUM *dst);
	virtual void nativeColor(const QColor& c, QUANTUM *dst);
	virtual void nativeColor(const QColor& c, QUANTUM opacity, QUANTUM *dst);
	virtual void nativeColor(QRgb rgb, QUANTUM *dst);
	virtual void nativeColor(QRgb rgb, QUANTUM opacity, QUANTUM *dst);
	virtual void render(KisImageSP projection, QPainter& painter, Q_INT32 x, Q_INT32 y, Q_INT32 width, Q_INT32 height);

private:
	KPixmapIO m_pixio;
	QUANTUM *m_buf;
};

#endif // KIS_STRATEGY_COLORSPACE_H_

