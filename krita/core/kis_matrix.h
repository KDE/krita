/* This file is part of the KDE project
   Copyright (c) 2004 Cyrille Berger <cberger@cberger.net>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#ifndef _KIS_MATRIX_H_
#define _KIS_MATRIX_H_

#include <string.h>

/**
 * This template is a generic way to store an array associated with a width and a height
 */
template<typename _Tp, int _W, int _H>
class KisMatrix
{
	typedef KisMatrix<_Tp, _W, _H> KisMatrixT;
	public:
		KisMatrix() {};
		KisMatrix( _Tp values[_H][_W], _Tp factor, _Tp offset) : m_factor(factor), m_offset(offset)
		{
			for(int i = 0; i < _H; i++)
			{
				memcpy(m_values[i], values[i], sizeof(_Tp) * _W);
			}
			computeSum();
		};
	public:
		/** This function is used only for debugging
			*/
		void dump()
		{
			kdDebug() << "KisMatrixT::dump()" << endl;
			for(int i = 0; i < _H; i++)
			{
				for(int j = 0; j < _W; j++)
				{
					kdDebug() << "m_values[" << i << "][" << j << "]=" << m_values[i][j] << endl;
				}
			}
		};
	public:
		/** This operator return a row.
			*/
		inline _Tp* operator[](int i) { return m_values[i]; };
	public:
		void computeSum()
		{
			m_sum = 0;
			for(int i = 0; i < _H; i++)
			{
				for(int j = 0; j < _W; j++)
				{
					m_sum += m_values[i][j];
				}
			}
		};
		inline _Tp sum() { return m_sum; }
		inline void setFactor(_Tp factor) { m_factor = factor; };
		inline _Tp factor() { return m_factor; };
		inline void setOffset(_Tp offset) { m_offset = offset; };
		inline _Tp offset() { return m_offset; };
	private:
		_Tp m_values[_H][_W];
		_Tp m_factor;
		_Tp m_offset;
		_Tp m_sum;
};

typedef KisMatrix<int, 3, 3> KisMatrix3x3;

#endif
