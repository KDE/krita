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

#include "KDChartLeveyJenningsDiagram.h"
#include "KDChartDataValueAttributes.h"

#include "KDChartLeveyJenningsDiagram_p.h"

using namespace KDChart;

LeveyJenningsDiagram::Private::Private( const Private& rhs )
    : LineDiagram::Private( rhs ),
      lotChangedPosition( rhs.lotChangedPosition ),
      fluidicsPackChangedPosition( rhs.fluidicsPackChangedPosition ),
      sensorChangedPosition( rhs.sensorChangedPosition ),
      fluidicsPackChanges( rhs.fluidicsPackChanges ),
      sensorChanges( rhs.sensorChanges ),
      scanLinePen( rhs.scanLinePen ),
      icons( rhs.icons ),
      expectedMeanValue( rhs.expectedMeanValue ),
      expectedStandardDeviation( rhs.expectedStandardDeviation )
{
}

void LeveyJenningsDiagram::Private::setYAxisRange() const
{
    CartesianCoordinatePlane* const plane = static_cast< CartesianCoordinatePlane* >( diagram->coordinatePlane() );
    if( plane == 0 )
        return;

    plane->setVerticalRange( QPair< qreal, qreal >( expectedMeanValue - 4 * expectedStandardDeviation, 
                                                    expectedMeanValue + 4 * expectedStandardDeviation ) );
}
