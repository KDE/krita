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

#include "kis_illuminant_profile.h"

#include <KoColorProfile.h>

#include <QDataStream>
#include <QFile>
#include <QString>
#include <QDebug>

#include <cmath>

double **allocateMatrix(int m, int n)
{
    double **matrix;
    matrix = new double*[m];
    for (int i = 0; i < m; i++)
        matrix[i] = new double[n];
    return matrix;
}

double *allocateVector(int n)
{
    return new double[n];
}

void freeMatrix(int m,double **matrix)
{
    for (int i = 0; i < m; i++)
        delete [] matrix[i];
    delete [] matrix;
}

void freeVector(double *vector)
{
    delete [] vector;
}

void applyMatrix(int m,int n, double **T,double *b, double *d)
{
    for (int i = 0; i < m; i++) {
        d[i] = 0;
        for (int j = 0; j < n; j++)
            d[i] += T[i][j] * b[j];
    }
}

double polyval(int n, const double *P, double x)
{
    double y = 0;
    for (int i = 0; i < n; i++)
        y += P[n-1-i]*pow(x,i);
    return y;
}

double phi(double r)
{
    return 2.0*r / pow(1.0-r,2);
}

double psi(double r)
{
    return pow(1.0-r,2) / (2.0*r);
}

double invphi(double y)
{
    return ( 1.0 + y - sqrt(2.0 * y + 1.0) ) / y;
}

KisIlluminantProfile::KisIlluminantProfile(const QString &fileName)
    : KoColorProfile(fileName),
      m_wl(-1), m_T(0), m_L(0), m_red(0), m_green(0), m_blue(0), m_refvec(0),
      Ca(0), Cs(0), m_illuminant(""), m_valid(false)
{

}

KisIlluminantProfile::KisIlluminantProfile(const KisIlluminantProfile &copy)
    : KoColorProfile(copy.fileName()),
      m_wl(-1), m_T(0), m_L(0), m_red(0), m_green(0), m_blue(0), m_refvec(0),
      Ca(0), Cs(0), m_illuminant(""), m_valid(false)
{
    if (copy.valid()) {
        m_valid = copy.m_valid;
        m_wl = copy.m_wl;
        m_illuminant = copy.m_illuminant;
        setName(copy.name());
        m_T = allocateMatrix(3,m_wl);
        for (int i = 0; i < m_wl; i++)
            for (int j = 0; j < 3; j++)
                m_T[j][i] = copy.m_T[j][i];
        m_L = allocateMatrix(1,m_wl);
        for (int i = 0; i < m_wl; i++)
            m_L[0][i] = copy.m_L[0][i];
        m_red   = allocateVector(m_wl);
        m_green = allocateVector(m_wl);
        m_blue  = allocateVector(m_wl);
        for (int i = 0; i < m_wl; i++) {
            m_red[i] = copy.m_red[i];
        }
        for (int i = 0; i < m_wl; i++) {
            m_green[i] = copy.m_green[i];
        }
        for (int i = 0; i < m_wl; i++) {
            m_blue[i] = copy.m_blue[i];
        }
        Np = copy.Np;
        Cla = copy.Cla;
        Nla = copy.Nla;
        Ca = allocateVector(Np);
        for (qint8 i = 0; i < Np; i++)
            Ca[i] = copy.Ca[i];
        Cls = copy.Cls;
        Nls = copy.Nls;
        Cs = allocateVector(Np);
        for (qint8 i = 0; i < Np; i++)
            Cs[i] = copy.Cs[i];
        Rh = copy.Rh;
    }
}

KisIlluminantProfile::~KisIlluminantProfile()
{
    reset();
}

bool KisIlluminantProfile::load()
{
    reset();

    if (fileName().isEmpty())
        return false;

    QFile file(fileName());
    if (!file.open(QIODevice::ReadOnly))
        return false;

    QDataStream data(&file);

    {
        qint8 letter;
        while (true) {
            data >> letter;
            if (!letter)
                break;
            m_illuminant += (char)letter;
        }
    }
    {
        qint8 tmp;
        data >> tmp;
        m_wl = (int)tmp;
        data >> tmp >> tmp >> tmp;
        m_T = allocateMatrix(3,m_wl);
        for (int i = 0; i < m_wl; i++)
            for (int j = 0; j < 3; j++)
                data.readRawData((char*)&m_T[j][i],8);
    }
    {
        m_L = allocateMatrix(1,m_wl);
        for (int i = 0; i < m_wl; i++)
            data.readRawData((char*)&m_L[0][i],8);
    }
    {
        m_red   = allocateVector(m_wl);
        m_green = allocateVector(m_wl);
        m_blue  = allocateVector(m_wl);
        for (int i = 0; i < m_wl; i++) {
            data.readRawData((char*)&m_red[i],8);
        }
        for (int i = 0; i < m_wl; i++) {
            data.readRawData((char*)&m_green[i],8);
        }
        for (int i = 0; i < m_wl; i++) {
            data.readRawData((char*)&m_blue[i],8);
        }
    }
    {
        qint8 tmp;
        data >> tmp;
        Np = (int)tmp-2;
        data.readRawData((char*)&Cla,8);
        data.readRawData((char*)&Nla,8);
        Ca = allocateVector(Np);
        for (qint8 i = 0; i < Np; i++)
            data.readRawData((char*)&Ca[i],8);
        data.readRawData((char*)&Cls,8);
        data.readRawData((char*)&Nls,8);
        Cs = allocateVector(Np);
        for (qint8 i = 0; i < Np; i++)
            data.readRawData((char*)&Cs[i],8);
        data.readRawData((char*)&Rh,8);
    }

    // Initialize the reflectance vector and channel converter
    m_refvec = allocateVector(m_wl);
    setName(QString("%1").arg(m_illuminant));
    m_valid = true;

    return true;
}

bool KisIlluminantProfile::save(const QString &fileName)
{
    if (!valid())
        return false;

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly))
        return false;
    
    QDataStream data(&file);

    for (int i = 0; i < m_illuminant.size(); i++)
        data << (qint8)m_illuminant[i].toAscii();
    data << (qint8)0;
    
    data << (qint8)m_wl << (qint8)m_wl << (qint8)m_wl << (qint8)m_wl;
    for (int i = 0; i < m_wl; i++)
        for (int j = 0; j < 3; j++)
            data.writeRawData((char*)&m_T[j][i],8);

    for (int i = 0; i < m_wl; i++)
        data.writeRawData((char*)&m_L[0][i],8);
    
    for (int i = 0; i < m_wl; i++)
        data.writeRawData((char*)&m_red[i],8);
    for (int i = 0; i < m_wl; i++)
        data.writeRawData((char*)&m_green[i],8);
    for (int i = 0; i < m_wl; i++)
        data.writeRawData((char*)&m_blue[i],8);

    data << (qint8)Np;
    data.writeRawData((char*)&Cla,8);
    data.writeRawData((char*)&Nla,8);
    for (qint8 i = 0; i < Np; i++)
        data.writeRawData((char*)&Ca[i],8);
    data.writeRawData((char*)&Cls,8);
    data.writeRawData((char*)&Nls,8);
    for (qint8 i = 0; i < Np; i++)
        data.writeRawData((char*)&Cs[i],8);
    data.writeRawData((char*)&Rh,8);

    return true;
}

void KisIlluminantProfile::fromRgb(const double *rgbvec, double *ksvec) const
{
    // TODO: add cache!

    rgbToReflectance(rgbvec);
    reflectanceToKS(ksvec);
}

void KisIlluminantProfile::toRgb(const double *ksvec, double *rgbvec) const
{
    KSToReflectance(ksvec);
    reflectanceToRgb(rgbvec);
}

double KisIlluminantProfile::falpha(double R, double L) const
{
    return fabs( Cla * pow(fabs(L+0.05),Nla) * polyval(Np,Ca,R) );
}

double KisIlluminantProfile::fsigma(double R, double L) const
{
    return fabs( Cls * pow(fabs(L+0.05),Nls) * polyval(Np,Cs,R) );
}

void KisIlluminantProfile::reflectanceToKS(double *ksvec) const
{
    double L;
    applyMatrix(1,m_wl, m_L,m_refvec, &L);
    
    for (int i = 0; i < m_wl; i++) {
        if (m_refvec[i] <= Rh) {
            ksvec[2*i+0] = falpha(m_refvec[i],L);
            ksvec[2*i+1] = ksvec[2*i+0] * phi(m_refvec[i]);
        } else {
            ksvec[2*i+1] = fsigma(m_refvec[i],L);
            ksvec[2*i+0] = ksvec[2*i+1] * psi(m_refvec[i]);
        }
    }
}

void KisIlluminantProfile::KSToReflectance(const double *ksvec) const
{
    for (int i = 0; i < m_wl; i++) {
        if (ksvec[2*i+0] <= 0)
            m_refvec[i] = 1;
        else if (ksvec[2*i+1] <= 0)
            m_refvec[i] = 0;
        else
            m_refvec[i] = invphi(ksvec[2*i+1]/ksvec[2*i+0]);
    }
}

void KisIlluminantProfile::rgbToReflectance(const double *rgbvec) const
{
    // Each reflectance is a linear combination of three base colors.
    for (int i = 0; i < m_wl; i++) {
        m_refvec[i] = rgbvec[0] * m_red[i] + rgbvec[1] * m_green[i] + rgbvec[2] * m_blue[i];
    }
}

void KisIlluminantProfile::reflectanceToRgb(double *rgbvec) const
{
    // Avoid calculation of black and white
    double sum = 0;
    for (int i = 0; i < m_wl; i++)
        sum += m_refvec[i];
    if (sum <= 0) {
        rgbvec[0] = rgbvec[1] = rgbvec[2] = 0;
        return;
    }
    if (sum >= m_wl) {
        rgbvec[0] = rgbvec[1] = rgbvec[2] = 1;
        return;
    }

    applyMatrix(3,m_wl, m_T,m_refvec, rgbvec);
    for (int i = 0; i < 3; i++) {
        if (rgbvec[i] < 0) rgbvec[i] = 0;
        if (rgbvec[i] > 1) rgbvec[i] = 1;
    }
}

void KisIlluminantProfile::reset()
{
    if (m_T)
        freeMatrix(3,m_T);
    if (m_L)
        freeMatrix(1,m_L);
    if (m_refvec)
        freeVector(m_refvec);
    if (m_red)
        freeVector(m_red);
    if (m_green)
        freeVector(m_green);
    if (m_blue)
        freeVector(m_blue);
    if (Ca)
        freeVector(Ca);
    if (Cs)
        freeVector(Cs);
    
    m_T = m_L = 0;
    m_refvec = m_red = m_green = m_blue = 0;
    Ca = Cs = 0;
    
    m_illuminant = "";
    m_wl = -1;
    
    m_valid = false;
}
