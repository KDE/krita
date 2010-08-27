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

#ifndef KDCHARTSTOCKBARATTRIBUTES_H
#define KDCHARTSTOCKBARATTRIBUTES_H

#include <QMetaType>
#include "KDChartGlobal.h"

namespace KDChart {

/**
  * @brief Attributes to customize the appearance of a column in a stock chart
  */
class KDCHART_EXPORT StockBarAttributes
{
public:
    StockBarAttributes();
    StockBarAttributes( const StockBarAttributes& );
    StockBarAttributes &operator= ( const StockBarAttributes& );

    ~StockBarAttributes();

    void setCandlestickWidth( qreal width );
    qreal candlestickWidth() const;

    void setTickLength( qreal length );
    qreal tickLength() const;

    bool operator==( const StockBarAttributes& ) const;
    inline bool operator!=( const StockBarAttributes& other ) const { return !operator==(other); }

private:
    class Private;
    Private * _d;
    Private * d_func() { return _d; }
    const Private * d_func() const { return _d; }
};

}

Q_DECLARE_METATYPE( KDChart::StockBarAttributes )

#endif // KDCHARTSTOCKBARATTRIBUTES_H
