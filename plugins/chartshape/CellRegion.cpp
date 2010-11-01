/* This file is part of the KDE project

   Copyright 2008 Johannes Simon <johannes.simon@gmail.com>
   Copyright 2008 Inge Wallin     <inge@lysator.liu.se>

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


// Own
#include "CellRegion.h"

// C
#include <cmath>

// Qt
#include <QPoint>
#include <QRect>
#include <QVector>
#include <QDebug>
#include <QStringList>

// KDE
#include <kdebug.h>

// KChart
#include "TableSource.h"


using std::pow;
using namespace KChart;

static QString columnName( uint column );
//static int rangeCharToInt( char c );

/**
 * Makes sure that quotes are added if name contains spaces or special
 * characters. May also be used to escape certain characters if needed.
 */
static QString formatTableName( QString name )
{
    static const QList<QChar> specialChars =
        QList<QChar>() << ' ' << '\t' << '-' << '\'';

    bool containsSpecialChars = false;
    foreach( QChar c, specialChars )
        containsSpecialChars = containsSpecialChars || name.contains( c );

    if( containsSpecialChars )
        name.prepend( '\'' ).append( '\'' );

    return name;
}

/**
 * Reverts any operation done by formatTableName(), so that ideally
 * unformatTableName( formatTableName( name ) ) == name
 */
static QString unformatTableName( QString name )
{
    if ( name.startsWith( '\'' ) && name.endsWith( '\'' ) ) {
        name.remove( 0, 1 );
        name.remove( name.length() - 1, 1 );
    }

    return name;
}

class CellRegion::Private
{
public:
    Private();
    ~Private();

    QString pointToString( const QPoint &point ) const;

    // These are actually one-dimensional, but can have different
    // orientations (hor / vert).
    QVector<QRect> rects;

    QRect          boundingRect;
    // NOTE: Don't forget to extend operator=() if you add new members

    /// Table this region is in (name/model pair provided by TableSource)
    Table *table;
};


CellRegion::Private::Private()
{
    table = 0;
}

CellRegion::Private::~Private()
{
}


// ================================================================
//                         Class CellRegion


CellRegion::CellRegion()
    : d( new Private() )
{
}

CellRegion::CellRegion( const CellRegion &region )
    : d( new Private() )
{
    // Use operator=();
    *this = region;
}

CellRegion::CellRegion( TableSource *source, const QString& regions )
    : d( new Private() )
{
    // A dollar sign before a part of the address means that this part
    // is absolute. This is irrelevant for us, however, thus we can remove
    // all occurences of '$', and handle relative and absolute addresses in
    // the same way.
    // See ODF specs $8.3.1 "Referencing Table Cells"
    QStringList regionsList = regions.split( " ", QString::SkipEmptyParts );
    Q_FOREACH( const QString& region, regionsList ) {
      QString searchStr = QString( region ).remove( "$" );
      QRegExp regEx;

      QStringList regionList = searchStr.split( ";" );
      Q_FOREACH( const QString &region, regionList ) {
          const bool isPoint = !region.contains( ':' );
          if ( isPoint )
              regEx = QRegExp( "(|.*\\.)([A-Z]+)([0-9]+)" );
          else // support range-notations like Sheet1.D2:Sheet1.F2 Sheet1.D2:F2 D2:F2
              regEx = QRegExp ( "(|.*\\.)([A-Z]+)([0-9]+)\\:(|.*\\.)([A-Z]+)([0-9]+)" );

          // Check if region string is valid (e.g. not empty)
          if ( regEx.indexIn( region ) >= 0 ) {
              // It is possible for a cell-range-address as defined in ODF to contain
              // refernces to cells of more than one sheet. This, however, we ignore
              // here. We do not support more than one table in a cell region.
              // Also we do not support regions spanned over different sheets. For us
              // everything is either on no sheet or on the same sheet.
              QString sheetName = regEx.cap( 1 );
              if ( sheetName.endsWith( "." ) )
                  sheetName = sheetName.left( sheetName.length() - 1 );
              // TODO: Support for multiple tables in one region
              d->table = source->get( unformatTableName( sheetName ) );

              QPoint topLeft( rangeStringToInt( regEx.cap(2) ), regEx.cap(3).toInt() );
              if ( isPoint ) {
                  d->rects.append( QRect( topLeft, QSize( 1, 1 ) ) );
              } else {
                  QPoint bottomRight( rangeStringToInt( regEx.cap(5) ), regEx.cap(6).toInt() );
                  d->rects.append( QRect( topLeft, bottomRight ) );
              }
          }
      }
    }
}

CellRegion::CellRegion( Table *table, const QPoint &point )
    : d( new Private() )
{
    d->table = table;
    add( point );
}

CellRegion::CellRegion( Table *table, const QRect &rect )
    : d( new Private() )
{
    d->table = table;
    add( rect );
}

CellRegion::CellRegion( Table *table, const QVector<QRect> &rects )
    : d( new Private() )
{
    d->table = table;
    foreach( const QRect& rect, rects )
        add( rect );
}

CellRegion::CellRegion( Table *table )
    : d( new Private() )
{
    d->table = table;
}

CellRegion::~CellRegion()
{
    delete d;
}


CellRegion& CellRegion::operator = ( const CellRegion& region )
{
    d->rects        = region.d->rects;
    d->boundingRect = region.d->boundingRect;
    d->table = region.d->table;

    return *this;
}

bool CellRegion::operator == ( const CellRegion &other ) const
{
    return d->rects == other.d->rects;
}


Table *CellRegion::table() const
{
    return d->table;
}

QVector<QRect> CellRegion::rects() const
{
    return d->rects;
}

int CellRegion::rectCount() const
{
    return d->rects.size();
}

QString CellRegion::sheetName() const
{
    return d->table->name();
}

bool CellRegion::isValid() const
{
    return d->rects.size() > 0 && d->table ;
}

QString CellRegion::Private::pointToString( const QPoint &point ) const
{
    QString result;

    result.append( '$' + columnName( point.x() ) );
    result.append( '$' + QString::number( point.y() ) );

    return result;
}

QString CellRegion::toString() const
{
    if ( !isValid() )
        return QString();

    QString result;
    for ( int i = 0; i < d->rects.count(); ++i ) {
        const QRect range = d->rects[i];
        // Top-left corner
        if ( table() )
            result.append( '$' + formatTableName( table()->name() ) + '.' );
        result.append( d->pointToString( range.topLeft() ) );

        // If it is not a point, append rect's bottom-right corner
        if ( range.topLeft() != range.bottomRight() ) {
            result.append( ':' );
            result.append( d->pointToString( range.bottomRight() ) );
        }

        // Separate ranges by a comma, except for the last one
        if ( i < d->rects.count() - 1 )
            result.append( ';' );
    }
    return result;
}


bool CellRegion::contains( const QPoint &point, bool proper ) const
{
    foreach ( const QRect &rect, d->rects ) {
        if ( rect.contains( point, proper ) )
            return true;
    }

    return false;
}

bool CellRegion::contains( const QRect &rect, bool proper ) const
{
    foreach ( const QRect &r, d->rects ) {
        if ( r.contains( rect, proper ) )
            return true;
    }

    return false;
}

bool CellRegion::intersects( const CellRegion &other ) const
{
    // If both regions lie within only one table and these tables
    // are different, they trivially do not intersect.
    if ( table() && other.table() &&
         table() != other.table() )
        return false;

    foreach ( const QRect &r, d->rects ) {
        foreach( const QRect &_r, other.d->rects ) {
            if ( r.intersects( _r ) )
                return true;
        }
    }

    return false;
}

CellRegion CellRegion::intersected( const QRect &rect ) const
{
    CellRegion intersections;

    foreach ( const QRect &r, d->rects ) {
        if ( r.intersects( rect ) )
            intersections.add( r.intersected( rect ) );
    }

    return intersections;
}

Qt::Orientation CellRegion::orientation() const
{
    foreach ( const QRect &rect, d->rects ) {
        if ( rect.width() > 1 )
                return Qt::Horizontal;
        if ( rect.height() > 1 )
                return Qt::Vertical;
    }

    // Default if region is only one cell
    return Qt::Vertical;
}

int CellRegion::cellCount() const
{
    int count = 0;
    if ( orientation() == Qt::Horizontal ) {
        foreach ( const QRect &rect, d->rects )
            count += rect.width();
    }
    else {
        foreach( const QRect &rect, d->rects )
            count += rect.height();
    }

    return count;
}

void CellRegion::add( const CellRegion &other )
{
    add( other.rects() );
}

void CellRegion::add( const QPoint &point )
{
    add( QRect( point, QSize( 1, 1 ) ) );
}

void CellRegion::add( const QRect &rect )
{
// These checks are obsolete, a CellRegion can be used otherwise as well
#if 0
    if ( !rect.isValid() ) {
        qWarning() << "CellRegion::add() Attempt to add invalid rectangle";
        qWarning() << "CellRegion::add():" << rect;
        return;
    }

    if ( rect.width() > 1 && rect.height() > 1 ) {
        qWarning() << "CellRegion::add() Attempt to add rectangle with height AND width > 1";
        qWarning() << "CellRegion::add():" << rect;
        return;
    }
#endif

    d->rects.append( rect );
    d->boundingRect |= rect;
}

void CellRegion::add( const QVector<QRect> &rects )
{
    foreach ( const QRect &rect, rects )
        add( rect );
}

QRect CellRegion::boundingRect() const
{
    return d->boundingRect;
}

bool CellRegion::hasPointAtIndex( int index ) const
{
    return pointAtIndex( index ) != QPoint( -1, -1 );
}

QPoint CellRegion::pointAtIndex( int index ) const
{
    // sum of all previous rectangle indices
    int i = 0;

    foreach ( const QRect &rect, d->rects ) {
        // Rectangle is horizontal
        if ( rect.width() > 1 ) {
            // Found it!
            // Index refers to point in current rectangle
            if ( i + rect.width() > index ) {
                // Local index of point in this rectangle
                int j = index - i;
                return QPoint( rect.x() + j, rect.y() );
            }

            // add number of indices in current rectangle to total index count
            i += rect.width();
        }
        else {
            // Found it!
            // Index refers to point in current rectangle
            if ( i + rect.height() > index ) {
                // Local index of point in this rectangle
                int j = index - i;
                return QPoint( rect.x(), rect.y() + j );
            }

            // add number of indices in current rectangle to total index count
            i += rect.height();
        }
    }

    // Invalid index!
    return QPoint( -1, -1 );
}

int CellRegion::indexAtPoint( const QPoint &point ) const
{
    int indicesLeftToPoint = 0;
    bool found = false;

    foreach ( const QRect &rect, d->rects ) {
        if ( !rect.contains( point ) ) {
            indicesLeftToPoint += rect.width() > 1 ? rect.width() : rect.height();
            continue;
        }

        found = true;
        if ( rect.width() > 1 )
            indicesLeftToPoint += point.x() - rect.topLeft().x();
        else
            indicesLeftToPoint += point.y() - rect.topLeft().y();
    }

    return found ? indicesLeftToPoint : -1;
}

#if 0 // Unused?
static int rangeCharToInt( char c )
{
    return (c >= 'A' && c <= 'Z') ? (c - 'A' + 1) : -1;
}

static int rangeStringToInt( const QString &string )
{
    int result = 0;
    const int size = string.size();
    for ( int i = 0; i < size; i++ ) {
        //kDebug(350001) << "---" << float( rangeCharToInt( string[i].toAscii() ) * pow( 10.0, ( size - i - 1 ) ) );
        result += rangeCharToInt( string[i].toAscii() ) * pow( 10.0, ( size - i - 1 ) );
    }
    //kDebug(350001) << "+++++ result=" << result;
    return result;
}

static QString rangeIntToString( int i )
{
    QString tmp = QString::number( i );
    for( int j = 0; j < tmp.size(); j++ ) {
        tmp[j] = 'A' + tmp[j].toAscii() - '1';
    }

    //kDebug(350001) << "tmp=" << tmp;
    return tmp;
}
#endif

int CellRegion::rangeCharToInt( char c )
{
    return (c >= 'A' && c <= 'Z') ? (c - 'A' + 1) : -1;
}

int CellRegion::rangeStringToInt( const QString &string )
{
    int result = 0;
    const int size = string.size();
    for ( int i = 0; i < size; i++ ) {
        result += rangeCharToInt( string[i].toAscii() ) * pow( 10.0, ( size - i - 1 ) );
    }

    return result;
}

QString CellRegion::rangeIntToString( int i )
{
    QString tmp = QString::number( i );
    for( int j = 0; j < tmp.size(); j++ ) {
        tmp[j] = 'A' + tmp[j].toAscii() - '1';
    }

    return tmp;
}

// Return the symbolic name of any column.
static QString columnName( uint column )
{
    if ( column < 1 || column > 32767 )
        return QString( "@@@" );

    QString   str;
    unsigned  digits = 1;
    unsigned  offset = 0;

    column--;

    for( unsigned limit = 26; column >= limit + offset; limit *= 26, ++digits )
        offset += limit;

    for( unsigned col = column - offset; digits; --digits, col /= 26 )
        str.prepend( QChar( 'A' + ( col % 26 ) ) );

    return str;
}
