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
 * Boston, MA 02110-1301, USA.
*/

#include <qstring.h>

/**
 * Routines for converting to and from the ISO-8601 duration format
 * http://www.w3.org/TR/xmlschema-2/datatypes.html#duration
 *
 * This is not the ISO time format supported by Qt (HH:MM:SS),
 * it's PT1H15M12S for 1:15:12.
 *
 */

static QString minutesToISODuration( int mn )
{
    bool neg = mn < 0;
    // PT == period of time - see ISO8601
    QString str = QString::fromLatin1( "PT00H%1M00S" ).arg( QABS( mn ) );
    if ( neg )
        str.prepend( '-' );
    return str;
}

static QString daysToISODuration( int days )
{
    bool neg = days < 0;
    // P == period, time is ommitted - see ISO8601
    QString str = QString::fromLatin1( "P%1D" ).arg( QABS( days ) );
    if ( neg )
        str.prepend( '-' );
    return str;
}

static int ISODurationToMinutes( const QString& str )
{
    int idx = 0;
    const int len = str.length();
    bool neg = str[idx] == '-';
    if ( neg )
        ++idx;
    if ( idx < len && str[idx] == 'P' ) // should always be true
        ++idx;
    if ( idx < len && str[idx] == 'T' )
        ++idx;
    int minutes = 0;
    int currentNum = 0;
    while ( idx < len ) {
        if ( str[idx].isDigit() )
            currentNum = currentNum * 10 + str[idx].toLatin1() - '0';
        else {
            if ( str[idx] == 'D' )
                minutes += 24 * 60 * currentNum;
            else if ( str[idx] == 'H' )
                minutes += 60 * currentNum;
            else if ( str[idx] == 'M' )
                minutes += currentNum;
            currentNum = 0;
        }
        ++idx;
    }
    return neg ? -minutes : minutes;
}

static int ISODurationToDays( const QString& str )
{
    int idx = 0;
    const int len = str.length();
    bool neg = str[idx] == '-';
    if ( neg )
        ++idx;
    if ( idx < len && str[idx] == 'P' ) // should always be true
        ++idx;
    if ( idx < len && str[idx] == 'T' )
        ++idx;
    int days = 0;
    int currentNum = 0;
    while ( idx < len ) {
        if ( str[idx].isDigit() )
            currentNum = currentNum * 10 + str[idx].toLatin1() - '0';
        else {
            if ( str[idx] == 'Y' )
                days += 365 * currentNum;
            else if ( str[idx] == 'M' )
                days += 30 * currentNum; // approx
            else if ( str[idx] == 'D' )
                days += currentNum;
            currentNum = 0;
        }
        ++idx;
    }
    return neg ? -days : days;
}

