/*
 *  Copyright (c) 2004 Adrian Page <adrian@pagenet.plus.com>
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
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include <cmath>
#include <cfloat>

#include "kis_rect.h"

bool KisRect::isNull() const
{
	return width() < DBL_EPSILON || height() < DBL_EPSILON;
}

QRect KisRect::qRect() const
{
	return QRect(static_cast<int>(floor(m_x1)), static_cast<int>(floor(m_y1)), static_cast<int>(ceil(m_x2) - floor(m_x1)), static_cast<int>(ceil(m_y2) - floor(m_y1)));
}

