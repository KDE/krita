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

#ifndef CHANNEL_CONVERTER_H_
#define CHANNEL_CONVERTER_H_

class ChannelConverter {

    public:
        ChannelConverter() {}
        ChannelConverter(float whiteS, float blackK);
        ~ChannelConverter();

        void KSToReflectance(float K, float S, float &R) const;
        void reflectanceToKS(float R, float &K, float &S) const;
        void RGBTosRGB(float C, float &sC) const;
        void sRGBToRGB(float sC, float &C) const;

    private:
        float Kb, Sb;
        float w0, w1; // For whitening
        float b0, b1, b2; // For blackening and making K and S continuous in 0.5

        inline float PHI(float R) const { return (2.0*R)/((1.0-R)*(1.0-R)); }
        inline float PSI(float R) const { return ((1.0-R)*(1.0-R))/(2.0*R); }

        inline float W(float R) const { return w1*R + w0; }
        inline float B(float R) const { return b2*R*R + b1*R + b0; }

        inline float K(float R) const;
        inline float S(float R) const;
};

inline float ChannelConverter::K(float R) const
{
    if (R <= 0.5)
        return ( 1.0 / (PHI(W(R))-PHI(R)) );
    else
        return ( S(R)*PSI(R) );
}

inline float ChannelConverter::S(float R) const
{
    if (R > 0.5)
        return ( Kb - Sb*PSI(B(R)) ) / ( PSI(B(R)) - PSI(R) );
    else
        return ( K(R)*PHI(R) );
}

#endif // CHANNEL_CONVERTER_H_
