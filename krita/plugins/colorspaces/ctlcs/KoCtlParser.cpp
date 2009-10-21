/*
 *  Copyright (c) 2009 Cyrille Berger <cberger@cberger.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#include "KoCtlParser.h"

#include<QString>

#include <kis_debug.h>
#include <KoColorModelStandardIds.h>
#include <klocale.h>

KoID KoCtlParser::generateDepthID(const QString& depth, const QString& type)
{
    if (type == "integer") {
        if (depth == "8")
            return Integer8BitsColorDepthID;
        else if (depth == "16")
            return Integer16BitsColorDepthID;
    } else if (type == "float") {
        if (depth == "16")
            return Float16BitsColorDepthID;
        else if (depth == "32")
            return Float32BitsColorDepthID;
        else if (depth.isEmpty())
            return KoID("F", i18n("Float"));
    }
    dbgPlugins << "Invalid type";
    return KoID("", "");
}

