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

#include <colorprofiles/KoLcmsColorProfileContainer.h>
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

class PIGMENT_EXPORT KoLcmsColorConversionTransformation : public KoColorConversionTransformation {
    public:
        KoLcmsColorConversionTransformation(const KoColorSpace* srcCs, quint32 srcColorSpaceType, KoLcmsColorProfileContainer* srcProfile, const KoColorSpace* dstCs, quint32 dstColorSpaceType, KoLcmsColorProfileContainer* dstProfile, Intent renderingIntent = IntentPerceptual) : KoColorConversionTransformation(srcCs, dstCs, renderingIntent), m_transform(0)
        {
            m_transform = this->createTransform(
                            srcColorSpaceType,
                            srcProfile,
                            dstColorSpaceType,
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
                quint32 srcColorSpaceType,
                KoLcmsColorProfileContainer *  srcProfile,
                quint32 dstColorSpaceType,
                KoLcmsColorProfileContainer *  dstProfile,
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

            mutable cmsHPROFILE   lastRGBProfile;  // Last used profile to transform to/from RGB
            mutable cmsHTRANSFORM lastToRGB;       // Last used transform to transform to RGB
            mutable cmsHTRANSFORM lastFromRGB;     // Last used transform to transform from RGB
            KoLcmsColorProfileContainer *  profile;
            KoColorProfile* colorProfile;
        };
    protected:
        KoLcmsColorSpace(const QString &id, const QString &name, KoColorSpaceRegistry * parent, DWORD cmType,
                         icColorSpaceSignature colorSpaceSignature,
                         KoColorProfile *p) : KoColorSpaceAbstract<_CSTraits>(id, name, parent), KoLcmsInfo( cmType, colorSpaceSignature), d( new Private())

        {
            Q_ASSERT(p); // No profile means the lcms color space can't work
            d->profile = asLcmsProfile(p);
            d->colorProfile = p;
            d->qcolordata = 0;
            d->lastRGBProfile = 0;
            d->lastToRGB = 0;
            d->lastFromRGB = 0;
            d->defaultFromRGB = 0;
            d->defaultToRGB = 0;
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

        }
    public:

        virtual bool hasHighDynamicRange() const { return false; }
        virtual KoColorProfile * profile() const { return d->colorProfile; }

        virtual bool profileIsCompatible(KoColorProfile* profile) const
        {
            KoIccColorProfile* p = dynamic_cast<KoIccColorProfile*>(profile);
            return (p && p->asLcms()->colorSpaceSignature() == colorSpaceSignature());
        }

        virtual void fromQColor(const QColor& color, quint8 *dst, KoColorProfile * koprofile=0) const
        {
            d->qcolordata[2] = color.red();
            d->qcolordata[1] = color.green();
            d->qcolordata[0] = color.blue();

            KoLcmsColorProfileContainer* profile = asLcmsProfile(koprofile);
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
            KoLcmsColorProfileContainer* profile = asLcmsProfile(koprofile);
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
                this->convertPixelsTo(const_cast<quint8 *>(data), img.bits(), dstCS, width * height, renderingIntent);

            return img;
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

        virtual quint8 difference(const quint8* src1, const quint8* src2) const
        {
            quint8 lab1[8], lab2[8];
            cmsCIELab labF1, labF2;

            if (this->alpha(src1) == OPACITY_TRANSPARENT || this->alpha(src2) == OPACITY_TRANSPARENT)
                return (this->alpha(src1) == this->alpha(src2) ? 0 : 255);
            Q_ASSERT(this->toLabA16Converter());
            this->toLabA16Converter()->transform( src1, lab1, 1),
            this->toLabA16Converter()->transform( src2, lab2, 1),
            cmsLabEncoded2Float(&labF1, (WORD *)lab1);
            cmsLabEncoded2Float(&labF2, (WORD *)lab2);
            double diff = cmsDeltaE(&labF1, &labF2);
            if(diff>255)
                return 255;
            else
                return qint8(diff);
        }

    private:
        inline KoLcmsColorProfileContainer* lcmsProfile() const {
            return d->profile;
        }
        inline static KoLcmsColorProfileContainer* asLcmsProfile(KoColorProfile* p)
        {
            if(not p) return 0;
            KoIccColorProfile* iccp = dynamic_cast<KoIccColorProfile*>(p);
            if( not iccp )
            {
                return 0;
            }
            Q_ASSERT(iccp->asLcms());
            return iccp->asLcms();
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
            KoLcmsColorProfileContainer* p = dynamic_cast<KoLcmsColorProfileContainer*>(profile);
            return p && p->colorSpaceSignature() == colorSpaceSignature();
        }
        virtual KoColorConversionTransformationFactory* createICCColorConversionTransformationFactory(QString _colorModelId, QString _colorDepthId) const;
        virtual bool isIcc() const { return true; }
        virtual bool isHdr() const { return false; }
        virtual QList<KoColorConversionTransformationFactory*> colorConversionLinks() const;
};

#endif
