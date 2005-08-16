/*
 *  Copyright (c) 2005 Boudewijn Rempt <boud@valdyas.org>
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
#ifndef KIS_U8_BASE_COLORSPACE_H_
#define KIS_U8_BASE_COLORSPACE_H_

#include <qcolor.h>

#include <qcolor.h>

#include "kis_global.h"
#include "kis_abstract_colorspace.h"
#include "kis_pixel.h"

/**
 * This class is the base for all 8-bit/channel colorspaces
 */
class KisU8BaseColorSpace : public KisAbstractColorSpace {

public:

    KisU8BaseColorSpace(const KisID & id, DWORD cmType, icColorSpaceSignature colorSpaceSignature)
	: KisAbstractColorSpace(id, cmType, colorSpaceSignature)
    {
	m_alphaSize = sizeof(Q_UINT8);
    };

};

#endif // KIS_U8_BASE_COLORSPACE_H_
