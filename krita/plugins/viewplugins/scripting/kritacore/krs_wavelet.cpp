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

namespace Kross {

namespace KritaCore {

Wavelet::Wavelet(KisMathToolbox::KisWavelet* kwl)
    : Kross::Api::Class<Wavelet>("KritaWavelet"), m_wavelet(kwl)
{
    addFunction("getNCoeff", &Wavelet::getNCoeff);
    addFunction("setNCoeff", &Wavelet::setNCoeff);
    addFunction("getXYCoeff", &Wavelet::getXYCoeff);
    addFunction("setXYCoeff", &Wavelet::setXYCoeff);
    addFunction("getDepth", &Wavelet::getDepth);
    addFunction("getSize", &Wavelet::getSize);
    addFunction("getNumCoeffs", &Wavelet::getNumCoeffs);
    m_numCoeff = m_wavelet->size*m_wavelet->size*m_wavelet->depth;
}


Wavelet::~Wavelet()
{
}


Kross::Api::Object::Ptr Wavelet::getNCoeff(Kross::Api::List::Ptr args)
{
    quint32 n = Kross::Api::Variant::toUInt(args->item(0));
    if( n > m_numCoeff)
    {
        throw Kross::Api::Exception::Ptr( new Kross::Api::Exception( i18n("An error has occured in %1",QString("getNCoeff")) + '\n' + i18n("Index out of bound") ) );
    }
    return Kross::Api::Object::Ptr(new Kross::Api::Variant(*(m_wavelet->coeffs + n )));
}

Kross::Api::Object::Ptr Wavelet::setNCoeff(Kross::Api::List::Ptr args)
{
    quint32 n = Kross::Api::Variant::toUInt(args->item(0));
    double v = Kross::Api::Variant::toDouble(args->item(1));
    if( n > m_numCoeff)
    {
        throw Kross::Api::Exception::Ptr( new Kross::Api::Exception( i18n("An error has occured in %1",QString("setNCoeff")) + '\n' + i18n("Index out of bound") ) );
    }
    *(m_wavelet->coeffs + n ) = v;
    return Kross::Api::Object::Ptr(0);
}

Kross::Api::Object::Ptr Wavelet::getXYCoeff(Kross::Api::List::Ptr args)
{
    quint32 x = Kross::Api::Variant::toUInt(args->item(0));
    quint32 y = Kross::Api::Variant::toUInt(args->item(1));
    if( x > m_wavelet->size && y > m_wavelet->size)
    {
        throw Kross::Api::Exception::Ptr( new Kross::Api::Exception( i18n("An error has occured in %1",QString("getXYCoeff")) + '\n' + i18n("Index out of bound") ) );
    }
    return Kross::Api::Object::Ptr(new Kross::Api::Variant(*(m_wavelet->coeffs  + (x + y * m_wavelet->size ) * m_wavelet->depth )));
}

Kross::Api::Object::Ptr Wavelet::setXYCoeff(Kross::Api::List::Ptr args)
{
    quint32 x = Kross::Api::Variant::toUInt(args->item(0));
    quint32 y = Kross::Api::Variant::toUInt(args->item(1));
    double v = Kross::Api::Variant::toDouble(args->item(2));
    if( x > m_wavelet->size && y > m_wavelet->size)
    {
        throw Kross::Api::Exception::Ptr( new Kross::Api::Exception( i18n("An error has occured in %1",QString("setXYCoeff")) + '\n' + i18n("Index out of bound") ));
    }
    *(m_wavelet->coeffs + (x + y * m_wavelet->size ) * m_wavelet->depth ) = v;
    return Kross::Api::Object::Ptr(0);
}

Kross::Api::Object::Ptr Wavelet::getDepth(Kross::Api::List::Ptr /*args*/)
{
    return Kross::Api::Object::Ptr(new Kross::Api::Variant(m_wavelet->depth));
}

Kross::Api::Object::Ptr Wavelet::getSize(Kross::Api::List::Ptr)
{
    return Kross::Api::Object::Ptr(new Kross::Api::Variant(m_wavelet->size));
}

Kross::Api::Object::Ptr Wavelet::getNumCoeffs(Kross::Api::List::Ptr)
{
    return Kross::Api::Object::Ptr(new Kross::Api::Variant(m_numCoeff));
}


}

}
