/*
 *  Copyright (c) 2003 Patrick Julien  <freak@codepimps.org>
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

#if !defined KIS_COLORSPACE_FACTORY_H_
#define KIS_COLORSPACE_FACTORY_H_

#include "kis_global.h"
#include "kis_types.h"
#include <map>
#include <qstring.h>
#include <kdemacros.h>

class KisColorSpaceFactory {
	typedef std::map<QString, KisStrategyColorSpaceSP> acFlyweights;
	typedef acFlyweights::iterator acFlyweights_it;
	typedef acFlyweights::const_iterator acFlyweights_cit;

public:
	virtual ~KisColorSpaceFactory();

public:
	KisStrategyColorSpaceSP create(const KisPaintDeviceSP& device) KDE_DEPRECATED;
	KisStrategyColorSpaceSP create(enumImgType imgType) KDE_DEPRECATED;
	void add(enumImgType imgType, KisStrategyColorSpaceSP colorspace) KDE_DEPRECATED ;
public:
	void add(KisStrategyColorSpaceSP colorspace);
	KisStrategyColorSpaceSP colorSpace(const QString& name) const;
public:
	static KisColorSpaceFactory *singleton();

private:
	KisColorSpaceFactory();
	KisColorSpaceFactory(const KisColorSpaceFactory&);
	KisColorSpaceFactory operator=(const KisColorSpaceFactory&);

private:
	static KisColorSpaceFactory *m_singleton;
	acFlyweights m_flyweights;
};

#endif // KIS_COLORSPACE_FACTORY_H_

