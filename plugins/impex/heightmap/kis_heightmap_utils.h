/*
 *  Copyright (c) 2017 Victor Wåhlström <victor.wahlstrom@initiali.se>
 *
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; either version 2.1 of the License, or
 *  (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef _KIS_HEIGHTMAP_UTILS_H_
#define _KIS_HEIGHTMAP_UTILS_H_

#include <KoID.h>

namespace KisHeightmapUtils
{
KoID mimeTypeToKoID(const class QByteArray& mimeType);
}

#endif // _KIS_HEIGHTMAP_UTILS_H_
