/* This file is part of the KDE project
 * Copyright 2011 (C) Silvio Heinrich <plassy@web.de>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */
#ifndef _KIS_KRA_UTILS_
#define _KIS_KRA_UTILS_

#include <QString>
#include <QBitArray>

namespace KRA {

QString   flagsToString(const QBitArray& flags, int size=-1, char trueToken='1', char falseToken='0', bool defaultTrue=true);
QBitArray stringToFlags(const QString& string, int size=-1, char token='0', bool defaultTrue=true);

}

#endif // _KIS_KRA_UTILS_
