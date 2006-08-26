/*
 *  This file is part of the KDE project
 *
 *  Copyright (c) 2006 Cyrille Berger <cberger@cberger.net>
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

#include "krs_wavelet.h"

#include <klocale.h>

#include <kis_math_toolbox.h>

using namespace Kross::KritaCore;

Wavelet::Wavelet(KisMathToolbox::KisWavelet* kwl)
    : QObject(), m_wavelet(kwl)
{
    setObjectName("KritaWavelet");
    m_numCoeff = m_wavelet->size*m_wavelet->size*m_wavelet->depth;
}


Wavelet::~Wavelet()
{
}

double Wavelet::getNCoeff(uint index)
{
    quint32 n = index;
    if( n > m_numCoeff)
    {
        kWarning() << i18n("An error has occured in %1",QString("getNCoeff")) + '\n' + i18n("Index out of bound") << endl;
        return 0.0;
    }
    return *(m_wavelet->coeffs + n );
}

void Wavelet::setNCoeff(uint index, double value)
{
    quint32 n = index;
    if( n > m_numCoeff)
    {
        kWarning() << i18n("An error has occured in %1",QString("setNCoeff")) + '\n' + i18n("Index out of bound") << endl;
        return;
    }
    *(m_wavelet->coeffs + n ) = value;
}

double Wavelet::getXYCoeff(uint x, uint y)
{
    quint32 _x = x;
    quint32 _y = y;
    if( _x > m_wavelet->size && _y > m_wavelet->size)
    {
        kWarning() << i18n("An error has occured in %1",QString("getXYCoeff")) + '\n' + i18n("Index out of bound") << endl;
        return 0.0;
    }
    return *( m_wavelet->coeffs  + (_x + _y * m_wavelet->size ) * m_wavelet->depth );
}

void Wavelet::setXYCoeff(uint x, uint y, double value)
{
    quint32 _x = x;
    quint32 _y = y;
    if( _x > m_wavelet->size && _y > m_wavelet->size)
    {
        kWarning() << i18n("An error has occured in %1",QString("setXYCoeff")) + '\n' + i18n("Index out of bound") << endl;
        return;
    }
    *(m_wavelet->coeffs + (_x + _y * m_wavelet->size ) * m_wavelet->depth ) = value;
}

uint Wavelet::getDepth()
{
    return m_wavelet->depth;
}

uint Wavelet::getSize()
{
    return m_wavelet->size;
}

uint Wavelet::getNumCoeffs()
{
    return m_numCoeff;
}

#include "krs_wavelet.moc"
