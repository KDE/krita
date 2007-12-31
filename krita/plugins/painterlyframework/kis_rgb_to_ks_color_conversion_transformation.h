/*
 *  Copyright (c) 2007 Emanuele Tamponi <emanuele@valinor.it>
 *
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

#ifndef KIS_RGB_TO_KS_COLOR_CONVERSION_TRANSFORMATION_H_
#define KIS_RGB_TO_KS_COLOR_CONVERSION_TRANSFORMATION_H_

#include <KoColorConversionTransformation.h>

#include "channel_converter.h"
#include "kis_illuminant_profile.h"
#include "kis_ks_colorspace_traits.h"


#include <QHash>
#include <QString>

#include <gsl/gsl_vector.h>

class RGBID {
    public:
        RGBID() {}
        ~RGBID() {}
        void set(const quint16 *rgb)
        {
            memmove(RGB, rgb, 8);
        }
        int total()
        {
            return RGB[0]+RGB[1]+RGB[2];
        }
        bool operator==(const RGBID &op2) const
        {
            return ( op2.RGB[0] == RGB[0] &&
                     op2.RGB[1] == RGB[1] &&
                     op2.RGB[2] == RGB[2] );
        }

        friend uint qHash(const RGBID &key);

    private:

        quint16 RGB[3];
};

inline uint qHash(const RGBID &key)
{
    return qHash(key.RGB[0]^key.RGB[1]^key.RGB[2]);
}

template< int _N_ >
class KisRGBToKSColorConversionTransformation : public KoColorConversionTransformation {
typedef KoColorConversionTransformation parent;

public:
    KisRGBToKSColorConversionTransformation(const KoColorSpace *srcCs, const KoColorSpace *dstCs)
    : parent(srcCs, dstCs), m_rgbvec(0), m_refvec(0), m_converter(0), m_profile(0)
    {
        m_profile = static_cast<const KisIlluminantProfile*>(dstCs->profile());
        m_converter = new ChannelConverter(m_profile->Kblack(), m_profile->Sblack());
        m_rgbvec = gsl_vector_calloc(3);
        m_refvec = gsl_vector_calloc(_N_);
        m_cache.reserve(20000);
    }

    ~KisRGBToKSColorConversionTransformation()
    {
        delete m_converter;
        gsl_vector_free(m_rgbvec);
        gsl_vector_free(m_refvec);

        // We need to manually delete the items
        QHashIterator<RGBID, float *> i(m_cache);
        while (i.hasNext()) {
            i.next();
            delete [] i.value();
        }
    }

    void transform(const quint8 *src8, quint8 *dst, qint32 nPixels) const
    {
        // For each pixel we do this:
        // 1 - convert raw bytes to quint16
        // 2 - find reflectances using the transformation matrix of the profile.
        // 3 - convert reflectances to K/S

        const quint16 *src = reinterpret_cast<const quint16*>(src8);
        const int pixelSize = KisKSColorSpaceTrait<_N_>::pixelSize;

        for ( ; nPixels > 0; nPixels-- ) {
            // Load for the lookup
            m_lookup.set(src);
            // Check if the item is in the cache
            if (m_cache.contains(m_lookup)) {

                memmove(dst, m_cache.value(m_lookup), pixelSize-4);

            } else {

                // Do normal computations
                for (int i = 0; i < 3; i++) {
                    m_converter->sRGBToRGB(KoColorSpaceMaths<quint16,float>::scaleToA(src[2-i]), m_current);
                    gsl_vector_set(m_rgbvec, i, m_current);
                }

                // Avoid calculations of black and white
                if (m_lookup.total() == 0)
                    gsl_vector_set_all(m_refvec, 0.0);
                else if (m_lookup.total() == 3*65535)
                    gsl_vector_set_all(m_refvec, 1.0);
                else
                    RGBToReflectance();

                for (int i = 0; i < _N_; i++) {
                    m_converter->reflectanceToKS((float)gsl_vector_get(m_refvec, i),
                                                KisKSColorSpaceTrait<_N_>::K(dst, i),
                                                KisKSColorSpaceTrait<_N_>::S(dst, i));
                }

                // Add the new color conversion to the cache
                m_cache.insert(m_lookup, new float[pixelSize-4]);
                memmove(m_cache.value(m_lookup), dst, pixelSize-4);
            }

            KisKSColorSpaceTrait<_N_>::setAlpha(dst, KoColorSpaceMaths<quint16,quint8>::scaleToA(src[3]), 1);

            src += 4;
            dst += pixelSize;
        }

        // If the cache contains too many elements, erase the first 15000
        if (m_cache.count() >= 19000) {
            QHashIterator<RGBID, float *> i(m_cache);
            for (int j = 0; j < 15000; j++) {
                i.next();
                delete [] i.value();
                m_cache.remove(i.key());
            }
            m_cache.squeeze();
        }

    }

protected:
    virtual void RGBToReflectance() const = 0;

protected:
    gsl_vector *m_rgbvec;
    gsl_vector *m_refvec;

    ChannelConverter *m_converter;
    const KisIlluminantProfile *m_profile;

    mutable QHash<RGBID, float *> m_cache;
    mutable float m_current;
    mutable RGBID m_lookup;
};

#endif // KIS_RGB_TO_KS_COLOR_CONVERSION_TRANSFORMATION_H_
