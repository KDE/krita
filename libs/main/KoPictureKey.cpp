
/* This file is part of the KDE project
   Copyright (c) 2001 Simon Hausmann <hausmann@kde.org>
   Copyright 2002 Nicolas GOUTTE <goutte@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#include <QDateTime>
#include <QFileInfo>
#include <qdom.h>

#include <kdebug.h>

#include "KoPictureKey.h"

static void resetDateTimeToEpoch(QDateTime& dt)
{
    // set the time point to 1970-01-01
    dt.setDate(QDate(1970,1,1));
    dt.setTime(QTime(0,0));
    // Note: we cannot use QDateTime;;setTime_t as it makes a local time correction! (### TODO: not true anymore with recent Qt versions)
}

KoPictureKey::KoPictureKey()
{
    resetDateTimeToEpoch(m_lastModified);
}

KoPictureKey::KoPictureKey( const QString &fn, const QDateTime &mod )
    : m_filename( fn ), m_lastModified( mod )
{
    if (!m_lastModified.isValid())
    {
        // As we have an invalid date, set the time point to 1970-01-01
        resetDateTimeToEpoch(m_lastModified);
    }
}

KoPictureKey::KoPictureKey( const QString &fn )
    : m_filename( fn )
{
    resetDateTimeToEpoch(m_lastModified);
}

KoPictureKey::KoPictureKey( const KoPictureKey &key )
    : m_filename( key.m_filename ), m_lastModified( key.m_lastModified )
{
}

KoPictureKey& KoPictureKey::operator=( const KoPictureKey &key )
{
    m_filename = key.m_filename;
    m_lastModified = key.m_lastModified;
    return *this;
}

bool KoPictureKey::operator==( const KoPictureKey &key ) const
{
    return ( key.m_filename == m_filename &&
             key.m_lastModified == m_lastModified );
}

bool KoPictureKey::operator<( const KoPictureKey &key ) const
{
    return key.toString() < toString();
}

void KoPictureKey::saveAttributes( QDomElement &elem ) const
{
    QDate date = m_lastModified.date();
    QTime time = m_lastModified.time();
    elem.setAttribute( "filename", m_filename );
    elem.setAttribute( "year", date.year() );
    elem.setAttribute( "month", date.month() );
    elem.setAttribute( "day", date.day() );
    elem.setAttribute( "hour", time.hour() );
    elem.setAttribute( "minute", time.minute() );
    elem.setAttribute( "second", time.second() );
    elem.setAttribute( "msec", time.msec() );
}

void KoPictureKey::loadAttributes( const QDomElement &elem )
{
    // Default date/time is the *nix epoch: 1970-01-01 00:00:00,000
    int year=1970, month=1, day=1;
    int hour=0, minute=0, second=0, msec=0; // We must initialize to zero, as not all compilers are C99-compliant

    if( elem.hasAttribute( "key" ) )
    {
         // Note: the old KWord format (up to 1.1-beta2) has no date/time
        m_filename=elem.attribute( "key" );
    }
    else
    {   
        // ### TODO: document which format is this?
        m_filename=elem.attribute( "filename" );
    }

    if( elem.hasAttribute( "year" ) )
        year=elem.attribute( "year" ).toInt();
    if( elem.hasAttribute( "month" ) )
        month=elem.attribute( "month" ).toInt();
    if( elem.hasAttribute( "day" ) )
        day=elem.attribute( "day" ).toInt();
    if( elem.hasAttribute( "hour" ) )
        hour=elem.attribute( "hour" ).toInt();
    if( elem.hasAttribute( "minute" ) )
        minute=elem.attribute( "minute" ).toInt();
    if( elem.hasAttribute( "second" ) )
        second=elem.attribute( "second" ).toInt();
    if( elem.hasAttribute( "msec" ) )
        msec=elem.attribute( "msec" ).toInt();

    m_lastModified.setDate( QDate( year, month, day ) );
    m_lastModified.setTime( QTime( hour, minute, second, msec ) );

    if (!m_lastModified.isValid())
    {
        // If the date/time is not valid, make it valid by force!
        kWarning(30003) << "Correcting invalid date/time: " << toString()  << " (in KoPictureKey::loadAttributes)" << endl;
        resetDateTimeToEpoch(m_lastModified);
    }
}

QString KoPictureKey::toString() const
{
    // We do not use the default QDateTime::toString has it does not show microseconds
    return QString::fromLatin1( "%1 %2" )
        .arg( m_filename, m_lastModified.toString("yyyy-MM-dd hh:mm:ss.zzz") );
}

void KoPictureKey::setKeyFromFile (const QString& filename)
{
    QFileInfo inf(filename);
    m_filename = filename;
    m_lastModified = inf.lastModified();
}
