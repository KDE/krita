/*
 * This file is part of Krita
 *
 * Copyright (c) 2005 Boudewijn Rempt <boud@valdyas.org>
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

#ifndef BUMPMAP_H
#define BUMPMAP_H

#include <qwidget.h>

#include <kparts/plugin.h>

#include <kis_filter.h>
#include "kis_view.h"
#include <kis_paint_device.h>

#include "wdgbumpmap.h"

namespace krita {

	enum enumBumpmapType {
		LINEAR = 0,
		SPHERICAL = 1,
		SINUSOIDAL = 2,
	};

};

using namespace krita;



class KritaBumpmap : public KParts::Plugin
{
public:
	KritaBumpmap(QObject *parent, const char *name, const QStringList &);
	virtual ~KritaBumpmap();

private:

	KisView * m_view;

};




/**
 * First stab at a bumpmapping filter. For now, this is taken both
 * from the Gimp source and the code from emboss.c:
 *			 ANSI C code from the article
 *			 "Fast Embossing Effects on Raster Image Data"
 *			 by John Schlag, jfs@kerner.com
 * 			in "Graphics Gems IV", Academic Press, 1994
 */
class KisFilterBumpmap : public KisFilter
{
public:
	KisFilterBumpmap(KisView * view);
public:
	virtual void process(KisPaintDeviceSP src, KisPaintDeviceSP dst, KisFilterConfiguration*, const QRect&);
	static inline KisID id() { return KisID("bumpmap", i18n("Bumpmap")); };
	virtual bool supportsPainting() { return false; }
	virtual bool supportsIncrementalPainting() { return false; }

        virtual QWidget* createConfigurationWidget(QWidget* parent);
        virtual KisFilterConfiguration* configuration(QWidget*);

};



class KisBumpmapConfiguration : public KisFilterConfiguration {

public:

	KisBumpmapConfiguration();

	KisPaintDeviceSP bumpmap;
	double  azimuth;
	double  elevation;
	double  depth;
	Q_INT32 xofs;
	Q_INT32 yofs;
	Q_INT32 waterlevel;
	Q_INT32 ambient;
	bool    compensate;
	bool    invert;
	bool	tile;
	enumBumpmapType type;
};


class KisBumpmapConfigWidget : public QWidget {

	Q_OBJECT

public:
	KisBumpmapConfigWidget(KisFilter * filter, QWidget * parent, const char * name = 0, WFlags f = 0 );
	virtual ~KisBumpmapConfigWidget() {};

	KisBumpmapConfiguration * config();

	WdgBumpmap * m_page;

private:

	KisFilter * m_filter;

};

#endif
