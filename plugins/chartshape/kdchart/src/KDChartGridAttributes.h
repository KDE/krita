/* -*- Mode: C++ -*-
   KDChart - a multi-platform charting engine
   */

/****************************************************************************
 ** Copyright (C) 2005-2007 Klarälvdalens Datakonsult AB.  All rights reserved.
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

#ifndef KDCHARTGRIDATTRIBUTES_H
#define KDCHARTGRIDATTRIBUTES_H

#include <QMetaType>
#include "KDChartGlobal.h"
#include "KDChartEnums.h"

class QPen;

namespace KDChart {

/**
  * @brief A set of attributes controlling the appearance of grids
  */
class KDCHART_EXPORT GridAttributes
{
public:
    GridAttributes();
    GridAttributes( const GridAttributes& );
    GridAttributes &operator= ( const GridAttributes& );

    ~GridAttributes();

    void setGridVisible( bool visible );
    bool isGridVisible() const;


    void setGridStepWidth( qreal stepWidth=0.0 );
    qreal gridStepWidth() const;

    void setGridSubStepWidth(  qreal subStepWidth=0.0 );
    qreal gridSubStepWidth() const;

    /**
     * Specify which granularity sequence is to be used to find a matching
     * grid granularity.
     *
     * See details explained at KDChartEnums::GranularitySequence.
     *
     * You might also want to use setAdjustBoundsToGrid for fine-tuning the
     * start/end value.
     *
     * \sa setAdjustBoundsToGrid, GranularitySequence
     */
    void setGridGranularitySequence( KDChartEnums::GranularitySequence sequence );
    KDChartEnums::GranularitySequence gridGranularitySequence() const;

    /**
     * By default visible bounds of the data area are adjusted to match
     * a main grid line.
     * If you set the respective adjust flag to false the bound will
     * not start at a grid line's value but it will be the exact value
     * of the data range set.
     *
     * \sa CartesianCoordinatePlane::setHorizontalRange
     * \sa CartesianCoordinatePlane::setVerticalRange
     */
    void setAdjustBoundsToGrid( bool adjustLower, bool adjustUpper );
    bool adjustLowerBoundToGrid() const;
    bool adjustUpperBoundToGrid() const;


    void setGridPen( const QPen & pen );
    QPen gridPen() const;


    void setSubGridVisible( bool visible );
    bool isSubGridVisible() const;

    void setSubGridPen( const QPen & pen );
    QPen subGridPen() const;


    void setZeroLinePen( const QPen & pen );
    QPen zeroLinePen() const;

    bool operator==( const GridAttributes& ) const;
    inline bool operator!=( const GridAttributes& other ) const { return !operator==(other); }

private:
    KDCHART_DECLARE_PRIVATE_BASE_VALUE( GridAttributes )
}; // End of class GridAttributes

}

#if !defined(QT_NO_DEBUG_STREAM)
KDCHART_EXPORT QDebug operator<<(QDebug, const KDChart::GridAttributes& );
#endif /* QT_NO_DEBUG_STREAM */

KDCHART_DECLARE_SWAP_SPECIALISATION( KDChart::GridAttributes )
Q_DECLARE_METATYPE( KDChart::GridAttributes )
Q_DECLARE_TYPEINFO( KDChart::GridAttributes, Q_MOVABLE_TYPE );


#endif // KDCHARTGRIDATTRIBUTES_H
