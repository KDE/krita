/*
 *  This file is part of the KDE project
 *
 *  Copyright (c) 2005 Cyrille Berger <cberger@cberger.net>
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

#ifndef KIS_BASIC_MATH_TOOLBOX_H
#define KIS_BASIC_MATH_TOOLBOX_H

#include "kis_math_toolbox.h"

/**
 * This class implement KisMathToolbox for most colorspaces, only colorspaces with "angular"
 * channels need to reimplement the functions
 */
class KisBasicMathToolbox : public KisMathToolbox
{
    public:
        KisBasicMathToolbox();
        ~KisBasicMathToolbox();
    public:
        virtual KisWavelet* fastWaveletTransformation(KisPaintDeviceSP src, const QRect&,  KisWavelet* buff = 0);
        virtual void fastWaveletUntransformation(KisPaintDeviceSP dst, const QRect&, KisWavelet* wav, KisWavelet* buff = 0);
    private:
        void wavetrans(KisWavelet* wav, KisWavelet* buff, uint halfsize);
        void waveuntrans(KisWavelet* wav, KisWavelet* buff, uint halfsize);

};

#endif
