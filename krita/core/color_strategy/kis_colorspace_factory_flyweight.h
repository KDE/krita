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

#if !defined KIS_COLORSPACE_FACTORY_FLYWEIGHT_H_
#define KIS_COLORSPACE_FACTORY_FLYWEIGHT_H_

#include <map>
#include "kis_colorspace_factory.h"

class KisColorSpaceFactoryFlyweight : public KisColorSpaceFactoryInterface {

	typedef KisColorSpaceFactoryInterface super;

	typedef std::map<enumImgType, KisStrategyColorSpaceSP> acFlyweights;
	typedef acFlyweights::iterator acFlyweights_it;
	typedef acFlyweights::const_iterator acFlyweights_cit;

public:
	KisColorSpaceFactoryFlyweight();
	virtual ~KisColorSpaceFactoryFlyweight();

public:
	virtual KisStrategyColorSpaceSP create(const KisPaintDeviceSP& device);
	virtual KisStrategyColorSpaceSP create(enumImgType imgType);
	virtual void add(enumImgType imgType, KisStrategyColorSpaceSP colorspace);

private:
	KisStrategyColorSpaceSP find(enumImgType imgType) const;
	
private:
	acFlyweights m_flyweights;
};

#endif // KIS_COLORSPACE_FACTORY_FLYWEIGHT_H_

