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

#ifndef KDCHARTLEVEYJENNINGSGRIDATTRIBUTES_H
#define KDCHARTLEVEYJENNINGSGRIDATTRIBUTES_H

#include <QMetaType>
#include "../KDChartGlobal.h"
#include "../KDChartEnums.h"

class QPen;

namespace KDChart {

/**
  * @brief A set of attributes controlling the appearance of grids
  */
class KDCHART_EXPORT LeveyJenningsGridAttributes
{
public:
    LeveyJenningsGridAttributes();
    LeveyJenningsGridAttributes( const LeveyJenningsGridAttributes& );
    LeveyJenningsGridAttributes &operator= ( const LeveyJenningsGridAttributes& );

    ~LeveyJenningsGridAttributes();

    enum GridType
    {
        Expected,
        Calculated
    };

    enum Range
    {
        NormalRange,
        CriticalRange,
        OutOfRange
    };

    void setGridVisible( GridType type, bool visible );
    bool isGridVisible( GridType type ) const;

    void setGridPen( GridType type, const QPen& pen );
    QPen gridPen( GridType type ) const;

    void setRangeBrush( Range range, const QBrush& brush );
    QBrush rangeBrush( Range range ) const;

    bool operator==( const LeveyJenningsGridAttributes& ) const;
    inline bool operator!=( const LeveyJenningsGridAttributes& other ) const { return !operator==(other); }

private:
    KDCHART_DECLARE_PRIVATE_BASE_VALUE( LeveyJenningsGridAttributes )
}; // End of class GridAttributes

}

KDCHART_DECLARE_SWAP_SPECIALISATION( KDChart::LeveyJenningsGridAttributes )
Q_DECLARE_METATYPE( KDChart::LeveyJenningsGridAttributes )
Q_DECLARE_TYPEINFO( KDChart::LeveyJenningsGridAttributes, Q_MOVABLE_TYPE );


#endif // KDCHARTLEVEYJENNINGSGRIDATTRIBUTES_H
