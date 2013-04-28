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
