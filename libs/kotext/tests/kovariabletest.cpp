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

#include <qtest_kde.h>

#include <QString>
#include <QObject>

#include "kovariabletest.h"
#include "../IsoDuration.h"

QTEST_KDEMAIN( KoVariableTest, NoGUI )

void KoVariableTest::testMinutes()
{
    int minutes = 145;
    QString str = minutesToISODuration( minutes );
    int result = ISODurationToMinutes( str );
    qDebug( "%d minutes -> %s -> %d", minutes, qPrintable(str), result );
    QCOMPARE( result, minutes );
}

void KoVariableTest::testNegativeMinutes()
{
    int minutes = -15;
    QString str = minutesToISODuration( minutes );
    int result = ISODurationToMinutes( str );
    qDebug( "%d minutes -> %s -> %d", minutes, qPrintable(str), result );
    QCOMPARE( result, minutes );
}

void KoVariableTest::testDays()
{
    int days = 14;
    QString str = daysToISODuration( days );
    int result = ISODurationToDays( str );
    qDebug( "%d days -> %s -> %d", days, qPrintable(str), result );
    QCOMPARE( result, days );
}

void KoVariableTest::testNegativeDays()
{
    int days = -14;
    QString str = daysToISODuration( days );
    int result = ISODurationToDays( str );
    qDebug( "%d days -> %s -> %d", days, qPrintable(str), result );
    QCOMPARE( result, days );
}

#include "kovariabletest.moc"
