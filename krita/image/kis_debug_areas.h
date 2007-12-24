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


/**
 * In krita code, include kis_debug.h instead of kdebug.h. Then use one of the areas #defined here
 * instead of the raw numbers. Also, try not to check in code outside of tests that uses kDebug
 * without an area.
 */

#define DBG_AREA_IMAGE 41001
#define DBG_AREA_REGISTRY 40002
#define DBG_AREA_TOOLS 41003
#define DBG_AREA_CMS 41004
#define DBG_AREA_FILTERS 41005
#define DBG_AREA_PLUGINS 41006
#define DBG_AREA_UI 41007
#define DBG_AREA_FILE 41008
#define DBG_AREA_MATH 41009
#define DBG_AREA_RENDER 41010
#define DBG_AREA_SCRIPT 41011


#endif
