/*
 * panoptim_p.h -- Part of Krita
 *
 * Copyright (c) 2007 Cyrille Berger (cberger@cberger.net)
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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA. */

#ifndef _PANOPTIM_FUNCTIONS_H_
#define _PANOPTIM_FUNCTIONS_H_

class BaseFunction {
    protected:
        BaseFunction() : m_epsilon(1e-3) {}
    protected:
        inline double poly(double a, double b, double c, double u)
        {
            return 1.0 + a*u+b*u*u+c*u*u*u;
        }
        inline double func(double a, double b, double c, double r1, double t1c, double t1, double t2c)
        {
            // Compute r2
            double r2 = r1;
            double v;
            int count = 0;
            while( fabs( v = r1 - r2*powf(poly(a,b,c,r2),2.0)) > m_epsilon and (++count) < 1000 )
            {
                r2 += 0.1*v;
            }
            if( count >= 10000)
            {
                kDebug() << "Too much iterations in estimating r2 v = " << v << endl;
            }
            if( r2 < 0 )
            {
                kDebug() << "r2 can't be of negative value, r2 = " << r2 << " a = " << a << " b = " << b << " r1 = " << r1 << endl;
                return 0.0;
            }
            // Compute t2
//             kDebug() << "Estimated r2 = " << r2 << endl;
            return (t1-t1c) / poly(a,b,c,r2) + t2c;
        }
        inline double derivEnA(double a, double b, double c, double r, double t1c, double t1, double t2c)
        {
            return (func(a + 1e-12, b,c,r, t1c, t1, t2c) - func(a - 1e-12, b,c,r, t1c, t1, t2c)) / ( 2e-12 );
        }
        inline double derivEnB(double a, double b, double c, double r, double t1c, double t1, double t2c)
        {
            return (func(a, b + 1e-12,c,r, t1c, t1, t2c) - func(a, b - 1e-12,c,r, t1c, t1, t2c)) / ( 2e-12 );
        }
        inline double derivEnC(double a, double b, double c, double r, double t1c, double t1, double t2c)
        {
            return (func(a, b,c+ 1e-12,r, t1c, t1, t2c) - func(a, b,c - 1e-12,r, t1c, t1, t2c)) / ( 2e-12 );
        }
    private:
        double m_epsilon;
};

// For each image there is four parameters to estimate : translation (2 parameters) + distortion (2 parameters)
class RotationFunction : public BaseFunction {
    public:
        enum indexes {
            INDX_a1, INDX_b1, INDX_c1, INDX_a2, INDX_b2, INDX_c2, INDX_angle, INDX_tx21, INDX_ty21
        };
    public:
        RotationFunction(const int idx[12], double xc, double yc, double norm, double i1, double j1, double i2, double j2) :
            m_xc(xc), m_yc(yc), m_norm(norm), m_i1(i1), m_j1(j1), m_i2(i2), m_j2(j2)
        {
            memcpy(m_idx, idx, 12 * sizeof(int));
        }
        void f(const std::vector<double>& parameters, double& res_f1, double& res_f2)
        {
            double aim1 = parameters[ m_idx[ INDX_a1 ] ];
            double bim1 = parameters[ m_idx[ INDX_b1 ] ];
            double cim1 = parameters[ m_idx[ INDX_c1 ] ];
            double aim2 = parameters[ m_idx[ INDX_a2 ] ];
            double bim2 = parameters[ m_idx[ INDX_b2 ] ];
            double cim2 = parameters[ m_idx[ INDX_c2 ] ];
            double angle21 = parameters[ m_idx[ INDX_angle ] ];
            double cosAngle21 = cos(angle21);
            double sinAngle21 = sin(angle21);
            double tx21 = parameters[ m_idx[ INDX_tx21 ] ];
            double ty21 = parameters[ m_idx[ INDX_ty21 ] ];
            
            double r1 = (m_i1 - m_xc) * (m_i1 - m_xc) + (m_j1 - m_yc) * (m_j1 - m_yc);
            r1 *= m_norm;
            double r2 = (m_i2 - m_xc) * (m_i2 - m_xc) + (m_j2 - m_yc) * (m_j2 - m_yc);
            r2 *= m_norm;
            double fx1 = func(aim1, bim1, cim1, r1, m_xc, m_i1, m_xc);
            double fy1 = func(aim1, bim1, cim1, r1, m_yc, m_j1, m_yc);
            double fx2 = func(aim2, bim2, cim2, r2, m_xc, m_i2, m_xc);
            double fy2 = func(aim2, bim2, cim2, r2, m_yc, m_j2, m_yc);
            res_f1 = (fx1 - (cosAngle21 * fx2 - sinAngle21 * fy2 + tx21));
            res_f2 = (fy1 - (sinAngle21 * fx2 + cosAngle21 * fy2 + ty21));
        }
        void jac(const std::vector<double>& parameters, gmm::row_matrix< gmm::wsvector<double> >& jt, int pos)
        {
            double aim1 = parameters[ m_idx[ INDX_a1 ] ];
            double bim1 = parameters[ m_idx[ INDX_b1 ] ];
            double cim1 = parameters[ m_idx[ INDX_c1 ] ];
            double aim2 = parameters[ m_idx[ INDX_a2 ] ];
            double bim2 = parameters[ m_idx[ INDX_b2 ] ];
            double cim2 = parameters[ m_idx[ INDX_c2 ] ];
            double angle21 = parameters[ m_idx[ INDX_angle ] ];
            double cosAngle21 = cos(angle21);
            double sinAngle21 = sin(angle21);
            
            double r1 = (m_i1 - m_xc) * (m_i1 - m_xc) + (m_j1 - m_yc) * (m_j1 - m_yc);
            r1 *= m_norm;
            double r2 = (m_i2 - m_xc) * (m_i2 - m_xc) + (m_j2 - m_yc) * (m_j2 - m_yc);
            r2 *= m_norm;
            // Compute the derivatives
            double da2f1 = derivEnA(aim2, bim2, cim2, r2, m_xc, m_i2, m_xc);
            double db2f1 = derivEnB(aim2, bim2, cim2, r2, m_xc, m_i2, m_xc);
            double dc2f1 = derivEnC(aim2, bim2, cim2, r2, m_xc, m_i2, m_xc);
            double da2f2 = derivEnA(aim2, bim2, cim2, r2, m_yc, m_j2, m_yc);
            double db2f2 = derivEnB(aim2, bim2, cim2, r2, m_yc, m_j2, m_yc);
            double dc2f2 = derivEnC(aim2, bim2, cim2, r2, m_yc, m_j2, m_yc);
            double fx2 = func(aim2, bim2, cim2, r2, m_xc, m_i2, m_xc);
            double fy2 = func(aim2, bim2, cim2, r2, m_yc, m_j2, m_yc);
            // Compute the jacobian
            jt(pos, 0) = derivEnA(aim1, bim1, cim1, r1, m_xc, m_i1, m_xc);
            jt(pos, 1) = derivEnB(aim1, bim1, cim1, r1, m_xc, m_i1, m_xc);
            jt(pos, 2) = derivEnC(aim1, bim1, cim1, r1, m_xc, m_i1, m_xc);
            jt(pos, 3) = -(cosAngle21 * da2f1 - sinAngle21 * da2f2);
            jt(pos, 4) = -(cosAngle21 * db2f1 - sinAngle21 * db2f2);
            jt(pos, 5) = -(cosAngle21 * dc2f1 - sinAngle21 * dc2f2);
            jt(pos, 6) = -(-sinAngle21 * fx2 - cosAngle21 * fy2);
            jt(pos, 7) = -1;
            jt(pos, 8) = 0;
            jt(pos + 1, 0) = derivEnA(aim1, bim1, cim1, r1, m_yc, m_i1, m_xc);
            jt(pos + 1, 1) = derivEnB(aim1, bim1, cim1, r1, m_yc, m_i1, m_xc);
            jt(pos + 1, 2) = derivEnC(aim1, bim1, cim1, r1, m_yc, m_i1, m_xc);
            jt(pos + 1, 3) = -(sinAngle21 * da2f1 + cosAngle21 * da2f2);
            jt(pos + 1, 4) = -(sinAngle21 * db2f1 + cosAngle21 * db2f2);
            jt(pos + 1, 5) = -(sinAngle21 * dc2f1 + cosAngle21 * dc2f2);
            jt(pos + 1, 6) = -(cosAngle21 * fx2 - sinAngle21 * fy2);
            jt(pos + 1, 7) = 0;
            jt(pos + 1, 8) = -1;
        }
    private:
        int m_idx[12];
        double m_xc, m_yc, m_norm, m_epsilon;
        double m_i1, m_j1, m_i2, m_j2;
};

class HomographyFunction : public BaseFunction {
    public:
        enum indexes {
            INDX_a1, INDX_b1, INDX_c1, INDX_a2, INDX_b2, INDX_c2, INDX_h11, INDX_h21, INDX_h31, INDX_h12, INDX_h22, INDX_h32, INDX_h13, INDX_h23, SIZEINDEXES
        };
    public:
        HomographyFunction(const int idx[SIZEINDEXES], double xc, double yc, double norm, double i1, double j1, double i2, double j2) :
            m_xc(xc), m_yc(yc), m_norm(norm), m_i1(i1), m_j1(j1), m_i2(i2), m_j2(j2)
        {
            memcpy(m_idx, idx, SIZEINDEXES * sizeof(int));
        }
        void f(const std::vector<double>& parameters, double& res_f1, double& res_f2)
        {
            double aim1 = parameters[ m_idx[ INDX_a1 ] ];
            double bim1 = parameters[ m_idx[ INDX_b1 ] ];
            double cim1 = parameters[ m_idx[ INDX_c1 ] ];
            double aim2 = parameters[ m_idx[ INDX_a2 ] ];
            double bim2 = parameters[ m_idx[ INDX_b2 ] ];
            double cim2 = parameters[ m_idx[ INDX_c2 ] ];
            double h11 = parameters[ m_idx[ INDX_h11 ] ];
            double h21 = parameters[ m_idx[ INDX_h21 ] ];
            double h31 = parameters[ m_idx[ INDX_h31 ] ];
            double h12 = parameters[ m_idx[ INDX_h12 ] ];
            double h22 = parameters[ m_idx[ INDX_h22 ] ];
            double h32 = parameters[ m_idx[ INDX_h32 ] ];
            double h13 = parameters[ m_idx[ INDX_h13 ] ];
            double h23 = parameters[ m_idx[ INDX_h23 ] ];
            
            double r1 = (m_i1 - m_xc) * (m_i1 - m_xc) + (m_j1 - m_yc) * (m_j1 - m_yc);
            r1 *= m_norm;
            double r2 = (m_i2 - m_xc) * (m_i2 - m_xc) + (m_j2 - m_yc) * (m_j2 - m_yc);
            r2 *= m_norm;
            double fx1 = func(aim1, bim1, cim1, r1, m_xc, m_i1, m_xc);
            double fy1 = func(aim1, bim1, cim1, r1, m_yc, m_j1, m_yc);
//             kDebug() << "Real r1 = " << ( (fx1- m_xc) * (fx1-m_xc) + (fy1-m_yc) * (fy1-m_yc)) * m_norm << " fx1 = " << fx1 << " fy1 = " << fy1 << " " << m_i1 << " " << m_j1 << endl;
            double fx2 = func(aim2, bim2, cim2, r2, m_xc, m_i2, m_xc);
            double fy2 = func(aim2, bim2, cim2, r2, m_yc, m_j2, m_yc);
            double norm = 1.0 / ( h13 * fx2 + h23 * fy2 + 1.0 );
//             kDebug() << "Real r2 = " << ( (fx2 - m_xc ) * (fx2 - m_xc) + (fy2 - m_yc) * (fy2 -m_yc)) * m_norm << " fx2 = " << fx2 << " fy2 = " << fy2 << " " << m_i2 << " " << m_j2 << " " << ((h11 * fx2 + h21 * fy2 + h31) * norm) << " " << ((h12 * fx2 + h22 * fy2 + h32) * norm) << endl;
            res_f1 = (fx1 - (h11 * fx2 + h21 * fy2 + h31) * norm);
            res_f2 = (fy1 - (h12 * fx2 + h22 * fy2 + h32) * norm);
        }
        void jac(const std::vector<double>& parameters, gmm::row_matrix< gmm::wsvector<double> >& jt, int pos)
        {
            double aim1 = parameters[ m_idx[ INDX_a1 ] ];
            double bim1 = parameters[ m_idx[ INDX_b1 ] ];
            double cim1 = parameters[ m_idx[ INDX_c1 ] ];
            double aim2 = parameters[ m_idx[ INDX_a2 ] ];
            double bim2 = parameters[ m_idx[ INDX_b2 ] ];
            double cim2 = parameters[ m_idx[ INDX_c2 ] ];
            double h11 = parameters[ m_idx[ INDX_h11 ] ];
            double h21 = parameters[ m_idx[ INDX_h21 ] ];
            double h31 = parameters[ m_idx[ INDX_h31 ] ];
            double h12 = parameters[ m_idx[ INDX_h12 ] ];
            double h22 = parameters[ m_idx[ INDX_h22 ] ];
            double h32 = parameters[ m_idx[ INDX_h32 ] ];
            double h13 = parameters[ m_idx[ INDX_h13 ] ];
            double h23 = parameters[ m_idx[ INDX_h23 ] ];
            
            double r1 = (m_i1 - m_xc) * (m_i1 - m_xc) + (m_j1 - m_yc) * (m_j1 - m_yc);
            r1 *= m_norm;
            double r2 = (m_i2 - m_xc) * (m_i2 - m_xc) + (m_j2 - m_yc) * (m_j2 - m_yc);
            r2 *= m_norm;
            // Compute the derivatives
            double da2f1 = derivEnA(aim2, bim2, cim2, r2, m_xc, m_i2, m_xc);
            double db2f1 = derivEnB(aim2, bim2, cim2, r2, m_xc, m_i2, m_xc);
            double dc2f1 = derivEnC(aim2, bim2, cim2, r2, m_xc, m_i2, m_xc);
            double da2f2 = derivEnA(aim2, bim2, cim2, r2, m_yc, m_j2, m_yc);
            double db2f2 = derivEnB(aim2, bim2, cim2, r2, m_yc, m_j2, m_yc);
            double dc2f2 = derivEnC(aim2, bim2, cim2, r2, m_yc, m_j2, m_yc);
            double fx2 = func(aim2, bim2, cim2, r2, m_xc, m_i2, m_xc);
            double fy2 = func(aim2, bim2, cim2, r2, m_yc, m_j2, m_yc);
            double norm = 1.0 / ( h13 * fx2 + h23 * fy2 + 1.0 );
            
//             double norm = 1.0 / ( h13 * fx2 + h23 * fy2 + 1.0 );
//             res_f1 = (fx1 - (h11 * fx2 + h21 * fy2 + h31) * norm);
//             res_f2 = (fy1 - (h12 * fx2 + h22 * fy2 + h32) * norm);
            
            // Compute the jacobian
            // derivative of the first function
            jt(pos, 0) = derivEnA(aim1, bim1, cim1, r1, m_xc, m_i1, m_xc);
            jt(pos, 1) = derivEnB(aim1, bim1, cim1, r1, m_xc, m_i1, m_xc);
            jt(pos, 2) = derivEnC(aim1, bim1, cim1, r1, m_xc, m_i1, m_xc);
            jt(pos, 3) = -(da2f1* h11 + da2f2 * h21) * norm;
            jt(pos, 4) = -(db2f1* h11 + db2f2 * h21) * norm;
            jt(pos, 5) = -(dc2f1* h11 + dc2f2 * h21) * norm;
            jt(pos, 6) = -(fx2)*norm; // dh11
            jt(pos, 7) = -(fy2)*norm; // dh21
            jt(pos, 8) = -norm; // dh31
            jt(pos, 9) = 0; // dh12
            jt(pos, 10) = 0; // dh22
            jt(pos, 11) = 0; // dh32
            jt(pos, 12) = (h11 * fx2 + h21 * fy2 + h31) * norm * norm * fx2; // dh13 note: (-1) * (-1) = +1
            jt(pos, 13) = (h11 * fx2 + h21 * fy2 + h31) * norm * norm * fy2; // dh23 note: (-1) * (-1) = +1
            // derivative of the second function
            jt(pos + 1, 0) = derivEnA(aim1, bim1, cim1, r1, m_yc, m_j1, m_yc);
            jt(pos + 1, 1) = derivEnB(aim1, bim1, cim1, r1, m_yc, m_j1, m_yc);
            jt(pos + 1, 2) = derivEnC(aim1, bim1, cim1, r1, m_yc, m_j1, m_yc);
            jt(pos + 1, 3) = -(da2f1* h12 + da2f2 * h22) * norm;
            jt(pos + 1, 4) = -(db2f1* h12 + db2f2 * h22) * norm;
            jt(pos + 1, 5) = -(dc2f1* h12 + dc2f2 * h22) * norm;
            jt(pos + 1, 6) = 0; // dh12
            jt(pos + 1, 7) = 0; // dh21
            jt(pos + 1, 8) = 0; // dh31
            jt(pos + 1, 9) = -(fx2)*norm; // dh12
            jt(pos + 1, 10) = -(fy2)*norm; // dh22
            jt(pos + 1, 12) = -norm; // dh32
            jt(pos + 1, 12) = (h12 * fx2 + h22 * fy2 + h32) * norm * norm * fx2; // dh13 note: (-1) * (-1) = +1
            jt(pos + 1, 13) = (h12 * fx2 + h22 * fy2 + h32) * norm * norm * fy2; // dh23 note: (-1) * (-1) = +1
        }
//     private:
    public:
        int m_idx[SIZEINDEXES];
        double m_xc, m_yc, m_norm, m_epsilon;
        double m_i1, m_j1, m_i2, m_j2;
};

class HomographySameDistortionFunction : public BaseFunction {
    public:
        enum indexes {
            INDX_a, INDX_b, INDX_c, INDX_h11, INDX_h21, INDX_h31, INDX_h12, INDX_h22, INDX_h32, INDX_h13, INDX_h23, SIZEINDEXES
        };
    public:
        HomographySameDistortionFunction(const int idx[SIZEINDEXES], double xc, double yc, double norm, double i1, double j1, double i2, double j2) :
            m_xc(xc), m_yc(yc), m_norm(norm), m_i1(i1), m_j1(j1), m_i2(i2), m_j2(j2)
        {
            memcpy(m_idx, idx, SIZEINDEXES * sizeof(int));
        }
        void f(const std::vector<double>& parameters, double& res_f1, double& res_f2)
        {
            double aim = parameters[ m_idx[ INDX_a ] ];
            double bim = parameters[ m_idx[ INDX_b ] ];
            double cim = parameters[ m_idx[ INDX_c ] ];
            double h11 = parameters[ m_idx[ INDX_h11 ] ];
            double h21 = parameters[ m_idx[ INDX_h21 ] ];
            double h31 = parameters[ m_idx[ INDX_h31 ] ];
            double h12 = parameters[ m_idx[ INDX_h12 ] ];
            double h22 = parameters[ m_idx[ INDX_h22 ] ];
            double h32 = parameters[ m_idx[ INDX_h32 ] ];
            double h13 = parameters[ m_idx[ INDX_h13 ] ];
            double h23 = parameters[ m_idx[ INDX_h23 ] ];
            
            double r1 = (m_i1 - m_xc) * (m_i1 - m_xc) + (m_j1 - m_yc) * (m_j1 - m_yc);
            r1 *= m_norm;
            double r2 = (m_i2 - m_xc) * (m_i2 - m_xc) + (m_j2 - m_yc) * (m_j2 - m_yc);
            r2 *= m_norm;
            double fx1 = func(aim, bim, cim, r1, m_xc, m_i1, m_xc);
            double fy1 = func(aim, bim, cim, r1, m_yc, m_j1, m_yc);
//             kDebug() << "Real r1 = " << ( (fx1- m_xc) * (fx1-m_xc) + (fy1-m_yc) * (fy1-m_yc)) * m_norm << " fx1 = " << fx1 << " fy1 = " << fy1 << " " << m_i1 << " " << m_j1 << endl;
            double fx2 = func(aim, bim, cim, r2, m_xc, m_i2, m_xc);
            double fy2 = func(aim, bim, cim, r2, m_yc, m_j2, m_yc);
            double norm = 1.0 / ( h13 * fx2 + h23 * fy2 + 1.0 );
//             kDebug() << "Real r2 = " << ( (fx2 - m_xc ) * (fx2 - m_xc) + (fy2 - m_yc) * (fy2 -m_yc)) * m_norm << " fx2 = " << fx2 << " fy2 = " << fy2 << " " << m_i2 << " " << m_j2 << " " << ((h11 * fx2 + h21 * fy2 + h31) * norm) << " " << ((h12 * fx2 + h22 * fy2 + h32) * norm) << endl;
            res_f1 = (fx1 - (h11 * fx2 + h21 * fy2 + h31) * norm);
            res_f2 = (fy1 - (h12 * fx2 + h22 * fy2 + h32) * norm);
        }
        void jac(const std::vector<double>& parameters, gmm::row_matrix< gmm::wsvector<double> >& jt, int pos)
        {
            double aim = parameters[ m_idx[ INDX_a ] ];
            double bim = parameters[ m_idx[ INDX_b ] ];
            double cim = parameters[ m_idx[ INDX_c ] ];
            double h11 = parameters[ m_idx[ INDX_h11 ] ];
            double h21 = parameters[ m_idx[ INDX_h21 ] ];
            double h31 = parameters[ m_idx[ INDX_h31 ] ];
            double h12 = parameters[ m_idx[ INDX_h12 ] ];
            double h22 = parameters[ m_idx[ INDX_h22 ] ];
            double h32 = parameters[ m_idx[ INDX_h32 ] ];
            double h13 = parameters[ m_idx[ INDX_h13 ] ];
            double h23 = parameters[ m_idx[ INDX_h23 ] ];
            
            double r1 = (m_i1 - m_xc) * (m_i1 - m_xc) + (m_j1 - m_yc) * (m_j1 - m_yc);
            r1 *= m_norm;
            double r2 = (m_i2 - m_xc) * (m_i2 - m_xc) + (m_j2 - m_yc) * (m_j2 - m_yc);
            r2 *= m_norm;
            // Compute the derivatives
            double da2f1 = derivEnA(aim, bim, cim, r2, m_xc, m_i2, m_xc);
            double db2f1 = derivEnB(aim, bim, cim, r2, m_xc, m_i2, m_xc);
            double dc2f1 = derivEnC(aim, bim, cim, r2, m_xc, m_i2, m_xc);
            double da2f2 = derivEnA(aim, bim, cim, r2, m_yc, m_j2, m_yc);
            double db2f2 = derivEnB(aim, bim, cim, r2, m_yc, m_j2, m_yc);
            double dc2f2 = derivEnC(aim, bim, cim, r2, m_yc, m_j2, m_yc);
            double fx2 = func(aim, bim, cim, r2, m_xc, m_i2, m_xc);
            double fy2 = func(aim, bim, cim, r2, m_yc, m_j2, m_yc);
            double norm = 1.0 / ( h13 * fx2 + h23 * fy2 + 1.0 );
            
//             double norm = 1.0 / ( h13 * fx2 + h23 * fy2 + 1.0 );
//             res_f1 = (fx1 - (h11 * fx2 + h21 * fy2 + h31) * norm);
//             res_f2 = (fy1 - (h12 * fx2 + h22 * fy2 + h32) * norm);
            
            // Compute the jacobian
            // derivative of the first function
            jt(pos, 0) = derivEnA(aim, bim, cim, r1, m_xc, m_i1, m_xc) - (da2f1* h11 + da2f2 * h21) * norm;
            jt(pos, 1) = derivEnB(aim, bim, cim, r1, m_xc, m_i1, m_xc) - (db2f1* h11 + db2f2 * h21) * norm;
            jt(pos, 2) = derivEnC(aim, bim, cim, r1, m_xc, m_i1, m_xc) - (dc2f1* h11 + dc2f2 * h21) * norm;
            jt(pos, 3) = -(fx2)*norm; // dh11
            jt(pos, 4) = -(fy2)*norm; // dh21
            jt(pos, 5) = -norm; // dh31
            jt(pos, 6) = 0; // dh12
            jt(pos, 7) = 0; // dh22
            jt(pos, 8) = 0; // dh32
            jt(pos, 9) = (h11 * fx2 + h21 * fy2 + h31) * norm * norm * fx2; // dh13 note: (-1) * (-1) = +1
            jt(pos, 10) = (h11 * fx2 + h21 * fy2 + h31) * norm * norm * fy2; // dh23 note: (-1) * (-1) = +1
            // derivative of the second function
            jt(pos + 1, 0) = derivEnA(aim, bim, cim, r1, m_yc, m_j1, m_yc) - (da2f1* h12 + da2f2 * h22) * norm;
            jt(pos + 1, 1) = derivEnB(aim, bim, cim, r1, m_yc, m_j1, m_yc) - (db2f1* h12 + db2f2 * h22) * norm;
            jt(pos + 1, 2) = derivEnC(aim, bim, cim, r1, m_yc, m_j1, m_yc) - (dc2f1* h12 + dc2f2 * h22) * norm;
            jt(pos + 1, 3) = 0; // dh12
            jt(pos + 1, 4) = 0; // dh21
            jt(pos + 1, 5) = 0; // dh31
            jt(pos + 1, 6) = -(fx2)*norm; // dh12
            jt(pos + 1, 7) = -(fy2)*norm; // dh22
            jt(pos + 1, 8) = -norm; // dh32
            jt(pos + 1, 9) = (h12 * fx2 + h22 * fy2 + h32) * norm * norm * fx2; // dh13 note: (-1) * (-1) = +1
            jt(pos + 1, 10) = (h12 * fx2 + h22 * fy2 + h32) * norm * norm * fy2; // dh23 note: (-1) * (-1) = +1
        }
//     private:
    public:
        int m_idx[SIZEINDEXES];
        double m_xc, m_yc, m_norm, m_epsilon;
        double m_i1, m_j1, m_i2, m_j2;
};


#endif
