/*
 *  SPDX-FileCopyrightText: 2017 Victor Wåhlström <victor.wahlstrom@initiali.se>
 *
 *  SPDX-License-Identifier: LGPL-2.1-or-later
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
