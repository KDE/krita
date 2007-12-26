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

#include <QDebug>

ChannelConverter::ChannelConverter(float Kblack, float Sblack)
: Kb(Kblack), Sb(Sblack)
{
    float A, q1, q2, k1, k2;
    w0 = 1.0 + Kb - sqrt( Kb*Kb + 2.0*Kb );
    A = 1.0 / ( PHI(W(0.5)) - PHI(0.5) );

    q1 = 0.25 * ( 1.0 + Kb/A ) / ( 1.0 + 0.25*Sb/A );
    q2 = Kb / ( 1.0 + Sb );

    k1 = (1.0 + q1 - sqrt( q1*q1 + 2.0*q1 )) * 2.0;
    k2 = (1.0 + q2 - sqrt( q2*q2 + 2.0*q2 ));

    b1 = 2.0*k1 - 1.0*k2;
    b2 = 1.0*k2 - 1.0*k1;
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
    if ( R <= 0.5 ) {
        K = 1.0 / (PHI(W(R))-PHI(R));
        S = K * PHI(R);
    }
    if ( R > 0.5 ) {
        S = ( Kb - PSI(B(R))*Sb ) / ( PSI(B(R)) - PSI(R) );
        K = S * PSI(R);
    }
}

void ChannelConverter::RGBTosRGB(float C, float &sC) const
{
    if (C <= 0.0031308)
        sC = 12.92 * C;
    else
        sC = 1.055 * pow( C, 1.0/2.4 ) - 0.055;

//     sC = C;
}

void ChannelConverter::sRGBToRGB(float sC, float &C) const
{
    if (sC <= 0.04045)
        C = sC / 12.92;
    else
        C = pow( ( sC + 0.055 ) / 1.055, 2.4 );

//     C = sC;
}
