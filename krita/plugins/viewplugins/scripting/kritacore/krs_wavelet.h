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

#include <QObject>

#include <kis_math_toolbox.h>

namespace Kross {

namespace KritaCore {

/**
 * Fast Wavelet object.
 */
class Wavelet : public QObject
{
        Q_OBJECT
    public:
        Wavelet(KisMathToolbox::KisWavelet* wavelet);
        ~Wavelet();

    public slots:

        /**
         * Return the value of the Nth coefficient.
         * The function takes one argument :
         * - the index of the coefficient
         */
        double getNCoeff(uint index);

        /**
         * Set the value of the Nth coefficient.
         * The function takes two arguments :
         * - the index of the coefficient
         * - the new value of the coefficient
         */
        void setNCoeff(uint index, double value);

        /**
         * Return the value of a coefficient
         * The function takes two arguments :
         *  - x
         *  - y
         */
        double getXYCoeff(uint x, uint y);

        /**
         * Set the value of a coefficient.
         * The function takes three arguments :
         * - x
         * - y
         * - the new value of the coefficient
         */
        void setXYCoeff(uint x, uint y, double value);

        /**
         * Return the depth of the layer.
         */
        uint getDepth();

        /**
         * Return the size of the wavelet (size = width = height).
         */
        uint getSize();

        /**
         * Return the number of coefficients in this wavelet (= size * size * depth).
         */
        uint getNumCoeffs();

    public:
        KisMathToolbox::KisWavelet* wavelet() { return m_wavelet; }

    private:
        KisMathToolbox::KisWavelet* m_wavelet;
        uint m_numCoeff;
};

}

}

#endif
