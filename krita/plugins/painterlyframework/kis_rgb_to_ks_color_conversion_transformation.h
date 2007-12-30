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

#include <QByteArray>
#include <QHash>
#include <QString>

#include <gsl/gsl_vector.h>

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
        m_cache.reserve(10000);
    }

    ~KisRGBToKSColorConversionTransformation()
    {
        delete m_converter;
        gsl_vector_free(m_rgbvec);
        gsl_vector_free(m_refvec);
        QHashIterator<QByteArray, float *> i(m_cache);
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
            m_lookup = QByteArray::number(src[0])+QByteArray("-")+QByteArray::number(src[1])+QByteArray("-")+QByteArray::number(src[2]);
            if (m_cache.contains(m_lookup)) {
                memmove(dst, m_cache.value(m_lookup), pixelSize-4);
            } else {
                for (int i = 0; i < 3; i++) {
                    m_converter->sRGBToRGB(KoColorSpaceMaths<quint16,float>::scaleToA(src[2-i]), m_current);
                    gsl_vector_set(m_rgbvec, i, m_current);
                }
                if (src[0]+src[1]+src[2] == 0)
                    gsl_vector_set_all(m_refvec, 0.0);
                else if (src[0]+src[1]+src[2] == 3*65535)
                    gsl_vector_set_all(m_refvec, 1.0);
                else
                    RGBToReflectance();

                for (int i = 0; i < _N_; i++) {
                    m_converter->reflectanceToKS((float)gsl_vector_get(m_refvec, i),
                                                KisKSColorSpaceTrait<_N_>::K(dst, i),
                                                KisKSColorSpaceTrait<_N_>::S(dst, i));
                }
                if (m_cache.count() == 10000) {
                    QHashIterator<QByteArray, float *> i(m_cache);
                    for (int j = 0; j < 7500; j++) {
                        i.next();
                        delete [] i.value();
                        m_cache.remove(i.key());
                    }
                    m_cache.squeeze();
                }
                m_cache.insert(m_lookup, new float[pixelSize-4]);
                memmove(m_cache.value(m_lookup), dst, pixelSize-4);
            }

            KisKSColorSpaceTrait<_N_>::setAlpha(dst, KoColorSpaceMaths<quint16,quint8>::scaleToA(src[3]), 1);

            src += 4;
            dst += pixelSize;
        }
    }

protected:
    virtual void RGBToReflectance() const = 0;

protected:
    gsl_vector *m_rgbvec;
    gsl_vector *m_refvec;

    ChannelConverter *m_converter;
    const KisIlluminantProfile *m_profile;

    mutable QHash<QByteArray, float *> m_cache;
    mutable float m_current;
    mutable QByteArray m_lookup;
};

#endif // KIS_RGB_TO_KS_COLOR_CONVERSION_TRANSFORMATION_H_
