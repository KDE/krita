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

 /*
	Illuminant Profile Generator

	This helper program will generate an Illuminant Profile to be use in
	Krita.

	It takes the illuminant spectrum as input and returns a .ill file as
	output, containing the transformation matrix for that illuminant.
*/

#include <cmath>

#include <fstream>
#include <iostream>
#include <string>

#include <gmm/gmm.h>
#include <gmp.h>

#include "matching_curves.h"

using namespace gmm;
using namespace std;

#define PREC 1024
#define NUM 10

class {
	long double mH[471];
	long double mW[NUM];
	long double mX[NUM];
	long double m_matrix[3][NUM];

	public:
		long double &H(int i)
		{
			if (i >= 0 && i <= 470)
				return mH[i];
		}
		long double &W(int i)
		{
			if (i >= 0 && i < NUM)
				return mW[i];
		}
		long double &X(int i)
		{
			if (i >= 0 && i < NUM)
				return mX[i];
		}
		long double &matrix(int i, int j)
		{
			if (i >= 0 && i < 3 && j >= 0 && j < NUM)
				return m_matrix[i][j];
		}
		const long double x(int i)
		{
			if (i >= 0 && i <= 470)
				return cmf::x[i];
		}
		const long double y(int i)
		{
			if (i >= 0 && i <= 470)
				return cmf::y[i];
		}
		const long double z(int i)
		{
			if (i >= 0 && i <= 470)
				return cmf::z[i];
		}
		const long double F(int i)
		{
			if (i >= 0 && i <= 470)
				return (x(i)+y(i)+z(i));
		}
		const long double W_func(int i)
		{
			if (i >= 0 && i <= 470)
				return H(i) * F(i);
		}
} C;

void init_v(mpf_t v[]);
void clear_v(mpf_t v[]);
void get_v(mpf_t v[], long double d[]);
void pvalue(mpf_t v[], long double x, mpf_t res);

void copy(mpf_t from[], mpf_t to[]);
void add(mpf_t v1[], mpf_t v2[], mpf_t vr[]);
void sub(mpf_t v1[], mpf_t v2[], mpf_t vr[]);
void mul(mpf_t v[], mpf_t k, mpf_t vr[]);
void div(mpf_t v[], mpf_t k, mpf_t vr[]);
void scalar(mpf_t v1[], mpf_t v2[], mpf_t r);

long double standardIntegral(long double p);
bool loadIlluminant(char *filename);
bool computeRoots();
bool computeWeights();
bool computeMatrix();
bool saveMatrix(char *filename);

int main(int argc, char *argv[])
{
	bool goon;

	if (argc != 3) {
		cout << "Usage: " << argv[0] << " inputfile outputfile" << endl;
		return 255;
	}

	mpf_set_default_prec(PREC);

	goon = loadIlluminant(argv[1]);
	if (!goon)
		return 1;

	goon = computeRoots();
	if (!goon)
		return 2;

	goon = computeWeights();
	if (!goon)
		return 3;

	goon = computeMatrix();
	if (!goon)
		return 4;

	goon = saveMatrix(argv[2]);
	if (!goon)
		return 5;

	return 0;
}

#define STD_FOR for (int i = 0; i <= NUM; i++)

void init_v(mpf_t v[])
{
	STD_FOR
		mpf_init(v[i]);
}

void clear_v(mpf_t v[])
{
	STD_FOR
		mpf_clear(v[i]);
}

void get_v(mpf_t v[], long double d[])
{
	STD_FOR
		d[i] = mpf_get_d(v[i]);
}

void pvalue(mpf_t v[], long double x, mpf_t res)
{
	mpf_t tmp, X;

	mpf_init(tmp);
	mpf_init(X);

	mpf_set_d(res, 0.0);
	STD_FOR {
		mpf_set_d(X, x);
		mpf_pow_ui(X, X, i);
		mpf_mul(tmp, v[i], X);
		mpf_add(res, res, tmp);
	}

	mpf_clear(tmp);
	mpf_clear(X);
}

void copy(mpf_t from[], mpf_t to[])
{
	STD_FOR
		mpf_set(to[i], from[i]);
}

void add(mpf_t v1[], mpf_t v2[], mpf_t vr[])
{
	STD_FOR
		mpf_add(vr[i], v1[i], v2[i]);
}

void sub(mpf_t v1[], mpf_t v2[], mpf_t vr[])
{
	STD_FOR
		mpf_sub(vr[i], v1[i], v2[i]);
}

void mul(mpf_t v[], mpf_t k, mpf_t vr[])
{
	STD_FOR
		mpf_mul(vr[i], v[i], k);
}

void div(mpf_t v[], mpf_t k, mpf_t vr[])
{
	STD_FOR
		mpf_div(vr[i], v[i], k);
}

void scalar(mpf_t v1[], mpf_t v2[], mpf_t r)
{
	mpf_t W, norm;

	mpf_init(W);
	mpf_init(norm);

	mpf_set_d(norm, 2.0/470.0);

	mpf_set_d(r, 0.0);
	for (int i = 0; i <= 470; i++) {
		mpf_t val1, val2, curr;
		long double x = 2.0*(long double)(i)/470.0 - 1.0;
// 		long double x = 360+i;

		mpf_init(val1);
		mpf_init(val2);
		mpf_init(curr);

		mpf_set_d(W, C.W_func(i));
		pvalue(v1, x, val1);
		pvalue(v2, x, val2);

		mpf_mul(curr, val1, val2);
		mpf_mul(curr, curr, W);
		mpf_mul(curr, curr, norm);
		mpf_add(r, r, curr);

		mpf_clear(curr);
		mpf_clear(val2);
		mpf_clear(val1);
	}

	mpf_clear(norm);
	mpf_clear(W);
}

long double standardIntegral(long double p)
{
	const long double norm = 2.0/470.0;
	long double I = 0;

	for (int i = 0; i <= 470; i++) {
		long double x = 2.0*(long double)(i)/470.0 - 1.0;
		I += C.W_func(i)*pow(x, p)*norm;
	}

	return I;
}

bool loadIlluminant(char *filename)
{
	ifstream f; // TODO Add error handling

	f.open(filename);
	for (int i = 0; i <= 470; i++)
		f >> C.H(i);

	return true;
}

bool computeRoots()
{
	mpf_t v[NUM+1][NUM+1];
	mpf_t u[NUM+1][NUM+1];
	mpf_t e[NUM+1][NUM+1];

	STD_FOR {
		init_v(v[i]);
		init_v(u[i]);
		init_v(e[i]);
	}

	for (int i = 0; i <= NUM; i++) {

		mpf_set_d(v[i][i], 1.0);
		copy(v[i], u[i]);

		cout << "v = [";
		for (int k = 0; k < NUM; k++)
		cout << mpf_get_d(v[i][k]) << ",";
		cout << mpf_get_d(v[i][NUM]) << "]" << endl;

		for (int j = 0; j < i; j++) {

			mpf_t et[NUM+1];
			mpf_t st;

			init_v(et);
			mpf_init(st);

			scalar(e[j], v[i], st);
			mul(e[j], st, et);
			sub(u[i], et, u[i]);

			mpf_clear(st);
			clear_v(et);

		}

		cout << "u = [";
		for (int k = 0; k < NUM; k++)
		cout << mpf_get_d(u[i][k]) << ",";
		cout << mpf_get_d(u[i][NUM]) << "]" << endl;

		mpf_t n;

		mpf_init(n);

		scalar(u[i], u[i], n);
		mpf_sqrt(n, n);
		div(u[i], n, e[i]);

		cout << "e = [";
		for (int k = 0; k < NUM; k++)
		cout << mpf_get_d(e[i][k]) << ",";
		cout << mpf_get_d(e[i][NUM]) << "]" << endl;

		cout << "----------------------------------" << endl;

		mpf_clear(n);

	}

	long double d[NUM+1];

	get_v(e[NUM], d);

	ofstream out("/tmp/ipg.tmp");
	out.precision(128);
	out << "roots([";
	for (int i = 0; i < NUM; i++)
		out << d[NUM-i] << ",";
	out << d[0] << "])" << endl;
	out.close();

	system("octave -q /tmp/ipg.tmp > /tmp/ipg_ans_raw.tmp");
	ostringstream s;
	s << "tail -n " << (NUM+1) << " /tmp/ipg_ans_raw.tmp > /tmp/ipg_ans.tmp";
	system(s.str().c_str());

	ifstream in("/tmp/ipg_ans.tmp");

	cout << "IN [-1, 1]: ";
	for (int i = 0; i < NUM; i++) {
		long double X;
		in >> X;
		C.X(i) = X;
		cout << C.X(i) << ", ";
	}
	cout << endl;

	in.close();

	STD_FOR {
		clear_v(v[i]);
		clear_v(u[i]);
		clear_v(e[i]);
	}

	system("rm /tmp/ipg.tmp");
	system("rm /tmp/ipg_ans_raw.tmp");
	system("rm /tmp/ipg_ans.tmp");

	return true;
}

bool computeWeights()
{
	dense_matrix<long double> M(NUM, NUM);
	vector<long double> B(NUM), W(NUM);
	clear(M);
	for (int i = 0; i < NUM; i++) {
		B[i] = standardIntegral((long double)i);
		for (int j = 0; j < NUM; j++)
			M(i, j) = pow(C.X(j), (long double)i);
	}
	lu_solve(M, W, B);
	cout << B << endl << M << endl;
	cout << endl << W << endl;
	cout << "INTEGERIZED: ";
	for (int i = 0; i < NUM; i++) {
		C.W(i) = W[i];
		// Integer-ize abscissas
		C.X(i) = (int)(470.0*C.X(i)/2.0 + 470.0/2.0);
		cout << C.X(i) << ", ";
	}
	cout << endl;

	return true;
}

bool computeMatrix()
{
	cout.precision(128);
	long double Hy_integral = 0, k;

	for (int i = 0; i < NUM; i++) {
		int X = (int)C.X(i);
		Hy_integral += C.W(i)*C.y(X)/C.F(X);
	}

	Hy_integral *= 470.0/2.0;

	k = 1.0/Hy_integral;
	cout << "K: " << k << endl;

	for (int j = 0; j < NUM; j++) {
		int X = (int)C.X(j);
		C.matrix(0,j) = k*(470.0/2.0)*C.W(j)*C.x(X)/C.F(X);
	}
	for (int j = 0; j < NUM; j++) {
		int X = (int)C.X(j);
		C.matrix(1,j) = k*(470.0/2.0)*C.W(j)*C.y(X)/C.F(X);
	}
	for (int j = 0; j < NUM; j++) {
		int X = (int)C.X(j);
		C.matrix(2,j) = k*(470.0/2.0)*C.W(j)*C.z(X)/C.F(X);
	}

	cout.precision(128);
	cout << "double g_matrix[3][10] = {" << endl;
	for (int i = 0; i < 3; i++) {
		cout << "\t{" << endl;
		for (int j = 0; j < (NUM-1); j++) {
			cout.fill(' ');
			cout << "\t\t" << C.matrix(i,j) << "," << endl;
		}
		cout.fill(' ');
		cout << "\t\t" << C.matrix(i,NUM-1) << endl << "\t}";
		if (i != 2)
			cout << "," << endl;
		else
			cout << endl;
	}
	cout << "};" << endl;

	return true;
}

bool saveMatrix(char *filename)
{
	ofstream f;

	f.open(filename);

	f.precision(64);
	for (int i = 0; i < 3; i++)
		for (int j = 0; j < 10; j++)
			if (C.matrix(i,j))
				f << C.matrix(i,j) << endl;
			else
				f << 1e-64 << endl;

	return true;
}
