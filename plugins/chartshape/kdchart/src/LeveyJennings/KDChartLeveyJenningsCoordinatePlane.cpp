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

#include "KDChartLeveyJenningsCoordinatePlane.h"
#include "KDChartLeveyJenningsCoordinatePlane_p.h"

#include <QtDebug>
#include <QPainter>

#include "KDChartPaintContext.h"
#include "KDChartPainterSaver_p.h"
#include "KDChartCartesianAxis.h"
#include "KDChartLeveyJenningsDiagram.h"

using namespace KDChart;

#define d d_func()

LeveyJenningsCoordinatePlane::Private::Private()
    : CartesianCoordinatePlane::Private()
{
}

LeveyJenningsCoordinatePlane::LeveyJenningsCoordinatePlane( Chart* parent )
    : CartesianCoordinatePlane( new Private(), parent )
{
}

LeveyJenningsCoordinatePlane::~LeveyJenningsCoordinatePlane()
{
}

void LeveyJenningsCoordinatePlane::init()
{
}

void LeveyJenningsCoordinatePlane::addDiagram( AbstractDiagram* diagram )
{
    Q_ASSERT_X ( dynamic_cast<LeveyJenningsDiagram*>( diagram ),
                 "LeveyJenningsCoordinatePlane::addDiagram", "Only Levey Jennings "
                 "diagrams can be added to a ternary coordinate plane!" );
    CartesianCoordinatePlane::addDiagram ( diagram );
}

LeveyJenningsGrid* LeveyJenningsCoordinatePlane::grid() const
{
    LeveyJenningsGrid* leveyJenningsGrid = static_cast<LeveyJenningsGrid*>( d->grid );
    Q_ASSERT( dynamic_cast<LeveyJenningsGrid*>( d->grid ) );
    return leveyJenningsGrid;
}

LeveyJenningsGridAttributes LeveyJenningsCoordinatePlane::gridAttributes() const
{
    return d->gridAttributes;
}

void LeveyJenningsCoordinatePlane::setGridAttributes( const LeveyJenningsGridAttributes& attr )
{
    d->gridAttributes = attr;
}

const QPointF LeveyJenningsCoordinatePlane::translateBack( const QPointF& screenPoint ) const
{
    return CartesianCoordinatePlane::translateBack( screenPoint );
}

#undef d
