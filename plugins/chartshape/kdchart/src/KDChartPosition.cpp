/* -*- Mode: C++ -*-
   KDChart - a multi-platform charting engine
   */

/****************************************************************************
 ** Copyright (C) 2001-2007 Klarälvdalens Datakonsult AB.  All rights reserved.
 **
 ** This file is part of the KD Chart library.
 **
 ** This file may be used under the terms of the GNU General Public
 ** License versions 2.0 or 3.0 as published by the Free Software
 ** Foundation and appearing in the files LICENSE.GPL2 and LICENSE.GPL3
 ** included in the packaging of this file.  Alternatively you may (at
 ** your option) use any later version of the GNU General Public
 ** License if such license has been publicly approved by
 ** Klarälvdalens Datakonsult AB (or its successors, if any).
 ** 
 ** This file is provided "AS IS" with NO WARRANTY OF ANY KIND,
 ** INCLUDING THE WARRANTIES OF DESIGN, MERCHANTABILITY AND FITNESS FOR
 ** A PARTICULAR PURPOSE. Klarälvdalens Datakonsult AB reserves all rights
 ** not expressly granted herein.
 ** 
 ** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 ** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 **
 **********************************************************************/

#include <KDChartPosition.h>
#include <KDChartEnums.h>

#include <QString>
#include <QStringList>
#include <QList>
#include <QByteArray>

#include <KDABLibFakes>

#include <cassert>

using namespace KDChart;

namespace {
/**
 * \internal
 * Static strings, to be translated in printable()
 */
static const char * staticPositionNames[] = {
    QT_TRANSLATE_NOOP("Position","Unknown Position"),
    QT_TRANSLATE_NOOP("Position","Center"),
    QT_TRANSLATE_NOOP("Position","NorthWest"),
    QT_TRANSLATE_NOOP("Position","North"),
    QT_TRANSLATE_NOOP("Position","NorthEast"),
    QT_TRANSLATE_NOOP("Position","East"),
    QT_TRANSLATE_NOOP("Position","SouthEast"),
    QT_TRANSLATE_NOOP("Position","South"),
    QT_TRANSLATE_NOOP("Position","SouthWest"),
    QT_TRANSLATE_NOOP("Position","West"),
    QT_TRANSLATE_NOOP("Position","Floating")
};


/**
 * \internal
 * One value for unknown positions, and nine values for predefined positions.
 */
static Position staticPositionUnknown   = Position( KDChartEnums::PositionUnknown );
static Position staticPositionCenter    = Position( KDChartEnums::PositionCenter );
static Position staticPositionNorthWest = Position( KDChartEnums::PositionNorthWest );
static Position staticPositionNorth     = Position( KDChartEnums::PositionNorth );
static Position staticPositionNorthEast = Position( KDChartEnums::PositionNorthEast );
static Position staticPositionEast      = Position( KDChartEnums::PositionEast );
static Position staticPositionSouthEast = Position( KDChartEnums::PositionSouthEast );
static Position staticPositionSouth     = Position( KDChartEnums::PositionSouth );
static Position staticPositionSouthWest = Position( KDChartEnums::PositionSouthWest );
static Position staticPositionWest      = Position( KDChartEnums::PositionWest );
static Position staticPositionFloating  = Position( KDChartEnums::PositionFloating );

static int maxPositionValue = 10;

} // anon namespace

const Position& Position::Unknown   = staticPositionUnknown;
const Position& Position::Center    = staticPositionCenter;
const Position& Position::NorthWest = staticPositionNorthWest;
const Position& Position::North     = staticPositionNorth;
const Position& Position::NorthEast = staticPositionNorthEast;
const Position& Position::East      = staticPositionEast;
const Position& Position::SouthEast = staticPositionSouthEast;
const Position& Position::South     = staticPositionSouth;
const Position& Position::SouthWest = staticPositionSouthWest;
const Position& Position::West      = staticPositionWest;
const Position& Position::Floating  = staticPositionFloating;


/**
 * Default constructor. Creates a new Position, defaulting it to Position::Unknown.
 */
Position::Position()
    : m_value( KDChartEnums::PositionUnknown )
{

}

Position::Position( int value )
    : m_value( value )
{
    assert( 0 <= value ); assert( value <= maxPositionValue );
}

/**
 * Constructor. Creates a new Position, defaulting it to the respective value.
 *
 * Valid values ranging from zero (unknown value) to 10.
 * If invalid value is passed, a Position::Unknown is created.
 *
 * \note Normally there is no need to call this constructor, but you would
 * rather use one of the nine pre-defined, static values, e.g. like this:
 * \verbatim
 * const KDChart::Position myPosition = KDChart::Position::NorthEast;
 * \endverbatim
 */
Position::Position( KDChartEnums::PositionValue value )
    : m_value( value )
{

}

/**
 * Returns an integer value corresponding to this Position.
 */
KDChartEnums::PositionValue Position::value() const
{
    return static_cast<KDChartEnums::PositionValue>( m_value );
}

bool Position::isUnknown() const
{
    return  m_value == Position::Unknown.value();
}

bool Position::isWestSide() const
{
    return  m_value == Position::SouthWest.value() ||
            m_value == Position::West.value() ||
            m_value == Position::NorthWest.value();
}
bool Position::isNorthSide() const
{
    return  m_value == Position::NorthWest.value() ||
            m_value == Position::North.value() ||
            m_value == Position::NorthEast.value();
}
bool Position::isEastSide() const
{
    return  m_value == Position::NorthEast.value() ||
            m_value == Position::East.value() ||
            m_value == Position::SouthEast.value();
}
bool Position::isSouthSide() const
{
    return  m_value == Position::SouthWest.value() ||
            m_value == Position::South.value() ||
            m_value == Position::SouthEast.value();
}

bool Position::isCorner() const
{
    return  m_value == Position::NorthWest.value() ||
            m_value == Position::NorthEast.value() ||
            m_value == Position::SouthEast.value() ||
            m_value == Position::SouthWest.value();
}
bool Position::isPole() const
{
    return  m_value == Position::North.value() ||
        m_value == Position::South.value();
}

bool Position::isFloating() const
{
    return  m_value == Position::Floating.value();
}

/**
 * Returns a non-translated string in English language, corresponding to this Position.
 */
const char * Position::name() const
{
    return staticPositionNames[m_value];
}

/**
 * Returns a translated string, corresponding to this Position.
 */
QString Position::printableName() const
{
    return tr(staticPositionNames[m_value]);
}


/**
 * \brief Returns a list of all string, corresponding to
 * the pre-defined positions.
 *
 * \param options if set to \c ExcludeCenter, the returned list
 * does not contain the Center position.
 */
QList<QByteArray> Position::names( Options options )
{
    QList<QByteArray> list;
    const int start = ( options & IncludeCenter   ) ? 1 : 2;
    const int end   = ( options & IncludeFloating ) ? maxPositionValue : maxPositionValue-1;
    for( int i=start; i<=end; ++i)
        list.append( staticPositionNames[i] );
    return list;
}

/**
 * \brief Returns a list of all translated string, corresponding to
 * the pre-defined positions.
 *
 * \param options if set to \c ExcludeCenter, the returned list
 * does not contain the Center position.
 */
QStringList Position::printableNames( Options options )
{
    QStringList list;
    const int start = ( options & IncludeCenter   ) ? 1 : 2;
    const int end   = ( options & IncludeFloating ) ? maxPositionValue : maxPositionValue-1;
    for( int i=start; i<=end; ++i)
        list.append( Position(i).printableName() );
    return list;
}

Position Position::fromName(const char * name)
{
    for( int i=1; i<=maxPositionValue; ++i)
        if ( !qstricmp( name, staticPositionNames[i] ) )
            return Position(i);
    return Position(0);
}

Position Position::fromName( const QByteArray & name ) {
    return fromName( name.data() );
}

bool Position::operator==( const Position& r ) const
{
    return ( value() == r.value() );
}


bool Position::operator==( int value_ ) const
{
    return ( value() == value_ );
}


#if !defined(QT_NO_DEBUG_STREAM)
QDebug operator<<(QDebug dbg, const KDChart::Position& p )
{
    dbg << "KDChart::Position("
	<< p.name() << ")";
    return dbg;
}
#endif /* QT_NO_DEBUG_STREAM */
