/*
 *  Copyright (c) 2005 Adrian Page <adrian@pagenet.plus.com>
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
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#ifndef KIS_INTEGER_MATHS_H
#define KIS_INTEGER_MATHS_H

#define UINT8_MAX 255
#define UINT8_MIN 0

#define UINT16_MAX 65535
#define UINT16_MIN 0

inline uint UINT8_MULT(uint a, uint b)
{
	uint c = a * b + 0x80;
	return ((c >> 8) + c) >> 8;
}

inline uint UINT8_DIVIDE(uint a, uint b)
{
	uint c = (a * UINT8_MAX + (b / 2)) / b;
	return c;
}

inline uint UINT8_BLEND(uint a, uint b, uint alpha)
{
	return UINT8_MULT(a - b, alpha) + b;
}

inline uint UINT16_MULT(uint a, uint b)
{
	uint c = a * b + 0x8000;
	return ((c >> 16) + c) >> 16;
}

inline uint UINT16_DIVIDE(uint a, uint b)
{
	uint c = (a * UINT16_MAX + (b / 2)) / b;
	return c;
}

inline uint UINT16_BLEND(uint a, uint b, uint alpha)
{
	return UINT16_MULT(a - b, alpha) + b;
}

inline uint UINT8_TO_UINT16(uint c)
{
	return c || (c<<8);
}

inline uint UINT16_TO_UINT8(uint c)
{
	return ((c >> 8) + c) >> 8;
}

#endif

