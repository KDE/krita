/*
 *  Copyright (c) 2004, Boudewijn Rempt <boud@valdyas.org>
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


#include <qstring.h>
#include "kis_image_type.h"

namespace {
	const Q_INT8 DEFAULT_BIT_DEPTH = 8;
	const Q_INT8 DEFAULT_BYTE_DEPTH = 1;
	const Q_INT8 DEFAULT_CHANNELS = 4;
	const Q_INT32 DEFAULT_MAX_CHANNEL_VALUE = UCHAR_MAX;
	const bool DEFAULT_HAS_ALPHA = true;
}
KisImageType::KisImageType() : super()
{
}

KisImageType::KisImageType(const QString& label, const QString& description) 
	: super( label , description ),
	  m_bitsPerChannel( DEFAULT_BIT_DEPTH ),
	  m_bytesPerChannel( DEFAULT_BYTE_DEPTH ),
	  m_maxChannelValue( DEFAULT_MAX_CHANNEL_VALUE ),
	  m_numChannels( DEFAULT_CHANNELS ),
	  m_hasAlpha( DEFAULT_HAS_ALPHA )
{
}

KisImageType::~KisImageType()
{
}
