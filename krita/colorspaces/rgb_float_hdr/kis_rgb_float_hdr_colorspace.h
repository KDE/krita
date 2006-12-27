/*
 *  Copyright (c) 2006 Cyrille Berger <cberger@cberger.net>
 *  Copyright (c) 2005 Adrian Page <adrian@pagenet.plus.com>
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
#ifndef KIS_RGB_FLOAT_HDR_COLORSPACE_H_
#define KIS_RGB_FLOAT_HDR_COLORSPACE_H_

#include "klocale.h"
#include <KoIncompleteColorSpace.h>
#include <KoFallBack.h>

template <class _CSTraits>
class KisRgbFloatHDRColorSpace : public KoIncompleteColorSpace<_CSTraits, KoRGB16Fallback>
{
    public:
        KisRgbFloatHDRColorSpace(const QString &id, const QString &name, KoColorSpaceRegistry * parent, qint32 type)
          : KoIncompleteColorSpace<_CSTraits, KoRGB16Fallback>(id, name, parent, type, icSigRgbData)
        {
        
        }
};

#endif // KIS_STRATEGY_COLORSPACE_RGB_H_
