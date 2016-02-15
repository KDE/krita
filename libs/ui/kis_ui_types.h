/*
 *  Copyright (c) 2010, Cyrille Berger <cberger@cberger.net>
 *  Copyright (c) 2010, Dmitry Kazakov <dimula73@gmail.com>
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

#ifndef KIS_UI_TYPES_H_
#define KIS_UI_TYPES_H_

template<class T>
class KisWeakSharedPtr;
template<class T>
class KisSharedPtr;

class KisPrescaledProjection;
typedef KisSharedPtr<KisPrescaledProjection> KisPrescaledProjectionSP;

class KisUpdateInfo;
class KisPPUpdateInfo;
typedef KisSharedPtr<KisUpdateInfo> KisUpdateInfoSP;
typedef KisSharedPtr<KisPPUpdateInfo> KisPPUpdateInfoSP;

#endif
