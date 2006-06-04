/*
 *  This file is part of the KDE project
 *
 *  Copyright (c) 2005 Cyrille Berger <cberger@cberger.net>
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

#ifndef KIS_MATH_TOOLBOX_H
#define KIS_MATH_TOOLBOX_H

#include <QObject>

// typedef unsigned int uint;

#include <KoGenericRegistry.h>
#include "kis_paint_device.h"
#include "kis_types.h"

#include <new>

class KRITAIMAGE_EXPORT KisMathToolbox : public QObject {
    Q_OBJECT
    public:
        struct KisFloatRepresentation {
            KisFloatRepresentation(uint nsize, uint ndepth) throw(std::bad_alloc ) : coeffs(new float[nsize*nsize*ndepth]) ,size(nsize), depth(ndepth)
            {
                // XXX: Valgrind shows that these are being used without being initialised.
                for (quint32 i = 0; i < nsize*nsize*ndepth; ++i) {
                    coeffs[i] = 0;
                }
            }
            ~KisFloatRepresentation() { if(coeffs) delete[] coeffs; }
            float* coeffs;
            uint size;
            uint depth;
        };
        typedef KisFloatRepresentation KisWavelet;
    public:
        KisMathToolbox(KoID id);
        ~KisMathToolbox();
    public:
        inline KoID id() { return m_id; };
        /**
         * This function initialize a wavelet structure
         * @param lay the layer that will be used for the transformation
         */
        inline KisWavelet* initWavelet(KisPaintDeviceSP lay, const QRect&) throw(std::bad_alloc );
        inline uint fastWaveletTotalSteps(const QRect&);
        /**
         * This function reconstruct the layer from the information of a wavelet
         * @param src layer from which the wavelet will be computed
         * @param buff if set to 0, the buffer will be initialized by the function,
         * you might want to give a buff to the function if you want to use the same buffer
         * in transformToWavelet and in untransformToWavelet, use initWavelet to initialize
         * the buffer
         */
         virtual KisWavelet* fastWaveletTransformation(KisPaintDeviceSP src, const QRect&, KisWavelet* buff = 0) =0;
        /**
         * This function reconstruct the layer from the information of a wavelet
         * @param dst layer on which the wavelet will be untransform
         * @param wav the wavelet
         * @param buff if set to 0, the buffer will be initialized by the function,
         * you might want to give a buff to the function if you want to use the same buffer
         * in transformToWavelet and in untransformToWavelet, use initWavelet to initialize
         * the buffer
         */
         virtual void fastWaveletUntransformation(KisPaintDeviceSP dst, const QRect&, KisWavelet* wav, KisWavelet* buff = 0) =0;
    signals:
        void nextStep();
    protected:
        /**
         * This function transform a paint device into a KisFloatRepresentation, this function is colorspace independant,
         * for Wavelet, Pyramid and FFT the data is allways the exact value of the channel stored in a float.
         */
        void transformToFR(KisPaintDeviceSP src, KisFloatRepresentation*, const QRect&);
        /**
         * This function transform a KisFloatRepresentation into a paint device, this function is colorspace independant,
         * for Wavelet, Pyramid and FFT the data is allways the exact value of the channel stored in a float.
         */
        void transformFromFR(KisPaintDeviceSP dst, KisFloatRepresentation*, const QRect&);
    private:
        KoID m_id;
};

class KRITAIMAGE_EXPORT KisMathToolboxFactoryRegistry : public KisGenericRegistry<KisMathToolbox*> {
    public:
        KisMathToolboxFactoryRegistry();
        ~KisMathToolboxFactoryRegistry();
};


inline KisMathToolbox::KisWavelet* KisMathToolbox::initWavelet(KisPaintDeviceSP src, const QRect& rect) throw(std::bad_alloc )
{
    int size;
    int maxrectsize = (rect.height() < rect.width()) ? rect.width() : rect.height();
    for(size = 2; size < maxrectsize; size *= 2) ;
    qint32 depth = src->colorSpace()->nColorChannels();
    return new KisWavelet(size, depth);
}

inline uint KisMathToolbox::fastWaveletTotalSteps(const QRect& rect)
{
    int size, steps;
    int maxrectsize = (rect.height() < rect.width()) ? rect.width() : rect.height();
    steps = 0;
    for(size = 2; size < maxrectsize; size *= 2) steps += size / 2; ;
    return steps;
}

#endif
