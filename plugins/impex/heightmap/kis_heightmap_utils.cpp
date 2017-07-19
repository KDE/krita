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

#include "kis_heightmap_utils.h"

#include <KoColorModelStandardIds.h>
#include <QByteArray>

KoID KisHeightmapUtils::mimeTypeToKoID(const QByteArray& mimeType)
{
    if (mimeType == "image/x-r8") {
        return Integer8BitsColorDepthID;
    }
    else if (mimeType == "image/x-r16") {
        return Integer16BitsColorDepthID;
    }
    else if (mimeType == "image/x-r32") {
        return Float32BitsColorDepthID;
    }
    return KoID();
}
