/* This file is part of the KDE project
   Copyright (C) 2005 David Faure <faure@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include <QString>
#include <assert.h>

#include "../IsoDuration.h"

void testMinutes()
{
    int minutes = 145;
    QString str = minutesToISODuration( minutes );
    int result = ISODurationToMinutes( str );
    qDebug( "%d minutes -> %s -> %d", minutes, str.latin1(), result );
    assert( result == minutes );
}

void testNegativeMinutes()
{
    int minutes = -15;
    QString str = minutesToISODuration( minutes );
    int result = ISODurationToMinutes( str );
    qDebug( "%d minutes -> %s -> %d", minutes, str.latin1(), result );
    assert( result == minutes );
}

void testDays()
{
    int days = 14;
    QString str = daysToISODuration( days );
    int result = ISODurationToDays( str );
    qDebug( "%d days -> %s -> %d", days, str.latin1(), result );
    assert( result == days );
}

void testNegativeDays()
{
    int days = -14;
    QString str = daysToISODuration( days );
    int result = ISODurationToDays( str );
    qDebug( "%d days -> %s -> %d", days, str.latin1(), result );
    assert( result == days );
}

int main ( int argc, char ** argv )
{
    testMinutes();
    testDays();
    testNegativeMinutes();
    testNegativeDays();
    return 0;
}
