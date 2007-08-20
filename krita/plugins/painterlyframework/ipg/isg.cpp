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

#include <fstream>
#include <iostream>

#include "sn_distributions.h"

using namespace std;

int main (int argc, char *argv[])
{
	if (argc != 3) {
usage:
		cout << "Usage: " << argv[0] << " temperature output_file" << endl;
		return 255;
	}

	double T = atof(argv[1]);

	if (T <= 0.0)
		goto usage;

	double x_d, y_d;

	x_d = ( -4.6070e9 / (T*T*T) ) + ( 2.9678e6 / (T*T) ) + ( 0.09911e3 / T ) + 0.244063;
	y_d = -3.0*x_d*x_d + 2.870*x_d - 0.275;

	double M, M1, M2;

	M = 0.0241 + 0.2562*x_d - 0.7341*y_d;
	M1 = (-1.3515 - 1.7703*x_d + 5.9114*y_d) / M;
	M2 = ( 0.0300 - 31.4424*x_d + 30.0717*y_d) / M;

	double S[471];
	ofstream f(argv[2]);

	f.precision(64);
	for (int i = 0; i <= 470; i++) {
		S[i] = sn::s0[i] + M1*sn::s1[i] + M2*sn::s2[i];
		f << S[i] << endl;
	}

	return 0;
}
