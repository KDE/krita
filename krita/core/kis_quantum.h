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
#include "kis_quantum_operations.h"


/**
 * An editable wrapper around the byte values for a single colour channel.
 */
class KisQuantum {
	public:
		inline KisQuantum(QUANTUM* q, KisQuantumOperation* op = new KisQuantumOperationLinear()) : m_quantum(q), m_op(op) { };
	public:
		inline operator QUANTUM() const { return *m_quantum; };

		inline QUANTUM operator=(QUANTUM q)
		{
			return *m_quantum = m_op->operation(q);
		};

		inline QUANTUM operator-=(QUANTUM q)
		{
			return (*this = *this - q);
		}

		inline QUANTUM operator+=(QUANTUM q)
		{
			return (*this = *this + q);
		}

		/** 
		 * This operator allow to acces to a neighbour quantum, it's mean to be used
		 * inside a pixel
		 */
		KisQuantum operator[](int index) const {
			return KisQuantum( m_quantum + index );
		}
	private:
		QUANTUM* m_quantum;

		KisQuantumOperation* m_op;
};


#endif
