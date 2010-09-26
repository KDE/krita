/*
 *  Copyright (c) 2005 Adrian Page <adrian@pagenet.plus.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#ifndef KO_INTEGER_MATHS_H
#define KO_INTEGER_MATHS_H

#define UINT8_MAX 255u
#define UINT8_MIN 0u

#define UINT16_MAX 65535u
#define UINT16_MIN 0u

#define UINT32_MAX (4294967295u)
#define UINT32_MIN 0u

#define INT16_MAX 32767
#define INT16_MIN -32768

// #define CLAMP(x,l,u) ((x)<(l)?(l):((x)>(u)?(u):(x)))

template<typename _T_, typename _T2_, typename _T3_>
inline _T_ CLAMP(_T_ x, _T2_ l, _T3_ u)
{
    if (x < l)
        return _T_(l);
    else if (x > u)
        return _T_(u);
    return x;
}

/// take a and scale it up by 256*b/255
inline uint UINT8_SCALEBY(uint a, uint b)
{
    uint c = a * b + 0x80u;
    return (c >> 8) + c;
}

/// multiplication of two scale values
/// A scale value is interpreted as 255 equaling 1.0 (such as seen in rgb8 triplets)
/// thus "255*255=255" because 1.0*1.0=1.0
inline uint UINT8_MULT(uint a, uint b)
{
    uint c = a * b + 0x80u;
    return ((c >> 8) + c) >> 8;
}

inline uint UINT8_DIVIDE(uint a, uint b)
{
    uint c = (a * UINT8_MAX + (b / 2u)) / b;
    return c;
}

/// Approximation of (a * b * c + 32512) / 65025.0
inline uint UINT8_MULT3(uint a, uint b, uint c)
{
  uint t = a * b * c + 0x7F5B;
  return ((t >> 7) + t) >> 16;
}

/// Blending of two scale values as described by the alpha scale value
/// A scale value is interpreted as 255 equaling 1.0 (such as seen in rgb8 triplets)
/// Basically we do: a*alpha + b*(1-alpha)
inline uint UINT8_BLEND(uint a, uint b, uint alpha)
{
    // However the formula is refactored to (a-b)*alpha + b  since that saves a multiplication
    // Signed arithmetic is needed since a-b might be negative
    int c = (int(a) - int(b)) * alpha + 0x80u;
    c = ((c >> 8) + c) >> 8;
    return c + b;
}

inline uint UINT16_MULT(uint a, uint b)
{
    uint c = a * b + 0x8000u;
    return ((c >> 16) + c) >> 16;
}

inline int INT16_MULT(int a, int b)
{
    return (a*b) / INT16_MAX;
}

inline uint UINT16_DIVIDE(uint a, uint b)
{
    uint c = (a * UINT16_MAX + (b / 2u)) / b;
    return c;
}

inline uint UINT16_BLEND(uint a, uint b, uint alpha)
{
    // Basically we do a*alpha + b*(1-alpha)
    // However refactored to (a-b)*alpha + b  since that saves a multiplication
    // Signed arithmetic is needed since a-b might be negative
    int c = ((int(a) - int(b)) * int(alpha)) >> 16;
    return uint(c + b);
}

inline uint UINT8_TO_UINT16(uint c)
{
    return c | (c << 8);
}

inline uint UINT16_TO_UINT8(uint c)
{
    //return round(c / 257.0);
    //For all UINT16 this calculation is the same and a lot faster (off by c/65656 which for every c is 0)
    c = c - (c >> 8) + 128;
    return c >> 8;
}

inline int INT16_BLEND(int a, int b, uint alpha)
{
    // Basically we do a*alpha + b*(1-alpha)
    // However refactored to (a-b)*alpha + b  since that saves a multiplication
    int c = ((int(a) - int(b)) * int(alpha)) >> 16;
    return c + b;
}

#endif

