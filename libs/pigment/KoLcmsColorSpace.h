/*
 *  Copyright (c) 2002 Patrick Julien  <freak@codepimps.org>
 *  Copyright (c) 2005-2006 Casper Boemann <cbr@boemann.dk>
 *  Copyright (c) 2004,2006-2007 Cyrille Berger <cberger@cberger.net>
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

#include <colorprofiles/KoLcmsColorProfile.h>
#include <KoColorSpaceAbstract.h>
#include <KoColorSpaceRegistry.h>

#include <pigment_export.h>

struct KoLcmsColorTransformation : public KoColorTransformation
{
    KoLcmsColorTransformation() : KoColorTransformation()
    {
        csProfile = 0;
        cmstransform = 0;
        profiles[0] = 0;
        profiles[1] = 0;
        profiles[2] = 0;
    }

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

struct KoLcmsDarkenTransformation : public KoColorTransformation
{
    KoLcmsDarkenTransformation(const KoColorSpace* cs, cmsHTRANSFORM defaultToLab, cmsHTRANSFORM defaultFromLab, qint32 shade, bool compensate, double compensation) : m_colorSpace(cs), m_defaultToLab(defaultToLab), m_defaultFromLab(defaultFromLab), m_shade(shade), m_compensate(compensate), m_compensation(compensation)
    {

    }
    virtual void transform(const quint8 *src, quint8 *dst, qint32 nPixels) const
    {
        quint16 * labcache = new quint16[nPixels * 4];
        cmsDoTransform( m_defaultToLab, const_cast<quint8*>( src ), reinterpret_cast<quint8*>( labcache ), nPixels );
        for ( int i = 0; i < nPixels * 4; ++i ) {
            if ( m_compensate ) {
                labcache[i] = static_cast<quint16>( ( labcache[i] * m_shade ) / ( m_compensation * 255 ) );
            }
            else {
                labcache[i] = static_cast<quint16>( labcache[i] * m_shade  / 255 );
            }
        }
        cmsDoTransform( m_defaultFromLab, reinterpret_cast<quint8*>( labcache ), dst, nPixels );
        // Copy alpha
        for ( int i = 0; i < nPixels; ++i ) {
            quint8 alpha = m_colorSpace->alpha( src );
            m_colorSpace->setAlpha( dst, alpha, 1 );
        }
        delete [] labcache;
    }
    const KoColorSpace* m_colorSpace;
    cmsHTRANSFORM m_defaultToLab;
    cmsHTRANSFORM m_defaultFromLab;
    qint32 m_shade;
    bool m_compensate;
    double m_compensation;
};

class PIGMENT_EXPORT KoLcmsColorConversionTransformation : public KoColorConversionTransformation {
    public:
        KoLcmsColorConversionTransformation(const KoColorSpace* srcCs, KoLcmsColorProfile* srcProfile, const KoColorSpace* dstCs, KoLcmsColorProfile* dstProfile, Intent renderingIntent = IntentPerceptual) : KoColorConversionTransformation(srcCs, dstCs, renderingIntent), m_transform(0)
        {
            m_transform = this->createTransform(
                            srcProfile,
                            dstProfile,
                            renderingIntent);
        }
    public:
        virtual void transform(const quint8 *src, quint8 *dst, qint32 numPixels) const
        {
            Q_ASSERT(m_transform);

            qint32 srcPixelSize = srcColorSpace()->pixelSize();
            qint32 dstPixelSize = dstColorSpace()->pixelSize();

            cmsDoTransform(m_transform, const_cast<quint8 *>(src), dst, numPixels);

        // Lcms does nothing to the destination alpha channel so we must convert that manually.
            while (numPixels > 0) {
                quint8 alpha = srcColorSpace()->alpha(src);
                dstColorSpace()->setAlpha(dst, alpha, 1);

                src += srcPixelSize;
                dst += dstPixelSize;
                numPixels--;
            }

        }
    private:
        cmsHTRANSFORM createTransform(
                KoLcmsColorProfile *  srcProfile,
                KoLcmsColorProfile *  dstProfile,
                qint32 renderingIntent) const;
    private:
        mutable cmsHTRANSFORM m_transform;
};

class KoLcmsInfo {
    struct Private {
        DWORD cmType;  // The colorspace type as defined by littlecms
        icColorSpaceSignature colorSpaceSignature; // The colorspace signature as defined in icm/icc files
    };
    public:
        KoLcmsInfo(DWORD cmType, icColorSpaceSignature colorSpaceSignature) : d(new Private)
        {
            d->cmType = cmType;
            d->colorSpaceSignature = colorSpaceSignature;
        }
        virtual ~KoLcmsInfo()
        {
            delete d;
        }
        virtual quint32 colorSpaceType() const
        {
            return d->cmType;
        }

        virtual icColorSpaceSignature colorSpaceSignature() const
        {
            return d->colorSpaceSignature;
        }
    private:
        Private* const d;
};
/**
 * This is the base class for all colorspaces that are based on the lcms library, for instance
 * RGB 8bits and 16bits, CMYK 8bits and 16bits, LAB...
 */
template<class _CSTraits>
class KoLcmsColorSpace : public KoColorSpaceAbstract<_CSTraits>, public KoLcmsInfo {
        struct Private {
            mutable quint8 * qcolordata; // A small buffer for conversion from and to qcolor.
            cmsHTRANSFORM defaultToRGB;    // Default transform to 8 bit sRGB
            cmsHTRANSFORM defaultFromRGB;  // Default transform from 8 bit sRGB
            cmsHTRANSFORM defaultToRGB16;    // Default transform to 16 bit sRGB
            cmsHTRANSFORM defaultFromRGB16;  // Default transform from 16 bit sRGB

            mutable cmsHPROFILE   lastRGBProfile;  // Last used profile to transform to/from RGB
            mutable cmsHTRANSFORM lastToRGB;       // Last used transform to transform to RGB
            mutable cmsHTRANSFORM lastFromRGB;     // Last used transform to transform from RGB

            cmsHTRANSFORM defaultToLab;
            cmsHTRANSFORM defaultFromLab;

            KoLcmsColorProfile *  profile;
            mutable const KoColorSpace *lastUsedDstColorSpace;
            mutable KoColorConversionTransformation* lastUsedTransform;

        // cmsHTRANSFORM is a void *, so this should work.
            typedef QMap<const KoColorSpace *, KoColorConversionTransformation*>  TransformMap;
            mutable TransformMap transforms; // Cache for existing transforms
        };
    protected:
        KoLcmsColorSpace(const QString &id, const QString &name, KoColorSpaceRegistry * parent, DWORD cmType,
                         icColorSpaceSignature colorSpaceSignature,
                         KoColorProfile *p) : KoColorSpaceAbstract<_CSTraits>(id, name, parent), KoLcmsInfo( cmType, colorSpaceSignature), d( new Private())

        {
            d->profile = toLcmsProfile(p);
            d->qcolordata = 0;
            d->lastUsedDstColorSpace = 0;
            d->lastUsedTransform = 0;
            d->lastRGBProfile = 0;
            d->lastToRGB = 0;
            d->lastFromRGB = 0;
            d->defaultFromRGB = 0;
            d->defaultToRGB = 0;
            d->defaultFromLab = 0;
            d->defaultToLab = 0;
            d->defaultToRGB16 = 0;
            d->defaultFromRGB16 = 0;
        }
        virtual ~KoLcmsColorSpace()
        {
            delete d;
        }

        void init()
        {
    // Default pixel buffer for QColor conversion
            d->qcolordata = new quint8[3];
            Q_CHECK_PTR(d->qcolordata);
            Q_ASSERT(d->profile );
//             if (d->profile == 0) { return; }

    // For conversions from default rgb
            d->lastFromRGB = cmsCreate_sRGBProfile();

            d->defaultFromRGB = cmsCreateTransform(d->lastFromRGB, TYPE_BGR_8,
                    d->profile->lcmsProfile(), this->colorSpaceType(),
                    INTENT_PERCEPTUAL, 0);

            d->defaultToRGB =  cmsCreateTransform(d->profile->lcmsProfile(), this->colorSpaceType(),
                    d->lastFromRGB, TYPE_BGR_8,
                    INTENT_PERCEPTUAL, 0);

            d->defaultFromRGB16 = cmsCreateTransform(d->lastFromRGB, TYPE_BGRA_16,
                    d->profile->lcmsProfile(), this->colorSpaceType(),
                    INTENT_PERCEPTUAL, 0);

            d->defaultToRGB16 =  cmsCreateTransform(d->profile->lcmsProfile(), this->colorSpaceType(),
                    d->lastFromRGB, TYPE_BGRA_16,
                    INTENT_PERCEPTUAL, 0);

            cmsHPROFILE hLab  = cmsCreateLabProfile(NULL);

            d->defaultFromLab = cmsCreateTransform(hLab,  (COLORSPACE_SH(PT_Lab)|CHANNELS_SH(3)|BYTES_SH(2)|EXTRA_SH(1)), d->profile->lcmsProfile(), this->colorSpaceType(),
                    INTENT_PERCEPTUAL, 0);

            d->defaultToLab = cmsCreateTransform(d->profile->lcmsProfile(), this->colorSpaceType(), hLab,  (COLORSPACE_SH(PT_Lab)|CHANNELS_SH(3)|BYTES_SH(2)|EXTRA_SH(1)),
                    INTENT_PERCEPTUAL, 0);
        }
    public:

        virtual bool hasHighDynamicRange() const { return false; }
        virtual KoColorProfile * profile() const { return lcmsProfile(); }

        virtual bool profileIsCompatible(KoColorProfile* profile) const
        {
            KoLcmsColorProfile* p = dynamic_cast<KoLcmsColorProfile*>(profile);
            return (p && p->colorSpaceSignature() == colorSpaceSignature());
        }

        virtual void fromQColor(const QColor& color, quint8 *dst, KoColorProfile * koprofile=0) const
        {
            d->qcolordata[2] = color.red();
            d->qcolordata[1] = color.green();
            d->qcolordata[0] = color.blue();

            KoLcmsColorProfile* profile = toLcmsProfile(koprofile);
            if (profile == 0) {
	    // Default sRGB
                if (!d->defaultFromRGB) return;

                cmsDoTransform(d->defaultFromRGB, d->qcolordata, dst, 1);
            }
            else {
                if (d->lastFromRGB == 0 || (d->lastFromRGB != 0 && d->lastRGBProfile != profile->lcmsProfile())) {
                    d->lastFromRGB = cmsCreateTransform(profile->lcmsProfile(), TYPE_BGR_8,
                            d->profile->lcmsProfile(), this->colorSpaceType(),
                            INTENT_PERCEPTUAL, 0);
                    d->lastRGBProfile = profile->lcmsProfile();

                }
                cmsDoTransform(d->lastFromRGB, d->qcolordata, dst, 1);
            }

            this->setAlpha(dst, OPACITY_OPAQUE, 1);
        }

        virtual void fromQColor(const QColor& color, quint8 opacity, quint8 *dst, KoColorProfile * profile=0) const
        {
            this->fromQColor(color, dst, profile);
            this->setAlpha(dst, opacity, 1);
        }

        virtual void toQColor(const quint8 *src, QColor *c, KoColorProfile * koprofile =0) const
        {
            KoLcmsColorProfile* profile = toLcmsProfile(koprofile);
            if (profile == 0) {
	// Default sRGB transform
                if (!d->defaultToRGB) return;
                cmsDoTransform(d->defaultToRGB, const_cast <quint8 *>(src), d->qcolordata, 1);
            }
            else {
                if (d->lastToRGB == 0 || (d->lastToRGB != 0 && d->lastRGBProfile != profile->lcmsProfile())) {
                    d->lastToRGB = cmsCreateTransform(d->profile->lcmsProfile(), this->colorSpaceType(),
                            profile->lcmsProfile(), TYPE_BGR_8,
                            INTENT_PERCEPTUAL, 0);
                    d->lastRGBProfile = profile->lcmsProfile();
                }
                cmsDoTransform(d->lastToRGB, const_cast <quint8 *>(src), d->qcolordata, 1);
            }
            c->setRgb(d->qcolordata[2], d->qcolordata[1], d->qcolordata[0]);
        }

        virtual void toQColor(const quint8 *src, QColor *c, quint8 *opacity, KoColorProfile * profile =0) const
        {
            this->toQColor(src, c, profile);
            *opacity = this->alpha(src);
        }
        virtual QImage convertToQImage(const quint8 *data, qint32 width, qint32 height,
                KoColorProfile *dstProfile,
                KoColorConversionTransformation::Intent renderingIntent, float exposure) const

        {
            Q_UNUSED(exposure);
            QImage img = QImage(width, height, QImage::Format_ARGB32);

            KoColorSpace * dstCS;

            dstCS = KoColorSpaceRegistry::instance()->colorSpace("RGBA", dstProfile);

            if (data)
                convertPixelsTo(const_cast<quint8 *>(data), img.bits(), dstCS, width * height, renderingIntent);

            return img;
        }
        virtual void toLabA16(const quint8 * src, quint8 * dst, quint32 nPixels) const
        {
            if ( d->defaultToLab == 0 ) return;

            cmsDoTransform( d->defaultToLab, const_cast<quint8 *>( src ), dst, nPixels );
            quint16* dstU16 = reinterpret_cast<quint16*>(dst);
            for(quint32 i = 0; i < nPixels; i++)
            {
                dstU16[4*i+3] = KoColorSpaceMaths<quint8,quint16>::scaleToA(this->alpha(src));
                src += this->pixelSize();
            }
        }

        virtual void fromLabA16(const quint8 * src, quint8 * dst, quint32 nPixels) const
        {
            if ( d->defaultFromLab == 0 ) return;

            cmsDoTransform( d->defaultFromLab,  const_cast<quint8 *>( src ), dst,  nPixels );
            const quint16* srcU16 = reinterpret_cast<const quint16*>(src);
            for(quint32 i = 0; i < nPixels; i++)
            {
                this->setAlpha(dst, KoColorSpaceMaths<quint16,quint8>::scaleToA(srcU16[4*i+3]), 1 );
                dst += this->pixelSize();
            }
        }
        virtual void fromRgbA16(const quint8 * src, quint8 * dst, quint32 nPixels) const
        {
            if ( d->defaultFromRGB16 == 0 ) return;

            cmsDoTransform( d->defaultFromRGB16,  const_cast<quint8 *>( src ), dst,  nPixels );
            const quint16* srcU16 = reinterpret_cast<const quint16*>(src);
            for(quint32 i = 0; i < nPixels; i++)
            {
                this->setAlpha(dst, KoColorSpaceMaths<quint16,quint8>::scaleToA(srcU16[4*i+3]), 1 );
                dst += this->pixelSize();
            }
        }
        virtual void toRgbA16(const quint8 * src, quint8 * dst, quint32 nPixels) const
        {
            if ( d->defaultToRGB16 == 0 ) return;

            cmsDoTransform( d->defaultToRGB16, const_cast<quint8 *>( src ), dst, nPixels );
            quint16* dstU16 = reinterpret_cast<quint16*>(dst);
            for(quint32 i = 0; i < nPixels; i++)
            {
                dstU16[4*i+3] = KoColorSpaceMaths<quint8,quint16>::scaleToA(this->alpha(src));
                src += this->pixelSize();
            }
        }

        virtual KoColorConversionTransformation* createColorConverter(const KoColorSpace * dstColorSpace, KoColorConversionTransformation::Intent renderingIntent = KoColorConversionTransformation::IntentPerceptual) const
        {
            KoLcmsColorProfile* dstprofile = toLcmsProfile(dstColorSpace->profile());
            if(d->profile && dstprofile && dynamic_cast<const KoLcmsInfo*>(dstColorSpace))
            {
                return new KoLcmsColorConversionTransformation(this, d->profile, dstColorSpace, dstprofile, renderingIntent);
            } else {
                return KoColorSpaceAbstract<_CSTraits>::createColorConverter(dstColorSpace, renderingIntent);
            }
        }

        virtual bool convertPixelsTo(const quint8 * src,
                quint8 * dst,
                const KoColorSpace * dstColorSpace,
                quint32 numPixels,
                KoColorConversionTransformation::Intent renderingIntent) const
        {
            if (dstColorSpace->id() == this->id()
                && dstColorSpace->profile() == profile())
            {
                if (src!= dst)
                    memcpy (dst, src, numPixels * this->pixelSize());

                return true;
            }

            KoColorConversionTransformation* tf = 0;

            if (d->lastUsedTransform != 0 && d->lastUsedDstColorSpace != 0) {
                if (dstColorSpace->id() == d->lastUsedDstColorSpace->id() &&
                    dstColorSpace->profile() == d->lastUsedDstColorSpace->profile()) {
                    tf = d->lastUsedTransform;
                    }
            }

            if (!tf) {

                if (!d->transforms.contains(dstColorSpace)) {
            // XXX: Should we clear the transform cache if it gets too big?
                    tf = this->createColorConverter(dstColorSpace, renderingIntent);
                    d->transforms[dstColorSpace] = tf;
                }
                else {
                    tf = d->transforms[dstColorSpace];
                }

                d->lastUsedTransform = tf;
                d->lastUsedDstColorSpace = dstColorSpace;
            }
            tf->transform(src, dst, numPixels);

            return true;
        }

        virtual KoColorTransformation *createBrightnessContrastAdjustment(const quint16 *transferValues) const
        {
            if (!d->profile) return 0;

            LPGAMMATABLE transferFunctions[3];
            transferFunctions[0] = cmsBuildGamma(256, 1.0);
            transferFunctions[1] = cmsBuildGamma(256, 1.0);
            transferFunctions[2] = cmsBuildGamma(256, 1.0);

            for(int i =0; i < 256; i++)
                transferFunctions[0]->GammaTable[i] = transferValues[i];

            KoLcmsColorTransformation *adj = new KoLcmsColorTransformation;
            adj->profiles[1] = cmsCreateLinearizationDeviceLink(icSigLabData, transferFunctions);
            cmsSetDeviceClass(adj->profiles[1], icSigAbstractClass);

            adj->profiles[0] = d->profile->lcmsProfile();
            adj->profiles[2] = d->profile->lcmsProfile();
            adj->cmstransform  = cmsCreateMultiprofileTransform(adj->profiles, 3, this->colorSpaceType(), this->colorSpaceType(), INTENT_PERCEPTUAL, 0);
            adj->csProfile = d->profile->lcmsProfile();
            return adj;
        }


        virtual KoColorTransformation *createDesaturateAdjustment() const
        {
            if (!d->profile) return 0;

            KoLcmsColorTransformation *adj = new KoLcmsColorTransformation;

            adj->profiles[0] = d->profile->lcmsProfile();
            adj->profiles[2] = d->profile->lcmsProfile();
            adj->csProfile = d->profile->lcmsProfile();

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

        virtual KoColorTransformation *createPerChannelAdjustment(const quint16 * const*transferValues) const
        {
            if (!d->profile) return 0;

            LPGAMMATABLE *transferFunctions = new LPGAMMATABLE[ _CSTraits::channels_nb +1];

            for(uint ch=0; ch < this->colorChannelCount(); ch++) {
                transferFunctions[ch] = cmsBuildGamma(256, 1.0);
                for(uint i =0; i < 256; i++) {
                    transferFunctions[ch]->GammaTable[i] = transferValues[ch][i];
                }
            }

            KoLcmsColorTransformation *adj = new KoLcmsColorTransformation;
            adj->profiles[0] = cmsCreateLinearizationDeviceLink(this->colorSpaceSignature(), transferFunctions);
            adj->profiles[1] = NULL;
            adj->profiles[2] = NULL;
            adj->csProfile = d->profile->lcmsProfile();
            adj->cmstransform  = cmsCreateTransform(adj->profiles[0], this->colorSpaceType(), NULL, this->colorSpaceType(), INTENT_PERCEPTUAL, 0);

            delete [] transferFunctions;

            return adj;
        }

        virtual KoColorTransformation *createDarkenAdjustement(qint32 shade, bool compensate, double compensation) const
        {
            return new KoLcmsDarkenTransformation(this, d->defaultToLab, d->defaultFromLab, shade, compensate, compensation);
        }

        virtual quint8 difference(const quint8* src1, const quint8* src2) const
        {
            if (d->defaultToLab) {

                quint8 lab1[8], lab2[8];
                cmsCIELab labF1, labF2;

                if (this->alpha(src1) == OPACITY_TRANSPARENT || this->alpha(src2) == OPACITY_TRANSPARENT)
                    return (this->alpha(src1) == this->alpha(src2) ? 0 : 255);

                cmsDoTransform( d->defaultToLab, const_cast<quint8*>( src1 ), lab1, 1);
                cmsDoTransform( d->defaultToLab, const_cast<quint8*>( src2 ), lab2, 1);
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

    private:
        inline KoLcmsColorProfile* lcmsProfile() const {
            return d->profile;
        }
        inline static KoLcmsColorProfile* toLcmsProfile(KoColorProfile* p)
        {
            KoLcmsColorProfile* lp = dynamic_cast<KoLcmsColorProfile*>(p);
            if(lp) return lp;
            KoIccColorProfile* iccp = dynamic_cast<KoIccColorProfile*>(p);
            if(iccp) return new KoLcmsColorProfile(iccp->rawData());
            return 0;
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

            return true;
        }
        Private * const d;
};

class PIGMENT_EXPORT KoLcmsColorSpaceFactory : public KoColorSpaceFactory, private KoLcmsInfo {
    public:
        KoLcmsColorSpaceFactory(DWORD cmType, icColorSpaceSignature colorSpaceSignature) : KoLcmsInfo(cmType, colorSpaceSignature)
        {
        }
        virtual bool profileIsCompatible(KoColorProfile* profile) const
        {
            KoLcmsColorProfile* p = dynamic_cast<KoLcmsColorProfile*>(profile);
            return p && p->colorSpaceSignature() == colorSpaceSignature();
        }
        virtual bool isIcc() const { return true; }
        virtual bool isHdr() const { return false; }
        virtual QList<KoColorConversionTransformationFactory*> colorConversionLinks() const;
};

#endif
