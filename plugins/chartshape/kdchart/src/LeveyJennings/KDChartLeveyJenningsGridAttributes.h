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
