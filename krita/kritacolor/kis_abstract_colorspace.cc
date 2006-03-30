/*
 *  Copyright (c) 2002 Patrick Julien  <freak@codepimps.org>
 *  Copyright (c) 2004 Cyrille Berger <cberger@cberger.net>
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
#include <qimage.h>

#include <kdebug.h>
#include <kconfig.h>

#include "kis_abstract_colorspace.h"
#include "kis_global.h"
#include "kis_profile.h"
#include "kis_id.h"
#include "kis_integer_maths.h"
#include "kis_color_conversions.h"
#include "kis_colorspace_factory_registry.h"
#include "kis_channelinfo.h"

class KisColorAdjustmentImpl : public KisColorAdjustment
{
    public:
        
    KisColorAdjustmentImpl() : KisColorAdjustment()
        {
            csProfile = 0;
            transform = 0;
            profiles[0] = 0;
            profiles[1] = 0;
            profiles[2] = 0;
        };
    
    ~KisColorAdjustmentImpl() {

        if (transform)
            cmsDeleteTransform(transform);
        if (profiles[0] && profiles[0] != csProfile)
            cmsCloseProfile(profiles[0]);
        if(profiles[1] && profiles[1] != csProfile)
            cmsCloseProfile(profiles[1]);
        if(profiles[2] && profiles[2] != csProfile)
            cmsCloseProfile(profiles[2]);
    }

    cmsHPROFILE csProfile;
    cmsHPROFILE profiles[3];
    cmsHTRANSFORM transform;
};

KisAbstractColorSpace::KisAbstractColorSpace(const KisID& id,
                                             DWORD cmType,
                                             icColorSpaceSignature colorSpaceSignature,
                                             KisColorSpaceFactoryRegistry * parent,
                                             KisProfile *p)
    : m_parent( parent )
    , m_profile( p )
    , m_id( id )
    , m_cmType( cmType )
    , m_colorSpaceSignature( colorSpaceSignature )
{
    m_alphaPos = -1;
    m_alphaSize = -1;
    m_qcolordata = 0;
    m_lastUsedDstColorSpace = 0;
    m_lastUsedTransform = 0;
    m_lastRGBProfile = 0;
    m_lastToRGB = 0;
    m_lastFromRGB = 0;
    m_defaultFromRGB = 0;
    m_defaultToRGB = 0;
    m_defaultFromLab = 0;
    m_defaultToLab = 0;
}

void KisAbstractColorSpace::init()
{
    // Default pixel buffer for QColor conversion
    m_qcolordata = new quint8[3];
    Q_CHECK_PTR(m_qcolordata);

    if (m_profile == 0) return;

    // For conversions from default rgb
    m_lastFromRGB = cmsCreate_sRGBProfile();

    m_defaultFromRGB = cmsCreateTransform(m_lastFromRGB, TYPE_BGR_8,
                                          m_profile->profile(), m_cmType,
                                          INTENT_PERCEPTUAL, 0);

    m_defaultToRGB =  cmsCreateTransform(m_profile->profile(), m_cmType,
                                         m_lastFromRGB, TYPE_BGR_8,
                                         INTENT_PERCEPTUAL, 0);

    cmsHPROFILE hLab  = cmsCreateLabProfile(NULL);

    m_defaultFromLab = cmsCreateTransform(hLab, TYPE_Lab_16, m_profile->profile(), m_cmType,
                                          INTENT_PERCEPTUAL, 0);

    m_defaultToLab = cmsCreateTransform(m_profile->profile(), m_cmType, hLab, TYPE_Lab_16,
                                        INTENT_PERCEPTUAL, 0);
}

KisAbstractColorSpace::~KisAbstractColorSpace()
{
}



void KisAbstractColorSpace::fromQColor(const QColor& color, quint8 *dst, KisProfile * profile)
{
    m_qcolordata[2] = color.Qt::red();
    m_qcolordata[1] = color.Qt::green();
    m_qcolordata[0] = color.Qt::blue();


    if (profile == 0) {
	    // Default sRGB
	    if (!m_defaultFromRGB) return;

	    cmsDoTransform(m_defaultFromRGB, m_qcolordata, dst, 1);
    }
    else {
    	if (m_lastFromRGB == 0 || (m_lastFromRGB != 0 && m_lastRGBProfile != profile->profile())) {
	        m_lastFromRGB = cmsCreateTransform(profile->profile(), TYPE_BGR_8,
					       m_profile->profile(), m_cmType,
					       INTENT_PERCEPTUAL, 0);
	        m_lastRGBProfile = profile->profile();

	    }
    	cmsDoTransform(m_lastFromRGB, m_qcolordata, dst, 1);
    }

    setAlpha(dst, OPACITY_OPAQUE, 1);
}

void KisAbstractColorSpace::fromQColor(const QColor& color, quint8 opacity, quint8 *dst, KisProfile * profile)
{
    fromQColor(color, dst, profile);
    setAlpha(dst, opacity, 1);
}

void KisAbstractColorSpace::toQColor(const quint8 *src, QColor *c, KisProfile * profile)
{
    if (profile == 0) {
	// Default sRGB transform
        if (!m_defaultToRGB) return;
    	cmsDoTransform(m_defaultToRGB, const_cast <quint8 *>(src), m_qcolordata, 1);
    }
    else {
	    if (m_lastToRGB == 0 || (m_lastToRGB != 0 && m_lastRGBProfile != profile->profile())) {
	        m_lastToRGB = cmsCreateTransform(m_profile->profile(), m_cmType,
                                                 profile->profile(), TYPE_BGR_8,
                                                 INTENT_PERCEPTUAL, 0);
	        m_lastRGBProfile = profile->profile();
	    }
	    cmsDoTransform(m_lastToRGB, const_cast <quint8 *>(src), m_qcolordata, 1);
    }
    c->setRgb(m_qcolordata[2], m_qcolordata[1], m_qcolordata[0]);
}

void KisAbstractColorSpace::toQColor(const quint8 *src, QColor *c, quint8 *opacity, KisProfile * profile)
{
    toQColor(src, c, profile);
    *opacity = getAlpha(src);
}

void KisAbstractColorSpace::getSingleChannelPixel(quint8 *dstPixel, const quint8 *srcPixel, quint32 channelIndex)
{
    if (channelIndex < m_channels.count()) {

        fromQColor(Qt::black, OPACITY_TRANSPARENT, dstPixel);

        const KisChannelInfo *channelInfo = m_channels[channelIndex];
        memcpy(dstPixel + channelInfo->pos(), srcPixel + channelInfo->pos(), channelInfo->size());
    }
}

bool KisAbstractColorSpace::convertPixelsTo(const quint8 * src,
					    quint8 * dst,
					    KisColorSpace * dstColorSpace,
					    quint32 numPixels,
					    qint32 renderingIntent)
{
    if (dstColorSpace->colorSpaceType() == colorSpaceType()
        && dstColorSpace->getProfile() == getProfile())
    {
        if (src!= dst)
            memcpy (dst, src, numPixels * pixelSize());

        return true;
    }

    cmsHTRANSFORM tf = 0;

    qint32 srcPixelSize = pixelSize();
    qint32 dstPixelSize = dstColorSpace->pixelSize();

    if (m_lastUsedTransform != 0 && m_lastUsedDstColorSpace != 0) {
        if (dstColorSpace->colorSpaceType() == m_lastUsedDstColorSpace->colorSpaceType() && 
            dstColorSpace->getProfile() == m_lastUsedDstColorSpace->getProfile()) {
            tf = m_lastUsedTransform;
        }
    }

    if (!tf && m_profile && dstColorSpace->getProfile()) {

        if (!m_transforms.contains(dstColorSpace)) {
            tf = createTransform(dstColorSpace,
				 m_profile,
				 dstColorSpace->getProfile(),
				 renderingIntent);
            if (tf) {
		// XXX: Should we clear the transform cache if it gets too big?
		m_transforms[dstColorSpace] = tf;
            }
        }
        else {
            tf = m_transforms[dstColorSpace];
        }

        if ( tf ) {
            m_lastUsedTransform = tf;
            m_lastUsedDstColorSpace = dstColorSpace;
        }
    }

    if (tf) {

        cmsDoTransform(tf, const_cast<quint8 *>(src), dst, numPixels);

        // Lcms does nothing to the destination alpha channel so we must convert that manually.
        while (numPixels > 0) {
            quint8 alpha = getAlpha(src);
            dstColorSpace->setAlpha(dst, alpha, 1);

            src += srcPixelSize;
            dst += dstPixelSize;
            numPixels--;
        }

        return true;
    }

    // Last resort fallback. This will be removed when this class is renamed KisLCMSColorSpace after 1.5.
    while (numPixels > 0) {
        QColor color;
        quint8 opacity;

        toQColor(src, &color, &opacity);
        dstColorSpace->fromQColor(color, opacity, dst);

        src += srcPixelSize;
        dst += dstPixelSize;
        numPixels--;
    }

    return true;
}


KisColorAdjustment *KisAbstractColorSpace::createBrightnessContrastAdjustment(quint16 *transferValues)
{
    if (!m_profile) return 0;

    LPGAMMATABLE transferFunctions[3];
    transferFunctions[0] = cmsBuildGamma(256, 1.0);
    transferFunctions[1] = cmsBuildGamma(256, 1.0);
    transferFunctions[2] = cmsBuildGamma(256, 1.0);

    for(int i =0; i < 256; i++)
        transferFunctions[0]->GammaTable[i] = transferValues[i];

    KisColorAdjustmentImpl *adj = new KisColorAdjustmentImpl;
    adj->profiles[1] = cmsCreateLinearizationDeviceLink(icSigLabData, transferFunctions);
    cmsSetDeviceClass(adj->profiles[1], icSigAbstractClass);

    adj->profiles[0] = m_profile->profile();
    adj->profiles[2] = m_profile->profile();
    adj->transform  = cmsCreateMultiprofileTransform(adj->profiles, 3, m_cmType, m_cmType, INTENT_PERCEPTUAL, 0);
    adj->csProfile = m_profile->profile();
    return adj;
}

typedef struct {
                double Saturation;

} BCHSWADJUSTS, *LPBCHSWADJUSTS;


static int desaturateSampler(register WORD In[], register WORD Out[], register LPVOID /*Cargo*/)
{
    cmsCIELab LabIn, LabOut;
    cmsCIELCh LChIn, LChOut;
    //LPBCHSWADJUSTS bchsw = (LPBCHSWADJUSTS) Cargo;

    cmsLabEncoded2Float(&LabIn, In);

    cmsLab2LCh(&LChIn, &LabIn);

    // Do some adjusts on LCh
    LChOut.L = LChIn.L;
    LChOut.C = 0;//LChIn.C + bchsw->Saturation;
    LChOut.h = LChIn.h;

    cmsLCh2Lab(&LabOut, &LChOut);

    // Back to encoded
    cmsFloat2LabEncoded(Out, &LabOut);

    return TRUE;
}

KisColorAdjustment *KisAbstractColorSpace::createDesaturateAdjustment()
{
    if (!m_profile) return 0;

    KisColorAdjustmentImpl *adj = new KisColorAdjustmentImpl;

    adj->profiles[0] = m_profile->profile();
    adj->profiles[2] = m_profile->profile();
    adj->csProfile = m_profile->profile();
    
     LPLUT Lut;
     BCHSWADJUSTS bchsw;

     bchsw.Saturation = -25;

     adj->profiles[1] = _cmsCreateProfilePlaceholder();
     if (!adj->profiles[1]) // can't allocate
        return NULL;

     cmsSetDeviceClass(adj->profiles[1], icSigAbstractClass);
     cmsSetColorSpace(adj->profiles[1], icSigLabData);
     cmsSetPCS(adj->profiles[1], icSigLabData);

     cmsSetRenderingIntent(adj->profiles[1], INTENT_PERCEPTUAL);

     // Creates a LUT with 3D grid only
     Lut = cmsAllocLUT();

     cmsAlloc3DGrid(Lut, 32, 3, 3);

     if (!cmsSample3DGrid(Lut, desaturateSampler, static_cast<LPVOID>(&bchsw), 0)) {
         // Shouldn't reach here
         cmsFreeLUT(Lut);
         cmsCloseProfile(adj->profiles[1]);
         return NULL;
     }

    // Create tags

    cmsAddTag(adj->profiles[1], icSigDeviceMfgDescTag,      (LPVOID) "(krita internal)");
    cmsAddTag(adj->profiles[1], icSigProfileDescriptionTag, (LPVOID) "krita saturation abstract profile");
    cmsAddTag(adj->profiles[1], icSigDeviceModelDescTag,    (LPVOID) "saturation built-in");

    cmsAddTag(adj->profiles[1], icSigMediaWhitePointTag, (LPVOID) cmsD50_XYZ());

    cmsAddTag(adj->profiles[1], icSigAToB0Tag, (LPVOID) Lut);

    // LUT is already on virtual profile
    cmsFreeLUT(Lut);

    adj->transform  = cmsCreateMultiprofileTransform(adj->profiles, 3, m_cmType, m_cmType, INTENT_PERCEPTUAL, 0);

    return adj;
}

KisColorAdjustment *KisAbstractColorSpace::createPerChannelAdjustment(quint16 **transferValues)
{
    if (!m_profile) return 0;

    LPGAMMATABLE *transferFunctions = new LPGAMMATABLE[nColorChannels()+1];

    for(uint ch=0; ch < nColorChannels(); ch++) {
        transferFunctions[ch] = cmsBuildGamma(256, 1.0);
        for(uint i =0; i < 256; i++) {
            transferFunctions[ch]->GammaTable[i] = transferValues[ch][i];
        }
    }

    KisColorAdjustmentImpl *adj = new KisColorAdjustmentImpl;
    adj->profiles[0] = cmsCreateLinearizationDeviceLink(colorSpaceSignature(), transferFunctions);
    adj->profiles[1] = NULL;
    adj->profiles[2] = NULL;
    adj->csProfile = m_profile->profile();
    adj->transform  = cmsCreateTransform(adj->profiles[0], m_cmType, NULL, m_cmType, INTENT_PERCEPTUAL, 0);

    delete [] transferFunctions;

    return adj;
}


void KisAbstractColorSpace::applyAdjustment(const quint8 *src, quint8 *dst, KisColorAdjustment *adjustment, qint32 nPixels)
{
    KisColorAdjustmentImpl * adj = dynamic_cast<KisColorAdjustmentImpl*>(adjustment);
    if (adj)
        cmsDoTransform(adj->transform, const_cast<quint8 *>(src), dst, nPixels);
}


void KisAbstractColorSpace::invertColor(quint8 * src, qint32 nPixels)
{
    QColor c;
    quint8 opacity;
    quint32 psize = pixelSize();

    while (nPixels--)
    {
        toQColor(src, &c, &opacity);
        c.setRgb(quint8_MAX - c.red(), quint8_MAX - c.green(), quint8_MAX - c.blue());
        fromQColor( c, opacity, src);

        src += psize;
    }
}

quint8 KisAbstractColorSpace::difference(const quint8* src1, const quint8* src2)
{
    if (m_defaultToLab) {

        quint8 lab1[8], lab2[8];
        cmsCIELab labF1, labF2;

        if (getAlpha(src1) == OPACITY_TRANSPARENT || getAlpha(src2) == OPACITY_TRANSPARENT)
            return (getAlpha(src1) == getAlpha(src2) ? 0 : 255);

        cmsDoTransform( m_defaultToLab, const_cast<quint8*>( src1 ), lab1, 1);
        cmsDoTransform( m_defaultToLab, const_cast<quint8*>( src2 ), lab2, 1);
        cmsLabEncoded2Float(&labF1, (WORD *)lab1);
        cmsLabEncoded2Float(&labF2, (WORD *)lab2);
        double diff = cmsDeltaE(&labF1, &labF2);
        if(diff>255)
            return 255;
        else
            return qint8(diff);
    }
    else {
        QColor c1;
        quint8 opacity1;
        toQColor(src1, &c1, &opacity1);

        QColor c2;
        quint8 opacity2;
        toQColor(src2, &c2, &opacity2);

        quint8 red = abs(c1.red() - c2.red());
        quint8 green = abs(c1.green() - c2.green());
        quint8 blue = abs(c1.blue() - c2.blue());
        return qMax(red, qMax(green, blue));
    }
}

void KisAbstractColorSpace::mixColors(const quint8 **colors, const quint8 *weights, quint32 nColors, quint8 *dst) const
{
    quint32 totalRed = 0, totalGreen = 0, totalBlue = 0, newAlpha = 0;

    QColor c;
    quint8 opacity;

    while (nColors--)
    {
        // Ugly hack to get around the current constness mess of the colour strategy...
        const_cast<KisAbstractColorSpace *>(this)->toQColor(*colors, &c, &opacity);

        quint32 alphaTimesWeight = UINT8_MULT(opacity, *weights);

        totalRed += c.red() * alphaTimesWeight;
        totalGreen += c.green() * alphaTimesWeight;
        totalBlue += c.blue() * alphaTimesWeight;
        newAlpha += alphaTimesWeight;

        weights++;
        colors++;
    }

    Q_ASSERT(newAlpha <= 255);

    if (newAlpha > 0) {
        totalRed = UINT8_DIVIDE(totalRed, newAlpha);
        totalGreen = UINT8_DIVIDE(totalGreen, newAlpha);
        totalBlue = UINT8_DIVIDE(totalBlue, newAlpha);
    }

    // Divide by 255.
    totalRed += 0x80;

    quint32 dstRed = ((totalRed >> 8) + totalRed) >> 8;
    Q_ASSERT(dstRed <= 255);

    totalGreen += 0x80;
    quint32 dstGreen = ((totalGreen >> 8) + totalGreen) >> 8;
    Q_ASSERT(dstGreen <= 255);

    totalBlue += 0x80;
    quint32 dstBlue = ((totalBlue >> 8) + totalBlue) >> 8;
    Q_ASSERT(dstBlue <= 255);

    const_cast<KisAbstractColorSpace *>(this)->fromQColor(QColor(dstRed, dstGreen, dstBlue), newAlpha, dst);
}

void KisAbstractColorSpace::convolveColors(quint8** colors, qint32 * kernelValues, KisChannelInfo::enumChannelFlags channelFlags,
                                           quint8 *dst, qint32 factor, qint32 offset, qint32 nColors) const
{
    qint32 totalRed = 0, totalGreen = 0, totalBlue = 0, totalAlpha = 0;

    QColor dstColor;
    quint8 dstOpacity;

    const_cast<KisAbstractColorSpace *>(this)->toQColor(dst, &dstColor, &dstOpacity);

    while (nColors--)
    {
        qint32 weight = *kernelValues;

        if (weight != 0) {
            QColor c;
            quint8 opacity;
            const_cast<KisAbstractColorSpace *>(this)->toQColor( *colors, &c, &opacity );
            totalRed += c.Qt::red() * weight;
            totalGreen += c.Qt::green() * weight;
            totalBlue += c.Qt::blue() * weight;
            totalAlpha += opacity * weight;
        }
        colors++;
        kernelValues++;
    }


    if (channelFlags & KisChannelInfo::FLAG_COLOR) {
        const_cast<KisAbstractColorSpace *>(this)->fromQColor(QColor(CLAMP((totalRed / factor) + offset, 0, quint8_MAX),
                                        CLAMP((totalGreen / factor) + offset, 0, quint8_MAX),
                                        CLAMP((totalBlue / factor) + offset, 0, quint8_MAX)),
            dstOpacity,
            dst);
    }
    if (channelFlags & KisChannelInfo::FLAG_ALPHA) {
        const_cast<KisAbstractColorSpace *>(this)->fromQColor(dstColor, CLAMP((totalAlpha/ factor) + offset, 0, quint8_MAX), dst);
    }

}

void KisAbstractColorSpace::darken(const quint8 * src, quint8 * dst, qint32 shade, bool compensate, double compensation, qint32 nPixels) const
{
    if (m_defaultToLab) {
        quint16 * labcache = new quint16[nPixels * 4];
        cmsDoTransform( m_defaultToLab, const_cast<quint8*>( src ), reinterpret_cast<quint8*>( labcache ), nPixels );
        for ( int i = 0; i < nPixels * 4; ++i ) {
            if ( compensate ) {
                labcache[i] = static_cast<quint16>( ( labcache[i] * shade ) / ( compensation * 255 ) );
            }
            else {
                labcache[i] = static_cast<quint16>( labcache[i] * shade  / 255 );
            }
        }
        cmsDoTransform( m_defaultFromLab, reinterpret_cast<quint8*>( labcache ), dst, nPixels );

        // Copy alpha
        for ( int i = 0; i < nPixels; ++i ) {
            quint8 alpha = getAlpha( src );
            setAlpha( dst, alpha, 1 );
        }
        delete [] labcache;
    }
    else {

        QColor c;
        qint32 psize = pixelSize();

        for (int i = 0; i < nPixels; ++i) {

            const_cast<KisAbstractColorSpace *>(this)->toQColor(src + (i * psize), &c);
            qint32 r, g, b;

            if (compensate) {
                r = static_cast<qint32>( qMin(255, (c.red() * shade) / (compensation * 255)));
                g = static_cast<qint32>( qMin(255, (c.green() * shade) / (compensation * 255)));
                b = static_cast<qint32>( qMin(255, (c.blue() * shade) / (compensation * 255)));
            }
            else {
                r = static_cast<qint32>( qMin(255, (c.red() * shade / 255)));
                g = static_cast<qint32>( qMin(255, (c.green() * shade / 255)));
                b = static_cast<qint32>( qMin(255, (c.blue() * shade / 255)));
            }
            c.setRgb(r, g, b);

            const_cast<KisAbstractColorSpace *>(this)->fromQColor( c, dst  + (i * psize));
        }
    }
}

quint8 KisAbstractColorSpace::intensity8(const quint8 * src) const
{
    QColor c;
    quint8 opacity;
    const_cast<KisAbstractColorSpace *>(this)->toQColor(src, &c, &opacity);
    return static_cast<quint8>((c.red() * 0.30 + c.green() * 0.59 + c.blue() * 0.11) + 0.5);

}


KisID KisAbstractColorSpace::mathToolboxID() const
{
    return KisID("Basic");
}

void KisAbstractColorSpace::bitBlt(quint8 *dst,
                   qint32 dststride,
                   KisColorSpace * srcSpace,
                   const quint8 *src,
                   qint32 srcRowStride,
                   const quint8 *srcAlphaMask,
                   qint32 maskRowStride,
                   quint8 opacity,
                   qint32 rows,
                   qint32 cols,
                   const KisCompositeOp& op)
{
    if (rows <= 0 || cols <= 0)
        return;

    if (this != srcSpace) {
        quint32 len = pixelSize() * rows * cols;

        // If our conversion cache is too small, extend it.
        if (!m_conversionCache.resize( len, Q3GArray::SpeedOptim )) {
            kWarning() << "Could not allocate enough memory for the conversion!\n";
            // XXX: We should do a slow, pixel by pixel bitblt here...
            abort();
        }

        for (qint32 row = 0; row < rows; row++) {
            srcSpace->convertPixelsTo(src + row * srcRowStride,
                                        m_conversionCache.data() + row * cols * pixelSize(), this,
                                        cols);
        }

        // The old srcRowStride is no longer valid because we converted to the current cs
        srcRowStride = cols * pixelSize();

        bitBlt(dst,
               dststride,
               m_conversionCache.data(),
               srcRowStride,
               srcAlphaMask,
               maskRowStride,
               opacity,
               rows,
               cols,
               op);

    }
    else {
        bitBlt(dst,
               dststride,
               src,
               srcRowStride,
               srcAlphaMask,
               maskRowStride,
               opacity,
               rows,
               cols,
               op);
    }
}

QImage KisAbstractColorSpace::convertToQImage(const quint8 *data, qint32 width, qint32 height,
                                              KisProfile *dstProfile,
                                              qint32 renderingIntent, float /*exposure*/)

{
    QImage img = QImage(width, height, 32, 0, QImage::LittleEndian);
    img.setAlphaBuffer( true );

    KisColorSpace * dstCS;

    if (dstProfile)
        dstCS = m_parent->getColorSpace(KisID("RGBA",""),dstProfile->productName());
    else
        dstCS = m_parent->getRGB8();

    if (data)
        convertPixelsTo(const_cast<quint8 *>(data), img.bits(), dstCS, width * height, renderingIntent);

    return img;
}


cmsHTRANSFORM KisAbstractColorSpace::createTransform(KisColorSpace * dstColorSpace,
                             KisProfile *  srcProfile,
                             KisProfile *  dstProfile,
                             qint32 renderingIntent)
{
    KConfig * cfg = KGlobal::config();
    bool bpCompensation = cfg->readBoolEntry("useBlackPointCompensation", false);

    int flags = 0;

    if (bpCompensation) {
        flags = cmsFLAGS_BLACKPOINTCOMPENSATION;
    }

    if (dstColorSpace && dstProfile && srcProfile ) {
        cmsHTRANSFORM tf = cmsCreateTransform(srcProfile->profile(),
                              colorSpaceType(),
                              dstProfile->profile(),
                              dstColorSpace->colorSpaceType(),
                              renderingIntent,
                              flags);

        return tf;
    }
    return 0;
}

void KisAbstractColorSpace::compositeCopy(quint8 *dstRowStart, qint32 dstRowStride, const quint8 *srcRowStart, qint32 srcRowStride, const quint8 * /*maskRowStart*/, qint32 /*maskRowStride*/, qint32 rows, qint32 numColumns, quint8 opacity)
{
    quint8 *dst = dstRowStart;
    const quint8 *src = srcRowStart;
    qint32 bytesPerPixel = pixelSize();

    while (rows > 0) {
        memcpy(dst, src, numColumns * bytesPerPixel);

        if (opacity != OPACITY_OPAQUE) {
            multiplyAlpha(dst, opacity, numColumns);
        }

        dst += dstRowStride;
        src += srcRowStride;
        --rows;
    }
}

