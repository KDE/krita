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

#if !defined KIS_IMAGE_TYPE_H
#define KIS_IMAGE_TYPE_H

#include <qstring.h>

#include <ksharedptr.h>

#include "kis_global.h"
#include "kis_types.h"
#include "kis_strategy_colorspace.h"

#include "kis_abstract_capability.h"

/**
   KisImageType is a class that gather together the defining
   characteristics of an image type: bit-depth, colour strategy and
   perhaps others.

   Note that ImageType is not per se the characteristic of an _image_,
   but of a layer in an image. Layers can be of different types, and still
   part of the same image.

*/
class KisImageType : public KisAbstractCapability {

	typedef KisAbstractCapability super;
 public:

	KisImageType();
	KisImageType(QString label,
		     QString description);
	virtual ~KisImageType();

	virtual Q_UINT8 bitsPerChannel() const { return m_bitsPerChannel; };
	virtual void setBitsPerChannel( Q_UINT8 bits ) { m_bitsPerChannel = bits; };

	virtual Q_UINT8 bytesPerChannel() const { return m_bytesPerChannel; };
	virtual void setBytesPerChannel( Q_UINT8 bytes ) { m_bytesPerChannel = bytes; };

	virtual Q_UINT32 maxChannelValue() const { return m_maxChannelValue; };
	virtual void setMaxChannelValue( Q_UINT32 maxChannelValue ) { m_maxChannelValue = maxChannelValue; };

	virtual Q_UINT8 numChannels() const { return m_numChannels; };
	virtual void setNumChannels( Q_UINT8 numChannels ) { m_numChannels = numChannels; };

	virtual KisStrategyColorSpaceSP colorSpace() const { return m_colorSpace; };
	virtual void setColorSpace( KisStrategyColorSpaceSP colorSpace ) { m_colorSpace = colorSpace; };
	
	virtual bool hasAlpha() const { return m_hasAlpha; };
	virtual void setHasAlpha(bool hasAlpha) { m_hasAlpha = hasAlpha; };

	// Compatibility methods that correspond to kis_global.h things.
	virtual Q_UINT8 quantumSize() const { return m_bitsPerChannel; };
	virtual Q_UINT32 quantumMax() const { return m_maxChannelValue; };
	virtual Q_UINT32 opaque() const { return m_maxChannelValue; };
	virtual Q_INT32 depth() const { return m_bytesPerChannel; };

 private:
	Q_UINT8 m_bitsPerChannel;
	Q_UINT8 m_bytesPerChannel;
	Q_UINT32 m_maxChannelValue;
	Q_UINT8 m_numChannels;
	bool m_hasAlpha;
	KisStrategyColorSpaceSP m_colorSpace;

};

typedef KSharedPtr<KisImageType> KisImageTypeSP;

#endif // KIS_IMAGE_TYPE_H

