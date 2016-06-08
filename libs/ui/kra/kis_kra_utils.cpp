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

#include "kis_kra_utils.h"

QString KRA::flagsToString(const QBitArray& flags, int size, char trueToken, char falseToken, bool defaultTrue)
{
    size = (size < 0) ? flags.count() : size;
    
    QString string(size, defaultTrue ? trueToken : falseToken);
    
    for(int i=0; i<qMin(size, flags.count()); ++i)
        string[i] = flags[i] ? trueToken : falseToken;
    
    return string;
}

QBitArray KRA::stringToFlags(const QString& string, int size, char token, bool defaultTrue)
{
    size = (size < 0) ? string.length() : size;
    
    QBitArray flags(size, defaultTrue);
    
    for(int i=0; i<qMin(size, string.length()); ++i)
        flags[i] = (string[i] == token) ? !defaultTrue : defaultTrue;
    
    return flags;
}
