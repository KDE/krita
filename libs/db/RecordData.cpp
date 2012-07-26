/* This file is part of the KDE project
   Copyright (C) 2002   Lucijan Busch <lucijan@gmx.at>
   Copyright (C) 2003   Daniel Molkentin <molkentin@kde.org>
   Copyright (C) 2003-2007 Jarosław Staniek <staniek@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.

   Original Author:  Till Busch <till@bux.at>
   Original Project: buX (www.bux.at)
*/

#include "RecordData.h"
#include "global.h"
#include <KDebug>

using namespace KexiDB;

QString RecordData::debugString() const
{
    QString s(QString("RECORD DATA (%1 columns)").arg(size()));
    int i = 0;
    foreach(const QVariant& value, *this) {
        i++;
        s.append(QString::number(i) + ":[" + value.typeName() + "]" + value.toString() + " ");
    }
    return s;
}

void RecordData::debug() const
{
    KexiDBDbg << debugString();
}
