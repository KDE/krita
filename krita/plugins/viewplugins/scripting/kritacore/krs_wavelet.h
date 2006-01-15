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

#ifndef KROSS_KRITACOREKRS_WAVELET_H
#define KROSS_KRITACOREKRS_WAVELET_H

#include <api/class.h>

#include <kis_math_toolbox.h>

namespace Kross {

namespace KritaCore {

/**
	@author Cyrille Berger <cberger@cberger.net>
*/
class Wavelet : public Kross::Api::Class<Wavelet>
{
    public:
        Wavelet(KisMathToolbox::KisWavelet* wavelet);
        ~Wavelet();
    private:
        Kross::Api::Object::Ptr getNCoeff(Kross::Api::List::Ptr);
        Kross::Api::Object::Ptr setNCoeff(Kross::Api::List::Ptr);
        Kross::Api::Object::Ptr getXYCoeff(Kross::Api::List::Ptr);
        Kross::Api::Object::Ptr setXYCoeff(Kross::Api::List::Ptr);
        Kross::Api::Object::Ptr getDepth(Kross::Api::List::Ptr);
        Kross::Api::Object::Ptr getSize(Kross::Api::List::Ptr);
        Kross::Api::Object::Ptr getNumCoeffs(Kross::Api::List::Ptr);
    public:
        KisMathToolbox::KisWavelet* wavelet() { return m_wavelet; }
    private:
        KisMathToolbox::KisWavelet* m_wavelet;
        int m_numCoeff;
};

}

}

#endif
