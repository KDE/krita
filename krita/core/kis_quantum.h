/*
 *  Copyright (c) 2004 Cyrille Berger <cberger@cberger.net>
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

#ifndef KIS_QUANTUM_H_
#define KIS_QUANTUM_H_

#include "kis_global.h"


class KisQuantum {
	public:
		inline KisQuantum(QUANTUM* q) : m_quantum(q) { };
	public:
		inline operator QUANTUM() { return *m_quantum; };
		inline QUANTUM operator=(QUANTUM q)
		{
			// TODO: the stuff concerning the selection should be put theres
			return ( *m_quantum = q );
		};
		inline QUANTUM operator-=(QUANTUM q)
		{
			return ((*this) = *this - q);
		}
		inline QUANTUM operator+=(QUANTUM q)
		{
			return ((*this) = *this + q);
		}
	private:
		QUANTUM* m_quantum;
};

#endif
