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

ChannelConverter::ChannelConverter(float whiteS, float blackK)
: Sw(whiteS), Kb(blackK)
{
    float Qw0, Qb1, alpha1, alpha2;

    Qw0 = Kb / Sw;
    Qb1 = Kb / Sw;

    w0 = 1.0 + Qw0 - sqrt( Qw0*Qw0 + 2.0*Qw0 );
    wi = 1.0 - w0;
    b1 = 1.0 + Qb1 - sqrt( Qb1*Qb1 + 2.0*Qb1 );

    alpha1 = Sw / ( PHI(W(0.5)) - PHI(0.5) );
    alpha2 = (Kb*PSI(0.5)) / (PSI(B(0.5))-PSI(0.5));

    Ke = alpha1/ alpha2;

//     b1 = 5.25797;
//     Ke = 0.17241;
//     w0 = 0.045549;
//     wi = 1.0 - w0;
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
        K = Sw / (PHI(W(R))-PHI(R));
        S = K * PHI(R);
    }
    if ( R > 0.5 ) {
        S = Ke*Kb / (PSI(B(R))-PSI(R));
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
