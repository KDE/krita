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


Q_UINT8 matchColors(const QColor & c, const QColor & c2, Q_UINT8 fuzziness);

bool isReddish(int h);
bool isYellowish(int h);
bool isGreenish(int h);
bool isCyanish(int h);
bool isBlueish(int h);
bool isMagentaish(int h);
bool isHighlight(int v);
bool isMidTone(int v);
bool isShadow(int v);

#endif // _KIS_UTILITIES_H_
