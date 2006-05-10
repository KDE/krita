/* This file is part of the KDE project
   Copyright (C) 2005 Tim Beaulen <tbscope@gmail.org>

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
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#ifndef __GRADIENT_LOADER__
#define __GRADIENT_LOADER__

#include <q3ptrlist.h>
#include <QString>
#include <qstringlist.h>
#include <QFile>
#include <qdom.h>
#include <QColor>
#include <koffice_export.h>
struct KoColorStop
{
	double offset;
	double midpoint;
	double opacity;
	double color1;
	double color2;
	double color3;
	double color4;
	int colorType;
	int interpolation;
};

struct KoGradient
{
	double originX;
	double originY;
	double vectorX;
	double vectorY;
	double focalpointX;
	double focalpointY;
	int gradientType;
	int gradientRepeatMethod;

	Q3PtrList<KoColorStop> colorStops;
};	

class KOPAINTER_EXPORT KoGradientManager
{
public:

	enum KoGradientType
	{
		gradient_type_linear = 0,
		gradient_type_radial = 1,
		gradient_type_conic = 2
	};
	
	enum KoGradientInterpolation
	{
		interpolation_linear = 0,
		interpolation_curved = 1,
		interpolation_sine = 2,
		interpolation_sphere_increasing = 3,
		interpolation_sphere_decreasing = 4
	};
	
	enum KoGradientColorType
	{
		color_type_rgb = 0,
		color_type_hsv_ccw = 1,
		color_type_hsv_cw = 2,
		color_type_gray = 3,
		color_type_cmyk = 4
	};
	
	enum KoGradientRepeatMethod
	{
		repeat_method_none = 0,
		repeat_method_reflect = 1,
		repeat_method_repeat = 2
	};

	KoGradientManager();
	~KoGradientManager();

	KoGradient* loadGradient(const QString& filename);
	static QStringList filters() 
	{
		QStringList filterList;
		filterList << "*.kgr" << "*.svg" << "*.ggr"; 
		return filterList; 
	}

private:
	KoGradient* loadKarbonGradient(QFile* file);
	KoGradient* loadKritaGradient(QFile* file);
	KoGradient* loadSvgGradient(QFile* file);
	KoGradient* parseKarbonGradient(const QDomElement& element);
	KoGradient* parseSvgGradient(const QDomElement& element);
	void parseSvgColor(QColor &color, const QString &s);
};

#endif


