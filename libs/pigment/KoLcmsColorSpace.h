/*
 *  Copyright (c) 2002 Patrick Julien  <freak@codepimps.org>
 *  Copyright (c) 2005-2006 Casper Boemann <cbr@boemann.dk>
 *  Copyright (c) 2004-2006 Cyrille Berger <cberger@cberger.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#ifndef KOLCMSCOLORSPACE_H_
#define KOLCMSCOLORSPACE_H_

#include <kconfig.h>
#include <kglobal.h>

#include <KoColorProfile.h>
#include <KoColorSpaceAbstract.h>
#include <KoColorSpaceRegistry.h>

struct KoLcmsColorTransformation : public KoColorTransformation
{
    KoLcmsColorTransformation() : KoColorTransformation()
    {
        csProfile = 0;
        cmstransform = 0;
        profiles[0] = 0;
        profiles[1] = 0;
        profiles[2] = 0;
    };

    ~KoLcmsColorTransformation() {

        if (cmstransform)
            cmsDeleteTransform(cmstransform);
        if (profiles[0] && profiles[0] != csProfile)
            cmsCloseProfile(profiles[0]);
        if(profiles[1] && profiles[1] != csProfile)
            cmsCloseProfile(profiles[1]);
        if(profiles[2] && profiles[2] != csProfile)
            cmsCloseProfile(profiles[2]);
    }

    virtual void transform(const quint8 *src, quint8 *dst, qint32 nPixels) const
    {
        cmsDoTransform(cmstransform, const_cast<quint8 *>(src), dst, nPixels);
    }

    cmsHPROFILE csProfile;
    cmsHPROFILE profiles[3];
    cmsHTRANSFORM cmstransform;
};

/**
 * This is the base class for all colorspaces that are based on the lcms library, for instance
 * RGB 8bits and 16bits, CMYK 8bits and 16bits, LAB...
 */
template<class _CSTraits>
class KoLcmsColorSpace : public KoColorSpaceAbstract<_CSTraits> {
    protected:
        KoLcmsColorSpace(const QString &id, const QString &name, KoColorSpaceRegistry * parent, DWORD cmType,
                         icColorSpaceSignature colorSpaceSignature,
                         KoColorProfile *p) : KoColorSpaceAbstract<_CSTraits>(id, name, parent, cmType, colorSpaceSignature), m_profile( p )

        {
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
            m_defaultToRGB16 = 0;
            m_defaultFromRGB16 = 0;
        }

        void init()
        {
    // Default pixel buffer for QColor conversion
            m_qcolordata = new quint8[3];
            Q_CHECK_PTR(m_qcolordata);
            Q_ASSERT(m_profile );
//             if (m_profile == 0) { return; }

    // For conversions from default rgb
            m_lastFromRGB = cmsCreate_sRGBProfile();

            m_defaultFromRGB = cmsCreateTransform(m_lastFromRGB, TYPE_BGR_8,
                    m_profile->profile(), this->colorSpaceType(),
                    INTENT_PERCEPTUAL, 0);

            m_defaultToRGB =  cmsCreateTransform(m_profile->profile(), this->colorSpaceType(),
                    m_lastFromRGB, TYPE_BGR_8,
                    INTENT_PERCEPTUAL, 0);

            m_defaultFromRGB16 = cmsCreateTransform(m_lastFromRGB, TYPE_BGR_16,
                    m_profile->profile(), this->colorSpaceType(),
                    INTENT_PERCEPTUAL, 0);

            m_defaultToRGB16 =  cmsCreateTransform(m_profile->profile(), this->colorSpaceType(),
                    m_lastFromRGB, TYPE_BGR_16,
                    INTENT_PERCEPTUAL, 0);

            cmsHPROFILE hLab  = cmsCreateLabProfile(NULL);

            m_defaultFromLab = cmsCreateTransform(hLab, TYPE_Lab_16, m_profile->profile(), this->colorSpaceType(),
                    INTENT_PERCEPTUAL, 0);

            m_defaultToLab = cmsCreateTransform(m_profile->profile(), this->colorSpaceType(), hLab, TYPE_Lab_16,
                    INTENT_PERCEPTUAL, 0);
        }
    public:
        
        virtual bool hasHighDynamicRange() const { return false; }
        virtual KoColorProfile * profile() const { return m_profile; };
        
        virtual void fromQColor(const QColor& color, quint8 *dst, KoColorProfile * profile=0) const
        {
            m_qcolordata[2] = color.red();
            m_qcolordata[1] = color.green();
            m_qcolordata[0] = color.blue();


            if (profile == 0) {
	    // Default sRGB
                if (!m_defaultFromRGB) return;

                cmsDoTransform(m_defaultFromRGB, m_qcolordata, dst, 1);
            }
            else {
                if (m_lastFromRGB == 0 || (m_lastFromRGB != 0 && m_lastRGBProfile != profile->profile())) {
                    m_lastFromRGB = cmsCreateTransform(profile->profile(), TYPE_BGR_8,
                            m_profile->profile(), this->colorSpaceType(),
                            INTENT_PERCEPTUAL, 0);
                    m_lastRGBProfile = profile->profile();

                }
                cmsDoTransform(m_lastFromRGB, m_qcolordata, dst, 1);
            }

            this->setAlpha(dst, OPACITY_OPAQUE, 1);
        }

        virtual void fromQColor(const QColor& color, quint8 opacity, quint8 *dst, KoColorProfile * profile=0) const
        {
            this->fromQColor(color, dst, profile);
            this->setAlpha(dst, opacity, 1);
        }

        virtual void toQColor(const quint8 *src, QColor *c, KoColorProfile * profile =0) const
        {
            if (profile == 0) {
	// Default sRGB transform
                if (!m_defaultToRGB) return;
                cmsDoTransform(m_defaultToRGB, const_cast <quint8 *>(src), m_qcolordata, 1);
            }
            else {
                if (m_lastToRGB == 0 || (m_lastToRGB != 0 && m_lastRGBProfile != profile->profile())) {
                    m_lastToRGB = cmsCreateTransform(m_profile->profile(), this->colorSpaceType(),
                            profile->profile(), TYPE_BGR_8,
                            INTENT_PERCEPTUAL, 0);
                    m_lastRGBProfile = profile->profile();
                }
                cmsDoTransform(m_lastToRGB, const_cast <quint8 *>(src), m_qcolordata, 1);
            }
            c->setRgb(m_qcolordata[2], m_qcolordata[1], m_qcolordata[0]);
        }

        virtual void toQColor(const quint8 *src, QColor *c, quint8 *opacity, KoColorProfile * profile =0) const
        {
            this->toQColor(src, c, profile);
            *opacity = this->alpha(src);
        }
        virtual QImage convertToQImage(const quint8 *data, qint32 width, qint32 height,
                KoColorProfile *dstProfile,
                qint32 renderingIntent, float exposure) const

        {
            Q_UNUSED(exposure);
            QImage img = QImage(width, height, QImage::Format_ARGB32);

            KoColorSpace * dstCS;

            if (dstProfile)
                dstCS = this->m_parent->colorSpace("RGBA", dstProfile);
            else
                dstCS = this->m_parent->rgb8();

            if (data)
                convertPixelsTo(const_cast<quint8 *>(data), img.bits(), dstCS, width * height, renderingIntent);

            return img;
        }
        virtual void toLabA16(const quint8 * src, quint8 * dst, const quint32 nPixels) const
        {
            if ( m_defaultToLab == 0 ) return;

            cmsDoTransform( m_defaultToLab, const_cast<quint8 *>( src ), dst, nPixels );
        }

        virtual void fromLabA16(const quint8 * src, quint8 * dst, const quint32 nPixels) const
        {
            if ( m_defaultFromLab == 0 ) return;

            cmsDoTransform( m_defaultFromLab,  const_cast<quint8 *>( src ), dst,  nPixels );
        }
        virtual void fromRgbA16(const quint8 * src, quint8 * dst, const quint32 nPixels) const
        {
            if ( m_defaultFromRGB16 == 0 ) return;

            cmsDoTransform( m_defaultFromRGB16,  const_cast<quint8 *>( src ), dst,  nPixels );
        }
        virtual void toRgbA16(const quint8 * src, quint8 * dst, const quint32 nPixels) const
        {
            if ( m_defaultToRGB16 == 0 ) return;

            cmsDoTransform( m_defaultToRGB16, const_cast<quint8 *>( src ), dst, nPixels );
        }

        virtual bool convertPixelsTo(const quint8 * src,
                quint8 * dst,
                const KoColorSpace * dstColorSpace,
                quint32 numPixels,
                qint32 renderingIntent) const
        {
            if (dstColorSpace->colorSpaceType() == this->colorSpaceType()
                && dstColorSpace->profile() == profile())
            {
                if (src!= dst)
                    memcpy (dst, src, numPixels * this->pixelSize());

                return true;
            }

            cmsHTRANSFORM tf = 0;

            qint32 srcPixelSize = this->pixelSize();
            qint32 dstPixelSize = dstColorSpace->pixelSize();

            if (m_lastUsedTransform != 0 && m_lastUsedDstColorSpace != 0) {
                if (dstColorSpace->colorSpaceType() == m_lastUsedDstColorSpace->colorSpaceType() &&
                    dstColorSpace->profile() == m_lastUsedDstColorSpace->profile()) {
                    tf = m_lastUsedTransform;
                    }
            }

            if (!tf && m_profile && dstColorSpace->profile()) {

                if (!m_transforms.contains(dstColorSpace)) {
                    tf = this->createTransform(dstColorSpace,
                            m_profile,
                            dstColorSpace->profile(),
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
                    quint8 alpha = this->alpha(src);
                    dstColorSpace->setAlpha(dst, alpha, 1);

                    src += srcPixelSize;
                    dst += dstPixelSize;
                    numPixels--;
                }

                return true;
            }

        // Last resort fallback. This happens when either src or dst isn't a lcms colorspace.
            return KoColorSpace::convertPixelsTo(src, dst, dstColorSpace, numPixels, renderingIntent);
        }

        virtual KoColorTransformation *createBrightnessContrastAdjustment(quint16 *transferValues) const
        {
            if (!m_profile) return 0;

            LPGAMMATABLE transferFunctions[3];
            transferFunctions[0] = cmsBuildGamma(256, 1.0);
            transferFunctions[1] = cmsBuildGamma(256, 1.0);
            transferFunctions[2] = cmsBuildGamma(256, 1.0);

            for(int i =0; i < 256; i++)
                transferFunctions[0]->GammaTable[i] = transferValues[i];

            KoLcmsColorTransformation *adj = new KoLcmsColorTransformation;
            adj->profiles[1] = cmsCreateLinearizationDeviceLink(icSigLabData, transferFunctions);
            cmsSetDeviceClass(adj->profiles[1], icSigAbstractClass);

            adj->profiles[0] = m_profile->profile();
            adj->profiles[2] = m_profile->profile();
            adj->cmstransform  = cmsCreateMultiprofileTransform(adj->profiles, 3, this->colorSpaceType(), this->colorSpaceType(), INTENT_PERCEPTUAL, 0);
            adj->csProfile = m_profile->profile();
            return adj;
        }


        virtual KoColorTransformation *createDesaturateAdjustment() const
        {
            if (!m_profile) return 0;

            KoLcmsColorTransformation *adj = new KoLcmsColorTransformation;

            adj->profiles[0] = m_profile->profile();
            adj->profiles[2] = m_profile->profile();
            adj->csProfile = m_profile->profile();

            LPLUT Lut;
            BCHSWADJUSTS bchsw;

            bchsw.Saturation = -25;

            adj->profiles[1] = _cmsCreateProfilePlaceholder();
            if (!adj->profiles[1]) { // can't allocate
                delete adj;
                return NULL;
            }

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
                delete adj;
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

            adj->cmstransform  = cmsCreateMultiprofileTransform(adj->profiles, 3, this->colorSpaceType(), this->colorSpaceType(), INTENT_PERCEPTUAL, 0);

            return adj;
        }

        virtual KoColorTransformation *createPerChannelAdjustment(quint16 **transferValues) const
        {
            if (!m_profile) return 0;

            LPGAMMATABLE *transferFunctions = new LPGAMMATABLE[ _CSTraits::channels_nb +1];

            for(uint ch=0; ch < _CSTraits::channels_nb; ch++) {
                transferFunctions[ch] = cmsBuildGamma(256, 1.0);
                for(uint i =0; i < 256; i++) {
                    transferFunctions[ch]->GammaTable[i] = transferValues[ch][i];
                }
            }

            KoLcmsColorTransformation *adj = new KoLcmsColorTransformation;
            adj->profiles[0] = cmsCreateLinearizationDeviceLink(this->colorSpaceSignature(), transferFunctions);
            adj->profiles[1] = NULL;
            adj->profiles[2] = NULL;
            adj->csProfile = m_profile->profile();
            adj->cmstransform  = cmsCreateTransform(adj->profiles[0], this->colorSpaceType(), NULL, this->colorSpaceType(), INTENT_PERCEPTUAL, 0);

            delete [] transferFunctions;

            return adj;
        }

        virtual quint8 difference(const quint8* src1, const quint8* src2) const
        {
            if (m_defaultToLab) {

                quint8 lab1[8], lab2[8];
                cmsCIELab labF1, labF2;

                if (this->alpha(src1) == OPACITY_TRANSPARENT || this->alpha(src2) == OPACITY_TRANSPARENT)
                    return (this->alpha(src1) == this->alpha(src2) ? 0 : 255);

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

        void darken(const quint8 * src, quint8 * dst, qint32 shade, bool compensate, double compensation, qint32 nPixels) const
        { // TODO: move it as a fallback and therefor port it to RGB32
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
                    quint8 alpha = this->alpha( src );
                    this->setAlpha( dst, alpha, 1 );
                }
                delete [] labcache;
            }
            else {

                QColor c;
                qint32 psize = this->pixelSize();

                for (int i = 0; i < nPixels; ++i) {

                    const_cast<KoLcmsColorSpace<_CSTraits>* >(this)->toQColor(src + (i * psize), &c);
                    qint32 r, g, b;

                    if (compensate) {
                        r = static_cast<qint32>( qMin(255, static_cast<qint32>((c.red() * shade) / (compensation * 255))));
                        g = static_cast<qint32>( qMin(255, static_cast<qint32>((c.green() * shade) / (compensation * 255))));
                        b = static_cast<qint32>( qMin(255, static_cast<qint32>((c.blue() * shade) / (compensation * 255))));
                    }
                    else {
                        r = static_cast<qint32>( qMin(255, (c.red() * shade / 255)));
                        g = static_cast<qint32>( qMin(255, (c.green() * shade / 255)));
                        b = static_cast<qint32>( qMin(255, (c.blue() * shade / 255)));
                    }
                    c.setRgb(r, g, b);

                    const_cast<KoLcmsColorSpace<_CSTraits>* >(this)->fromQColor( c, dst  + (i * psize));
                }
            }
        }


    private:
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

            return true;
        }
        cmsHTRANSFORM createTransform(const KoColorSpace * dstColorSpace,
                KoColorProfile *  srcProfile,
                KoColorProfile *  dstProfile,
                qint32 renderingIntent) const
        {
            KConfig * cfg = KGlobal::config();
            bool bpCompensation = cfg->readEntry("useBlackPointCompensation", false);

            int flags = 0;

            if (bpCompensation) {
                flags = cmsFLAGS_BLACKPOINTCOMPENSATION;
            }

            if (dstColorSpace && dstProfile && srcProfile ) {
                cmsHTRANSFORM tf = cmsCreateTransform(srcProfile->profile(),
                        this->colorSpaceType(),
                        dstProfile->profile(),
                        dstColorSpace->colorSpaceType(),
                        renderingIntent,
                        flags);

                return tf;
            }
            return 0;
        }
    private:
        mutable quint8 * m_qcolordata; // A small buffer for conversion from and to qcolor.
        cmsHTRANSFORM m_defaultToRGB;    // Default transform to 8 bit sRGB
        cmsHTRANSFORM m_defaultFromRGB;  // Default transform from 8 bit sRGB
        cmsHTRANSFORM m_defaultToRGB16;    // Default transform to 16 bit sRGB
        cmsHTRANSFORM m_defaultFromRGB16;  // Default transform from 16 bit sRGB

        mutable cmsHPROFILE   m_lastRGBProfile;  // Last used profile to transform to/from RGB
        mutable cmsHTRANSFORM m_lastToRGB;       // Last used transform to transform to RGB
        mutable cmsHTRANSFORM m_lastFromRGB;     // Last used transform to transform from RGB

        cmsHTRANSFORM m_defaultToLab;
        cmsHTRANSFORM m_defaultFromLab;

        KoColorProfile *  m_profile;
        mutable const KoColorSpace *m_lastUsedDstColorSpace;
        mutable cmsHTRANSFORM m_lastUsedTransform;
        
    // cmsHTRANSFORM is a void *, so this should work.
        typedef QMap<const KoColorSpace *, cmsHTRANSFORM>  TransformMap;
        mutable TransformMap m_transforms; // Cache for existing transforms

};


#endif
