/****************************************************************************
 ** Copyright (C) 2007 Klarälvdalens Datakonsult AB.  All rights reserved.
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

#ifndef KDCHARTBARATTRIBUTES_H
#define KDCHARTBARATTRIBUTES_H

#include <QMetaType>
#include "KDChartGlobal.h"

namespace KDChart {

/**
  * @brief Set of attributes for changing the appearance of bar charts
  */
class KDCHART_EXPORT BarAttributes
{
public:
    BarAttributes();
    BarAttributes( const BarAttributes& );
    BarAttributes &operator= ( const BarAttributes& );

    ~BarAttributes();

    void setFixedDataValueGap( qreal gap );
    qreal fixedDataValueGap() const;

    void setUseFixedDataValueGap( bool gapIsFixed );
    bool useFixedDataValueGap() const;

    void setFixedValueBlockGap( qreal gap );
    qreal fixedValueBlockGap() const;

    void setUseFixedValueBlockGap( bool gapIsFixed );
    bool useFixedValueBlockGap() const;

    void setFixedBarWidth( qreal width );
    qreal fixedBarWidth() const;

    void setUseFixedBarWidth( bool useFixedBarWidth );
    bool useFixedBarWidth() const;

    void setGroupGapFactor ( qreal gapFactor );
    qreal groupGapFactor() const;

    void setBarGapFactor( qreal gapFactor );
    qreal barGapFactor() const;

    void setDrawSolidExcessArrows( bool solidArrows );
    bool drawSolidExcessArrows() const;

    bool operator==( const BarAttributes& ) const;
    inline bool operator!=( const BarAttributes& other ) const { return !operator==(other); }

private:
    class Private;
    Private * _d;
    Private * d_func() { return _d; }
    const Private * d_func() const { return _d; }
}; // End of class BarAttributes

}

Q_DECLARE_METATYPE( KDChart::BarAttributes )

#endif // KDCHARTBARATTRIBUTES_H
