/*
 *  Copyright (c) 2007 Emanuele Tamponi <emanuele@valinor.it>
 *  Copyright (c) 2007 Paolo Capriotti
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
	output, containing the transformation matrices for that illuminant.
*/

#include <cmath>

#include <fstream>
#include <iostream>
#include <string>

#include <gmm/gmm.h>
#include <gmp.h>
#include <gmpxx.h>

#include "adaptation_matrices.h"
#include "matching_curves.h"
#include "precomputed_D50.h"

using namespace gmm;
using namespace std;

#define GMP_PREC 1024
#define NUM 15
#define DBL_PREC 64
#define ZERO 1e-64

class c {
	mpf_class mH[471];
	mpf_class mW[NUM];
	mpf_class mX[NUM];
	mpf_class m_matrix[3][NUM];
	mpf_class m_fromD50[3][3];
	mpf_class m_toD50[3][3];
	mpf_class m_AM[3][3];
	mpf_class m_AM_1[3][3];

	public:
		mpf_class &H(int i)
		{
			if (i >= 0 && i <= 470)
				return mH[i];
		}
		mpf_class &W(int i)
		{
			if (i >= 0 && i < NUM)
				return mW[i];
		}
		mpf_class &X(int i)
		{
			if (i >= 0 && i < NUM)
				return mX[i];
		}
		mpf_class &matrix(int i, int j)
		{
			if (i >= 0 && i < 3 && j >= 0 && j < NUM)
				return m_matrix[i][j];
		}
		mpf_class &fromD50(int i, int j)
		{
			if (i >= 0 && i < 3 && j >= 0 && j < 3)
				return m_fromD50[i][j];
		}
		mpf_class &toD50(int i, int j)
		{
			if (i >= 0 && i < 3 && j >= 0 && j < 3)
				return m_toD50[i][j];
		}
		mpf_class &AM(int i, int j)
		{
			if (i >= 0 && i < 3 && j >= 0 && j < 3)
				return m_AM[i][j];
		}
		mpf_class &AM_1(int i, int j)
		{
			if (i >= 0 && i < 3 && j >= 0 && j < 3)
				return m_AM_1[i][j];
		}
		const mpf_class x(int i)
		{
			if (i >= 0 && i <= 470) {
				mpf_class X(cmf::x[i]);
				return X;
			}
		}
		const mpf_class y(int i)
		{
			if (i >= 0 && i <= 470) {
				mpf_class Y(cmf::y[i]);
				return Y;
			}
		}
		const mpf_class z(int i)
		{
			if (i >= 0 && i <= 470) {
				mpf_class Z(cmf::z[i]);
				return Z;
			}
		}
		const mpf_class F(int i)
		{
			if (i >= 0 && i <= 470)
				return x(i)+y(i)+z(i);
		}
		const mpf_class W_func(int i)
		{
			if (i >= 0 && i <= 470)
				return H(i)*F(i);
		}
        const mpf_class final_normalization(int i)
        {
            if (i >= 0 && i <= 470)
                return 1.0/F(i);
        }
} C;

void get_v(mpf_class v[], double d[]);
void pvalue(mpf_class v[], mpf_class X, mpf_class &res);

void copy(mpf_class from[], mpf_class to[]);
void add(mpf_class v1[], mpf_class v2[], mpf_class vr[]);
void sub(mpf_class v1[], mpf_class v2[], mpf_class vr[]);
void mul(mpf_class v[], mpf_class k, mpf_class vr[]);
void div(mpf_class v[], mpf_class k, mpf_class vr[]);
mpf_class pow(mpf_class d, int p);
void scalar(mpf_class v1[], mpf_class v2[], mpf_class &r);

void AM_mul(mpf_class M[3][3], mpf_class v[], mpf_class vr[]);

mpf_class standardIntegral(int p);
bool loadIlluminant(char *filename);
bool chooseAdaptationMatrix(char *type);
bool computeRoots();
bool computeWeights();
bool computeMatrix();
bool computeD50Conversions();
bool saveMatrix(char *profilename, char *filename);

int main(int argc, char *argv[])
{
	bool goon;

	if (argc != 5) {
		cout << "Usage: " << argv[0] << " input_file adaptation_type profile_name output_file" << endl;
		return 255;
	} else {
		string choice;
		cout << "I'm going to create a profile with these settings:" << endl;
		cout << "\t- Input file: " << argv[1] << endl;
		cout << "\t- Adaptation matrix: " << argv[2] << endl;
		cout << "\t- Output profile name: " << argv[3] << endl;
		cout << "\t- Output profile filename: " << argv[4] << endl;
		cout << "Continue? [Y/n] " << endl;
again:
		cin >> choice;
		if (choice == "Y" || choice == "y" || choice == "yes" || choice == "Yes" || choice == "YES") {
			cout << "Starting..." << endl;
			goto start;
		}
		if (choice == "n" || choice == "N" || choice == "no" || choice == "No" || choice == "NO") {
			cout << "Bye" << endl;
			return 254;
		}
		cout << "You should answer Y or N" << endl;
		goto again;
	}

start:
	mpf_set_default_prec(GMP_PREC);

	goon = loadIlluminant(argv[1]);
	if (!goon)
		return 1;

	goon = chooseAdaptationMatrix(argv[2]);
	if (!goon)
		return 2;

	goon = computeRoots();
	if (!goon)
		return 3;

	goon = computeWeights();
	if (!goon)
		return 4;

	goon = computeMatrix();
	if (!goon)
		return 5;

	goon = computeD50Conversions();
	if (!goon)
		return 6;

	goon = saveMatrix(argv[3], argv[4]);
	if (!goon)
		return 7;

	return 0;
}

#define STD_FOR for (int i = 0; i <= NUM; i++)

void get_v(mpf_class v[], double d[])
{
	STD_FOR
		d[i] = v[i].get_d();
}

void pvalue(mpf_class v[], mpf_class X, mpf_class &res)
{
	res = 0.0;
	STD_FOR
		res += pow(X,i) * v[i];
}

void copy(mpf_class from[], mpf_class to[])
{
	STD_FOR
		to[i] = from[i];
}

void add(mpf_class v1[], mpf_class v2[], mpf_class vr[])
{
	STD_FOR
		vr[i] = v1[i] + v2[i];
}

void sub(mpf_class v1[], mpf_class v2[], mpf_class vr[])
{
	STD_FOR
		vr[i] = v1[i] - v2[i];
}

void mul(mpf_class v[], mpf_class k, mpf_class vr[])
{
	STD_FOR
		vr[i] = v[i] * k;
}

void div(mpf_class v[], mpf_class k, mpf_class vr[])
{
	STD_FOR
		vr[i] = v[i] / k;
}

mpf_class pow(mpf_class d, int p)
{
	mpf_class res;

	if (p == 0) {
		res = 1;
		return res;
	}

	res = d;

	for (int i = 1; i < abs(p); i++)
		res *= d;

	if (p < 0)
		res = 1.0/res;

	return res;
}

/*
   Thanks to Paolo Capriotti for this.
   Do the "scalar product" operation as:

   < v1, v2 > = integ_a^b ( w(x)v1(x)v2(x) )

   It respects all the properties of the scalar product.
*/
void scalar(mpf_class v1[], mpf_class v2[], mpf_class &r)
{
	mpf_class W, norm;

	norm = 2.0/470.0;

	r = 0.0;
	for (int i = 0; i <= 470; i++) {
		mpf_class val1, val2, x;

		x = 2.0*(double)(i)/470.0 - 1.0;
		W = C.W_func(i);

		pvalue(v1, x, val1);
		pvalue(v2, x, val2);

		r += W*val1*val2*norm;
	}
}

mpf_class standardIntegral(int p)
{
	mpf_class norm, I;

	norm = 2.0/470.0;
	I = 0.0;

	for (int i = 0; i <= 470; i++) {
		mpf_class tmp, x;

		x = 2.0*(double)(i)/470.0 - 1.0;

		I += pow(x, p)*C.W_func(i);
	}
    I *= norm;

	return I;
}

bool loadIlluminant(char *filename)
{
	ifstream f; // TODO Add error handling

	f.open(filename);
	for (int i = 0; i <= 470; i++) {
		double H;
		f >> H;
		C.H(i) = H;
	}

	return true;
}

bool chooseAdaptationMatrix(char *type)
{
	bool how = false;
	const double (*myAM)[3];
	const double (*myAM_1)[3];
	if (strcmp(type, "XYZ") == 0) {
		myAM = AM::XYZ;
		myAM_1 = AM::XYZ_1;
		how = true;
	}
	if (strcmp(type, "Bradford") == 0) {
		myAM = AM::Bradford;
		myAM_1 = AM::Bradford_1;
		how = true;
	}
	if (strcmp(type, "VonKries") == 0) {
		myAM = AM::VonKries;
		myAM_1 = AM::VonKries_1;
		how = true;
	}

	if (how) {
		for (int i = 0; i < 3; i++) {
			for (int j = 0; j < 3; j++) {
				C.AM(i,j) = myAM[i][j];
				C.AM_1(i,j) = myAM_1[i][j];
			}
		}
	}

	return how;
}

/*
   Thanks to Paolo Capriotti for this.
   Implement Gram-Schmidt to calculate an orthogonal base.
   The last component of this base is a Legendre Polynomial.
   Use this polynomialto calculate the roots.
*/
bool computeRoots()
{
	mpf_class v[NUM+1][NUM+1];
	mpf_class u[NUM+1][NUM+1];
	mpf_class e[NUM+1][NUM+1];

	for (int i = 0; i <= NUM; i++) {

		v[i][i] = 1.0;
		copy(v[i], u[i]);

		cout << "v = [";
		for (int k = 0; k < NUM; k++)
		cout << v[i][k].get_d() << ",";
		cout << v[i][NUM].get_d() << "]" << endl;

		for (int j = 0; j < i; j++) {

			mpf_class et[NUM+1];
			mpf_class st;

			scalar(e[j], v[i], st);
			mul(e[j], st, et);
			sub(u[i], et, u[i]);
		}

		cout << "u = [";
		for (int k = 0; k < NUM; k++)
		cout << u[i][k].get_d() << ",";
		cout << u[i][NUM].get_d() << "]" << endl;

		mpf_class n;

		scalar(u[i], u[i], n);
		n = sqrt(n);
		div(u[i], n, e[i]);

		cout << "e = [";
		for (int k = 0; k < NUM; k++)
		cout << e[i][k].get_d() << ",";
		cout << e[i][NUM].get_d() << "]" << endl;

		cout << "----------------------------------" << endl;
	}

	double d[NUM+1];

	get_v(e[NUM], d);

	ofstream out("/tmp/ipg.tmp");
	out.precision(DBL_PREC);
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
		double X;
		in >> X;
		C.X(i) = X;
		cout << X << ", ";
	}
	cout << endl;

	in.close();

	system("rm /tmp/ipg.tmp");
	system("rm /tmp/ipg_ans_raw.tmp");
	system("rm /tmp/ipg_ans.tmp");

	return true;
}

bool computeWeights()
{
	dense_matrix<mpf_class> M(NUM, NUM);
	vector<mpf_class> B(NUM), W(NUM);
	clear(M);
	for (int i = 0; i < NUM; i++) {
		B[i] = standardIntegral(i);
		for (int j = 0; j < NUM; j++)
			M(i,j) = pow(C.X(j), i);
	}
	lu_solve(M, W, B);
	cout << B << endl << M << endl;
	cout << endl << W << endl;
	cout << "INTEGERIZED: ";
	for (int i = 0; i < NUM; i++) {
		C.W(i) = W[i];
		// Integer-ize abscissas
		int X = (int)(470.0*C.X(i).get_d()/2.0 + 470.0/2.0);
		C.X(i) = X;
		cout << X << ", ";
	}
	cout << endl;

	return true;
}

bool computeMatrix()
{
	cout.precision(DBL_PREC);
	mpf_class Hy_integral(0.0), k;

	for (int i = 0; i < NUM; i++) {
		int X = C.X(i).get_ui();
		Hy_integral += C.W(i)*C.y(X)*C.final_normalization(X);
	}

	Hy_integral *= 470.0/2.0;

	k = 1.0/Hy_integral;
	cout << "K: " << k << endl;

	for (int j = 0; j < NUM; j++) {
		int X = C.X(j).get_ui();
		C.matrix(0,j) = k*(470.0/2.0)*C.W(j)*C.x(X)*C.final_normalization(X);
	}
	for (int j = 0; j < NUM; j++) {
		int X = C.X(j).get_ui();
		C.matrix(1,j) = k*(470.0/2.0)*C.W(j)*C.y(X)*C.final_normalization(X);
	}
	for (int j = 0; j < NUM; j++) {
		int X = C.X(j).get_ui();
		C.matrix(2,j) = k*(470.0/2.0)*C.W(j)*C.z(X)*C.final_normalization(X);
	}

    for (int i = 0; i < 3; i++)
        for (int j = 0; j < NUM; j++)
            if (C.matrix(i,j) < ZERO)
                C.matrix(i,j) = ZERO;

	return true;
}

bool computeD50Conversions() {
	mpf_class wD50[3];
	mpf_class wCur[3];
	mpf_class rD50[3];
	mpf_class rCur[3];

	mpf_class rMatFrom[3][3];
	mpf_class rMatTo[3][3];
	mpf_class tempFrom[3][3];
	mpf_class tempTo[3][3];

	cout << "White points" << endl;
	for (int i = 0; i < 3; i++) {
		wD50[i] = precom::wD50[i];

		wCur[i] = 0.0;
		for (int j = 0; j < NUM; j++)
			wCur[i] += C.matrix(i,j);

		cout.precision(7);
		cout << wD50[i] << "\t" << wCur[i] << endl;
	}
	cout << endl;

	cout << "Cone domain white points" << endl;
	for (int i = 0; i < 3; i++) {
		rD50[i] = rCur[i] = 0.0;
		for (int j = 0; j < 3; j++) {
			rD50[i] += C.AM(i,j) * wD50[j];
			rCur[i] += C.AM(i,j) * wCur[j];
		}

		cout.precision(7);
		cout << rD50[i] << "\t" << rCur[i] << endl;
	}
	cout << endl;

	cout << "Cone domain from D50 matrix" << endl;
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
			if (i == j) {
				rMatFrom[i][j] = rCur[i]/rD50[i];
				rMatTo[i][j] = rD50[i]/rCur[i];
			} else {
				rMatFrom[i][j] = 0.0;
				rMatTo[i][j] = 0.0;
			}
			cout.precision(7);
			cout.width(12);
			cout << rMatFrom[i][j];
		}
		cout << endl;
	}
	cout << endl;

	cout << "Temporary matrix, AM*Cone" << endl;
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
			tempFrom[i][j] = tempTo[i][j] = 0.0;
			for (int k = 0; k < 3; k++) {
				tempFrom[i][j] += C.AM(i,k)*rMatFrom[k][j];
				tempTo[i][j] += C.AM(i,k)*rMatTo[k][j];
			}
			cout.precision(7);
			cout.width(12);
			cout << tempFrom[i][j];
		}
		cout << endl;
	}
	cout << endl;

	cout << "Final chromatic adaptation matrix, Temp*AM_1" << endl;
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
			C.fromD50(i,j) = C.toD50(i,j) = 0.0;
			for (int k = 0; k < 3; k++) {
				C.fromD50(i,j) += tempFrom[i][k]*C.AM_1(k,j);
				C.toD50(i,j) += tempTo[i][k]*C.AM_1(k,j);
			}
			cout.precision(7);
			cout.width(12);
			cout << C.fromD50(i,j);
		}
		cout << endl;
	}
	cout << endl;

	return true;
}

bool saveMatrix(char *profilename, char *filename)
{
	ofstream f;

	f.open(filename);

	f << profilename << endl;

	f.precision(DBL_PREC);
	for (int i = 0; i < 3; i++)
		for (int j = 0; j < NUM; j++)
            f << C.matrix(i,j).get_d() << endl;

	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++)
			f << C.fromD50(i,j).get_d() << "\t";
		f << endl;
	}
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++)
			f << C.toD50(i,j).get_d() << "\t";
		f << endl;
	}

	return true;
}
