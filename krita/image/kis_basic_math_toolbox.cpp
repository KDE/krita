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

#include "kis_basic_math_toolbox.h"

KisBasicMathToolbox::KisBasicMathToolbox()
    : KisMathToolbox(KoID("Basic"))
{
}


KisBasicMathToolbox::~KisBasicMathToolbox()
{
}


void KisBasicMathToolbox::wavetrans(KisMathToolbox::KisWavelet* wav, KisMathToolbox::KisWavelet* buff, uint halfsize)
{
    uint l = (2*halfsize)*wav->depth*sizeof(float);
    for(uint i = 0; i < halfsize; i++)
    {
        float * itLL = buff->coeffs + i*buff->size*buff->depth;
        float * itHL = buff->coeffs + (i*buff->size + halfsize)*buff->depth;
        float * itLH = buff->coeffs + (halfsize+i)*buff->size*buff->depth;
        float * itHH = buff->coeffs + ( (halfsize+i)*buff->size + halfsize)*buff->depth;
        float * itS11 = wav->coeffs + 2*i*wav->size*wav->depth;
        float * itS12 = wav->coeffs + (2*i*wav->size+1)*wav->depth;
        float * itS21 = wav->coeffs + (2*i+1)*wav->size*wav->depth;
        float * itS22 = wav->coeffs + ((2*i+1)*wav->size+1)*wav->depth;
        for(uint j = 0; j < halfsize; j++)
        {
            for( uint k = 0; k < wav->depth; k++)
            {
                *(itLL++) = (*itS11 + *itS12 + *itS21 + *itS22) * M_SQRT1_2;
                *(itHL++) = (*itS11 - *itS12 + *itS21 - *itS22) * M_SQRT1_2;
                *(itLH++) = (*itS11 + *itS12 - *itS21 - *itS22) * M_SQRT1_2;
                *(itHH++) = (*(itS11++) - *(itS12++) - *(itS21++) + *(itS22++)) * M_SQRT1_2;
            }
            itS11 += wav->depth; itS12 += wav->depth;
            itS21 += wav->depth; itS22 += wav->depth;
        }
        emit nextStep();
    }
    for(uint i = 0; i < halfsize; i++)
    {
        uint p = i*wav->size*wav->depth;
        memcpy(wav->coeffs + p, buff->coeffs + p, l);
        p = (i + halfsize )*wav->size*wav->depth;
        memcpy(wav->coeffs + p, buff->coeffs + p, l);
    }
    if(halfsize != 1)
    {
        wavetrans(wav, buff, halfsize/2);
    }
}

void KisBasicMathToolbox::waveuntrans(KisMathToolbox::KisWavelet* wav, KisMathToolbox::KisWavelet* buff, uint halfsize)
{
    uint l = (2*halfsize)*wav->depth*sizeof(float);
    for(uint i = 0; i < halfsize; i++)
    {
        float * itLL = wav->coeffs + i*buff->size*buff->depth;
        float * itHL = wav->coeffs + (i*buff->size + halfsize)*buff->depth;
        float * itLH = wav->coeffs + (halfsize+i)*buff->size*buff->depth;
        float * itHH = wav->coeffs + ( (halfsize+i)*buff->size + halfsize)*buff->depth;
        float * itS11 = buff->coeffs + 2*i*wav->size*wav->depth;
        float * itS12 = buff->coeffs + (2*i*wav->size+1)*wav->depth;
        float * itS21 = buff->coeffs + (2*i+1)*wav->size*wav->depth;
        float * itS22 = buff->coeffs + ((2*i+1)*wav->size+1)*wav->depth;
        for(uint j = 0; j < halfsize; j++)
        {
            for( uint k = 0; k < wav->depth; k++)
            {
                *(itS11++) = (*itLL + *itHL + *itLH + *itHH)*0.25*M_SQRT2;
                *(itS12++) = (*itLL - *itHL + *itLH - *itHH)*0.25*M_SQRT2;
                *(itS21++) = (*itLL + *itHL - *itLH - *itHH)*0.25*M_SQRT2;
                *(itS22++) = (*(itLL++) - *(itHL++) - *(itLH++) + *(itHH++))*0.25*M_SQRT2;
            }
            itS11 += wav->depth; itS12 += wav->depth;
            itS21 += wav->depth; itS22 += wav->depth;
        }
        emit nextStep();
    }
    for(uint i = 0; i < halfsize; i++)
    {
        uint p = i*wav->size*wav->depth;
        memcpy(wav->coeffs + p, buff->coeffs + p, l);
        p = (i + halfsize )*wav->size*wav->depth;
        memcpy(wav->coeffs + p, buff->coeffs + p, l);
    }
    
    if(halfsize != wav->size/2)
    {
        waveuntrans(wav, buff, halfsize*2);
    }
}

KisMathToolbox::KisWavelet* KisBasicMathToolbox::fastWaveletTransformation(KisPaintDeviceSP src, const QRect& rect,  KisWavelet* buff)
{
    if(buff == 0)
    {
        buff = initWavelet( src, rect );
    }
    KisWavelet* wav = initWavelet( src, rect );
    transformToFR(src, wav, rect);
    wavetrans(wav, buff, wav->size / 2);
    
    return wav;
}

void KisBasicMathToolbox::fastWaveletUntransformation(KisPaintDeviceSP dst, const QRect& rect, KisWavelet* wav, KisWavelet* buff)
{
    if(buff == 0)
    {
        buff = initWavelet( dst, rect );
    }
    
    waveuntrans(wav, buff, 1 );
    transformFromFR(dst, wav, rect);
}
