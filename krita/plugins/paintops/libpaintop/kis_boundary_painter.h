/*
 *  Copyright (c) 2005 Adrian Page <adrian@pagenet.plus.com>
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
#ifndef _KIS_BOUNDARY_PAINTER_H_
#define _KIS_BOUNDARY_PAINTER_H_

#include <krita_export.h>
//Added by qt3to4:
#include <QPixmap>

#include <kis_types.h>

class KisBoundary;
class QPainter;
class KoViewConverter;

class PAINTOP_EXPORT KisBoundaryPainter
{
public:
    static void paint(const KisBoundary* boundary, KisImageSP image, QPainter& painter, const KoViewConverter &converter);
};

#endif // _KIS_BOUNDARY_PAINTER_H_

