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

#ifndef ADAPTATION_MATRICES_H_
#define ADAPTATION_MATRICES_H_

namespace AM {

	const double XYZ[3][3] = {
		{ 1.0, 0.0, 0.0 },
  		{ 0.0, 1.0, 0.0 },
		{ 0.0, 0.0, 1.0 }
	};

	const double XYZ_1[3][3] = {
		{ 1.0, 0.0, 0.0 },
  		{ 0.0, 1.0, 0.0 },
		{ 0.0, 0.0, 1.0 }
	};

	const double Bradford[3][3] = {
		{ 0.8951, -0.7502,  0.0389 },
  		{ 0.2664,  1.7135, -0.0685 },
		{-0.1614,  0.0367,  1.0296 }
	};

	const double Bradford_1[3][3] = {
		{ 0.986993,  0.432305, -0.008529 },
		{-0.147054,  0.518360,  0.040043 },
		{ 0.159963,  0.049291,  0.968487 }
	};

	const double VonKries[3][3] = {
		{ 0.40024, -0.22630,  0.00000 },
		{ 0.70760,  1.16532,  0.00000 },
		{-0.08081,  0.04570,  0.91822 }
	};

	const double VonKries_1[3][3] = {
		{ 1.859936, 0.361191, 0.000000 },
		{-1.129382, 0.638812, 0.000000 },
		{ 0.219897,-0.000006, 1.089064 }
	};

};

#endif // ADAPTATION_MATRICES_H_
