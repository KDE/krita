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

#ifndef _KIS_QUANTUM_OPERATIONS_H_
#define _KIS_QUANTUM_OPERATIONS_H_

#include "kis_global.h"

/** This class is meant to be used inside KisQuantum
	*/
class KisQuantumOperation {
	public:
		KisQuantumOperation( ) { };
		virtual QUANTUM operation(QUANTUM) =0;
		/** This function is called to clone this object.
		 * @param index this is the offset
		 */
		virtual KisQuantumOperation* clone(int index) =0;
};

class KisQuantumOperationLinear : public KisQuantumOperation {
	public:
		KisQuantumOperationLinear( ) { };
		virtual QUANTUM operation(QUANTUM);
		virtual KisQuantumOperation* clone(int index);
};

class KisQuantumOperationMasked : public KisQuantumOperation {
	public:
		KisQuantumOperationMasked( ) { };
		virtual QUANTUM operation(QUANTUM);
		virtual KisQuantumOperation* clone(int index);
};

#endif
