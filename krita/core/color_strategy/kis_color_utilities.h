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
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#ifndef _KIS_UTILITIES_H_
#define _KIS_UTILITIES_H_



#include <qglobal.h>
#include <kdebug.h>
#include "koffice_export.h"

KRITACORE_EXPORT Q_UINT8 matchColors(const QColor & c, const QColor & c2, Q_UINT8 fuzziness);

KRITACORE_EXPORT bool isReddish(int h);
KRITACORE_EXPORT bool isYellowish(int h);
KRITACORE_EXPORT bool isGreenish(int h);
KRITACORE_EXPORT bool isCyanish(int h);
KRITACORE_EXPORT bool isBlueish(int h);
KRITACORE_EXPORT bool isMagentaish(int h);
KRITACORE_EXPORT bool isHighlight(int v);
KRITACORE_EXPORT bool isMidTone(int v);
KRITACORE_EXPORT bool isShadow(int v);

#endif // _KIS_UTILITIES_H_
