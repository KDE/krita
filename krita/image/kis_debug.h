/*
 *  Copyright (c) 2005 Boudewijn Rempt <boud@valdyas.org>
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
#ifndef KIS_DEBUG_AREAS_H_
#define KIS_DEBUG_AREAS_H_

#include <kdebug.h>


/**
 * In krita code, include kis_debug.h instead of kdebug.h. Then use one of the areas #defined here
 * instead of the raw numbers. Also, try not to check in code outside of tests that uses kDebug
 * without an area.
 */
#ifdef KRITA_RELEASE_MODE
    #define dbgKrita if (false) kDebug(41000) // For temporary debug lines, where you'd have used kDebug() before.
    #define dbgImage if (false) kDebug(41001)
    #define dbgRegistry if (false) kDebug(41002)
    #define dbgTools if (false) kDebug(41003)
    #define dbgTiles if (false) kDebug(41004)
    #define dbgFilters if (false) kDebug(41005)
    #define dbgPlugins if (false) kDebug(41006)
    #define dbgUI if (false) kDebug(41007)
    #define dbgFile if (false) kDebug(41008)
    #define dbgMath if (false) kDebug(41009)
    #define dbgRender if (false) kDebug(41010)
    #define dbgScript if (false) kDebug(41011)
#else
    #define dbgKrita kDebug(41000) // For temporary debug lines, where you'd have used kDebug() before.
    #define dbgImage kDebug(41001)
    #define dbgRegistry kDebug(41002)
    #define dbgTools kDebug(41003)
    #define dbgTiles kDebug(41004)
    #define dbgFilters kDebug(41005)
    #define dbgPlugins kDebug(41006)
    #define dbgUI kDebug(41007)
    #define dbgFile kDebug(41008)
    #define dbgMath kDebug(41009)
    #define dbgRender kDebug(41010)
    #define dbgScript kDebug(41011)
#endif

#endif
