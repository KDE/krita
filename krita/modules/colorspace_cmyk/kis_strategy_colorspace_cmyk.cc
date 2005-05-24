/*
 *  Copyright (c) 2003 Boudewijn Rempt (boud@valdyas.org)
 *
 *  This program is free software; you can CYANistribute it and/or modify
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
#include <limits.h>
#include <stdlib.h>
#include <config.h>
#include LCMS_HEADER

#include <qimage.h>

#include <kdebug.h>
#include <klocale.h>

#include "kis_config.h"
#include "kis_image.h"
#include "kis_strategy_colorspace_cmyk.h"
#include "kis_colorspace_registry.h"
#include "kis_iterators_pixel.h"

#include "kis_resource.h"
#include "kis_resourceserver.h"
#include "kis_resource_mediator.h"
#include "kis_factory.h"
#include "kis_profile.h"

namespace cmyk {
	const Q_INT32 MAX_CHANNEL_CMYK = 4;
}

KisStrategyColorSpaceCMYK::KisStrategyColorSpaceCMYK() :
	KisStrategyColorSpace(KisID("CMYK", i18n("CMYK")), TYPE_CMYK_8, icSigCmykData)
{
	m_channels.push_back(new KisChannelInfo(i18n("cyan"), 0, COLOR));
	m_channels.push_back(new KisChannelInfo(i18n("magenta"), 1, COLOR));
	m_channels.push_back(new KisChannelInfo(i18n("yellow"), 2, COLOR));
	m_channels.push_back(new KisChannelInfo(i18n("black"), 3, COLOR));
	m_channels.push_back(new KisChannelInfo(i18n("alpha"), 4, ALPHA));

	if (profileCount() == 0) {
		kdDebug() << "No profiles loaded!\n";
		return;
	}

	m_defaultProfile = getProfileByName("Adobe CMYK"); // XXX: Do not i18n -- this is from a data file
	if (m_defaultProfile == 0) {
		kdDebug() << "No Adobe CMYK!\n";
		if (profileCount() != 0) {
			m_defaultProfile = profiles()[0];
		}
	}

	if (m_defaultProfile == 0) {
		kdDebug() << "No default CMYK profile; CMYK will not work!\n";
		return;
	}

	// Create the default transforms from and to a QColor. Use the
	// display profile if there's one, otherwise a generic sRGB profile
	// XXX: For now, always use the generic sRGB profile.

	cmsHPROFILE hsRGB = cmsCreate_sRGBProfile();
	cmsHPROFILE hsCMYK = m_defaultProfile -> profile();

	m_defaultFromRGB = cmsCreateTransform(hsRGB, TYPE_BGR_8,
					      hsCMYK, TYPE_CMYK_8,
					      INTENT_PERCEPTUAL, 0);

	m_defaultToRGB =  cmsCreateTransform(hsCMYK, TYPE_CMYK_8,
					     hsRGB, TYPE_BGR_8,
					     INTENT_PERCEPTUAL, 0);

	// Default pixel buffer for QColor conversion
	m_qcolordata = new QUANTUM[3];
	Q_CHECK_PTR(m_qcolordata);

}

KisStrategyColorSpaceCMYK::~KisStrategyColorSpaceCMYK()
{
	// XXX: These deletes cause a crash, but since the color strategy is a singleton
	//      that's only deleted at application close, it's no big deal.
	delete [] m_qcolordata;
	//cmsDeleteTransform(m_defaultToRGB);
	//cmsDeleteTransform(m_defaultFromRGB);
}

void KisStrategyColorSpaceCMYK::nativeColor(const QColor& color, QUANTUM *dst, KisProfileSP profile)
{
	m_qcolordata[2] = color.red();
	m_qcolordata[1] = color.green();
	m_qcolordata[0] = color.blue();

	cmsDoTransform(m_defaultFromRGB, m_qcolordata, dst, 1);
	dst[3] = OPACITY_OPAQUE;
}

void KisStrategyColorSpaceCMYK::nativeColor(const QColor& color, QUANTUM opacity, QUANTUM *dst, KisProfileSP profile)
{
	m_qcolordata[2] = color.red();
	m_qcolordata[1] = color.green();
	m_qcolordata[0] = color.blue();

	cmsDoTransform(m_defaultFromRGB, m_qcolordata, dst, 1);
	dst[3] = opacity;
}


void KisStrategyColorSpaceCMYK::toQColor(const QUANTUM *src, QColor *c, KisProfileSP profile)
{
	cmsDoTransform(m_defaultToRGB, const_cast <QUANTUM *>(src), m_qcolordata, 1);
	c -> setRgb(m_qcolordata[2], m_qcolordata[1], m_qcolordata[0]);
}

void KisStrategyColorSpaceCMYK::toQColor(const QUANTUM *src, QColor *c, QUANTUM *opacity, KisProfileSP profile)
{
	cmsDoTransform(m_defaultToRGB, const_cast <QUANTUM *>(src), m_qcolordata, 1);
	c -> setRgb(m_qcolordata[2], m_qcolordata[1], m_qcolordata[0]);

 	*opacity = src[3];
}

vKisChannelInfoSP KisStrategyColorSpaceCMYK::channels() const
{
	return m_channels;
}

bool KisStrategyColorSpaceCMYK::alpha() const
{
	return false;
}

Q_INT32 KisStrategyColorSpaceCMYK::nChannels() const
{
	return cmyk::MAX_CHANNEL_CMYK;
}

Q_INT32 KisStrategyColorSpaceCMYK::nColorChannels() const
{
	return cmyk::MAX_CHANNEL_CMYK;
}

Q_INT32 KisStrategyColorSpaceCMYK::pixelSize() const
{
	return cmyk::MAX_CHANNEL_CMYK;
}

QImage KisStrategyColorSpaceCMYK::convertToQImage(const QUANTUM *data, Q_INT32 width, Q_INT32 height,
						  KisProfileSP srcProfile, KisProfileSP dstProfile,
						  Q_INT32 renderingIntent)

{
//  	kdDebug() << "convertToQImage: (" << width << ", " << height << ")"
//  		  << " srcProfile: " << srcProfile << ", " << "dstProfile: " << dstProfile << "\n";

	QImage img = QImage(width, height, 32, 0, QImage::LittleEndian);
	memset(img.bits(), 255, width * height * sizeof(Q_UINT32));
	KisStrategyColorSpaceSP dstCS = KisColorSpaceRegistry::instance() -> get("RGBA");


 	if (srcProfile == 0 || dstProfile == 0 || dstCS == 0) {
//   		kdDebug() << "Going to use default transform\n";
 		cmsDoTransform(m_defaultToRGB,
 			       const_cast<QUANTUM *> (data),
 			       img.bits(),
 			       width * height);
 	}
 	else {
//   		kdDebug() << "Going to transform with profiles\n";
 		// Do a nice calibrated conversion
 		convertPixelsTo(const_cast<QUANTUM *>(data), srcProfile,
 				img.bits(), dstCS, dstProfile,
 				width * height, renderingIntent);
 	}

	return img;
}


void KisStrategyColorSpaceCMYK::bitBlt(Q_INT32 pixelSize,
				       QUANTUM *dst,
				       Q_INT32 dstRowStride,
				       const QUANTUM *src,
				       Q_INT32 srcRowStride,
				       QUANTUM opacity,
				       Q_INT32 rows,
				       Q_INT32 cols,
				       const KisCompositeOp& op)
{
	Q_INT32 linesize = pixelSize * sizeof(QUANTUM) * cols;
	QUANTUM *d;
	const QUANTUM *s;

	if (rows <= 0 || cols <= 0)
		return;

	switch (op.op()) {
	case COMPOSITE_COPY:
		d = dst;
		s = src;

		while (rows-- > 0) {
			memcpy(d, s, linesize);
			d += dstRowStride;
			s += srcRowStride;
		}
		break;
	case COMPOSITE_CLEAR:
		d = dst;
		s = src;
		while (rows-- > 0) {
			memset(d, 0, linesize);
			d += dstRowStride;
		}
		break;
	case COMPOSITE_OVER:
	default:
		compositeOver(dst, dstRowStride, src, srcRowStride, rows, cols, opacity);
	}
}

// XXX: Cut & Paste from colorspace_rgb

inline int INT_MULT(int a, int b)
{
	int c = a * b + 0x80;
	return ((c >> 8) + c) >> 8;
}

inline int INT_BLEND(int a, int b, int alpha)
{
	return INT_MULT(a - b, alpha) + b;
}


void KisStrategyColorSpaceCMYK::compositeOver(QUANTUM *dstRowStart, Q_INT32 dstRowStride, 
					     const QUANTUM *srcRowStart, Q_INT32 srcRowStride, 
					     Q_INT32 rows, Q_INT32 numColumns, 
					     QUANTUM opacity)
{
	while (rows > 0) {

		const QUANTUM *src = srcRowStart;
		QUANTUM *dst = dstRowStart;
		Q_INT32 columns = numColumns;

		while (columns > 0) {

			if (opacity == OPACITY_OPAQUE) {
				memcpy(dst, src, cmyk::MAX_CHANNEL_CMYK * sizeof(QUANTUM));

			} else {
				
				dst[PIXEL_CYAN] = INT_BLEND(src[PIXEL_CYAN], dst[PIXEL_CYAN], opacity);
				dst[PIXEL_MAGENTA] = INT_BLEND(src[PIXEL_MAGENTA], dst[PIXEL_MAGENTA], opacity);
				dst[PIXEL_YELLOW] = INT_BLEND(src[PIXEL_YELLOW], dst[PIXEL_YELLOW], opacity);
				dst[PIXEL_BLACK] = INT_BLEND(src[PIXEL_BLACK], dst[PIXEL_BLACK], opacity);

			}

			columns--;
			src += cmyk::MAX_CHANNEL_CMYK;
			dst += cmyk::MAX_CHANNEL_CMYK;
		}

		rows--;
		srcRowStart += srcRowStride;
		dstRowStart += dstRowStride;
	}
}


KisCompositeOpList KisStrategyColorSpaceCMYK::userVisiblecompositeOps() const
{
	KisCompositeOpList list;

	list.append(KisCompositeOp(COMPOSITE_OVER));

	return list;
}

