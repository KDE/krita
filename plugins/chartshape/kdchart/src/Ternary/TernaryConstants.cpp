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

#include "TernaryConstants.h"

#include <cmath>

extern const double Sqrt3 = sqrt( 3.0 );
extern const double TriangleWidth = 1.0;
extern const double TriangleHeight = 0.5 * Sqrt3;
extern const QPointF TriangleTop( 0.5, TriangleHeight );
extern const QPointF TriangleBottomLeft( 0.0, 0.0 );
extern const QPointF TriangleBottomRight( 1.0, 0.0 );
extern const QPointF AxisVector_C_A( TriangleTop - TriangleBottomRight );
extern const QPointF Norm_C_A( -AxisVector_C_A.y(), AxisVector_C_A.x() );
extern const QPointF AxisVector_B_A( TriangleTop );
extern const QPointF Norm_B_A( -AxisVector_B_A.y(), AxisVector_B_A.x() );
extern const QPointF AxisVector_B_C( TriangleBottomRight );
extern const QPointF Norm_B_C( -AxisVector_B_C.y(), AxisVector_B_C.x() );

extern const double RelMarkerLength = 0.03 * TriangleWidth;
extern const QPointF FullMarkerDistanceBC( RelMarkerLength * Norm_B_C );
extern const QPointF FullMarkerDistanceAC( -RelMarkerLength * Norm_C_A );
extern const QPointF FullMarkerDistanceBA( RelMarkerLength * Norm_B_A );

