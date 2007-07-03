/* This file is part of the KDE project
   Made by Emanuele Tamponi (emanuele@valinor.it)
   Copyright (C) 2007 Emanuele Tamponi

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#ifndef KIS_PAINTERLY_INFORMATION_H_
#define KIS_PAINTERLY_INFORMATION_H_

#include <QString>

#include <KoColor.h>

/**
* KisPainterlyInformation is a simple structure that keeps all information
* that a painterly paintop needs. Really, each "bristle" of a paintop could
* have a separate structure for its own.
* We provide here a brief description of these information. More can be found
* in the relative KisMask subclass and on http://wiki.koffice.org
* Please note that these information should be stored per-bristle, or per
* "paint unit", and represents the paint status in the paintop. Once the paintop
* actually paints something, then this information will be used to update the
* painterly overlays of the painted device.
*/
struct KisPainterlyBristleInformation {
    float Mixability; ///< how easily the paint mixes with the underlying color and returns to the bristle.
    float PigmentConcentration; ///< how much "pigment" is contained in the bristle (i.e., opacity)
    float PaintVolume; ///< how thick will be the track stroked by the bristle.
    float Reflectivity; ///< how much this paint is suscettible to reflection (high for watercolor, for example)
    float Viscosity; ///< how easily the paint spread.
    float Wetness; ///< how wet is the paint. This behaves on almost all other information.
};

#endif // KIS_PAINTERLY_INFORMATION_H_
