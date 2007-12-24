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

#include <cmath>

#include "channel_converter.h"

#define M_w 0.9f
#define M_b 0.9f

ChannelConverter::ChannelConverter(float whiteS, float blackK)
: S_w(whiteS), K_b(blackK)
{
    float K;

    K = K_b / ( M_w * S_w );
    m_blackening = 1.0 + K - sqrt( K*K + 2.0*K );

    K = M_b * K_b / S_w;
    m_basewhite = 1.0 + K - sqrt( K*K + 2.0*K );
    m_whitening = 1.0 - m_basewhite;
}

ChannelConverter::~ChannelConverter()
{
}

void ChannelConverter::KSToReflectance(float K, float S, float &R) const
{
    if (S == 0.0) {
        R = 0.0;
        return;
    }

    if (K == 0.0) {
        R = 1.0;
        return;
    }

    float Q = K/S;
    R = 1.0 + Q - sqrt( Q*Q + 2.0*Q );
}

void ChannelConverter::reflectanceToKS(float R, float &K, float &S) const
{
    if ( R == 0.0 ) {
        K = M_b * K_b;
        S = 0.0;
        return;
    }
    if ( R == 1.0 ) {
        K = 0.0;
        S = M_w * S_w;
        return;
    }
    if ( R <= 0.5 ) {
        float Rw = m_whitening * R + m_basewhite;
        float F  = ( 2.0 * R  ) / pow ( 1.0 - R , 2 );
        float Fw = ( 2.0 * Rw ) / pow ( 1.0 - Rw, 2 );
        K = 0.07 * S_w / ( Fw - F ); // TODO Remove that 0.07
        S = K * F;
    }
    if ( R > 0.5 ) {
        float Rb = m_blackening * R;
        float P  = pow ( 1.0 - R , 2 ) / ( 2.0 * R  );
        float Pb = pow ( 1.0 - Rb, 2 ) / ( 2.0 * Rb );
        S = K_b / ( Pb - P );
        K = S * P;
    }
}

void ChannelConverter::RGBTosRGB(float C, float &sC) const
{
    if (C <= 0.0031308)
        sC = 12.92 * C;
    else
        sC = 1.055 * pow( C, 1.0/2.4 ) - 0.055;
}

void ChannelConverter::sRGBToRGB(float sC, float &C) const
{
    if (sC <= 0.04045)
        C = sC / 12.92;
    else
        C = pow( ( sC + 0.055 ) / 1.055, 2.4 );
}
