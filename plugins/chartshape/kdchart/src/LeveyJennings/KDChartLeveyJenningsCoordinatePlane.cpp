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
