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

#include "KDChartAbstractPolarDiagram.h"
#include "KDChartAbstractPolarDiagram_p.h"

#include <KDABLibFakes>


using namespace KDChart;

AbstractPolarDiagram::Private::Private()
{
}

AbstractPolarDiagram::Private::~Private()
{
}

void AbstractPolarDiagram::init()
{
}

#define d d_func()

AbstractPolarDiagram::AbstractPolarDiagram (
    QWidget* parent, PolarCoordinatePlane* plane )
    : AbstractDiagram ( new Private(), parent, plane )
{
}


const PolarCoordinatePlane * AbstractPolarDiagram::polarCoordinatePlane() const
{
    return dynamic_cast<const PolarCoordinatePlane*>( coordinatePlane() );
}

int AbstractPolarDiagram::columnCount() const
{
    return static_cast<int>( numberOfValuesPerDataset() );
}

int AbstractPolarDiagram::rowCount() const
{
    return static_cast<int>( numberOfDatasets() );
}
