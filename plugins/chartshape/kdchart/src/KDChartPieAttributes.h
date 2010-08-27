/****************************************************************************
** Copyright (C) 2001-2010 Klaralvdalens Datakonsult AB.  All rights reserved.
**
** This file is part of the KD Chart library.
**
** Licensees holding valid commercial KD Chart licenses may use this file in
** accordance with the KD Chart Commercial License Agreement provided with
** the Software.
**
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 and version 3 as published by the
** Free Software Foundation and appearing in the file LICENSE.GPL included.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** Contact info@kdab.com if any conditions of this licensing are not
** clear to you.
**
**********************************************************************/

#ifndef KDCHART_PIE_ATTRIBUTES_H
#define KDCHART_PIE_ATTRIBUTES_H

#include <QMetaType>
#include "KDChartAbstractThreeDAttributes.h"
#include "KDChartGlobal.h"

namespace KDChart {

/**
  * @brief A set of attributes controlling the appearance of pie charts
  */
class KDCHART_EXPORT PieAttributes
{
public:
    PieAttributes();
    PieAttributes( const PieAttributes& );
    PieAttributes &operator= ( const PieAttributes& );

    ~PieAttributes();

    /** \brief Enable or disable exploding the respective pie piece(s).
     *
     * The default explode factor is 10 percent; use setExplodeFactor
     * to specify a different factor.
     *
     * \note This is a convenience function: Calling setExplode( true )
     * does the same as calling setExplodeFactor( 0.1 ), and calling
     * setExplode( false ) does the same as calling setExplodeFactor( 0.0 ).
     *
     * \sa setExplodeFactor
     */
    void setExplode( bool explode );

    /** @return whether the respective pie piece(s) will be exploded.  */
    bool explode() const;

    /** Set the explode factor.
     * The explode factor is a qreal between 0 and 1, and is interpreted
     * as a percentage of the total available radius of the pie.
     *
     * \sa setExplode
     */
    void setExplodeFactor( qreal factor );

    /** @return the explode factor set by setExplode or by setExplodeFactor. */
    qreal explodeFactor() const;
    
    void setGapFactor( bool circular, qreal factor );
    qreal gapFactor( bool circular ) const;

    bool operator==( const PieAttributes& ) const;
    inline bool operator!=( const PieAttributes& other ) const { return !operator==(other); }

private:
    KDCHART_DECLARE_PRIVATE_BASE_VALUE( PieAttributes )
}; // End of class PieAttributes

}

#if !defined(QT_NO_DEBUG_STREAM)
KDCHART_EXPORT QDebug operator<<(QDebug, const KDChart::PieAttributes& );
#endif /* QT_NO_DEBUG_STREAM */


Q_DECLARE_METATYPE( KDChart::PieAttributes )
Q_DECLARE_TYPEINFO( KDChart::PieAttributes, Q_MOVABLE_TYPE );
KDCHART_DECLARE_SWAP_SPECIALISATION( KDChart::PieAttributes )

#endif // KDCHART_PIE_ATTRIBUTES_H
