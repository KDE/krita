/*
 *  Copyright (c) 2005 Cyrille Berger <cberger@cberger.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Library General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "krs_color.h"

namespace Kross {

namespace KritaCore {

Color::Color ( int x, int y, int z, QColor::Spec colorSpec )
    : Kross::Api::Class<Color>("KritaColor"), m_color(x,y,z,colorSpec)
{
}

Color::Color()
 : Kross::Api::Class<Color>("KritaColor")
{
}

Color::~Color()
{
}


}

}
