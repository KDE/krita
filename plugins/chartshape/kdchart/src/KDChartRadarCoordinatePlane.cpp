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

#include "KDChartRadarCoordinatePlane.h"
#include "KDChartRadarCoordinatePlane_p.h"


using namespace KDChart;

#define d d_func()

KDChart::RadarCoordinatePlane::RadarCoordinatePlane ( Chart* parent ) :
    PolarCoordinatePlane( new Private(), parent )
{
}
KDChart::RadarCoordinatePlane::~RadarCoordinatePlane()
{
}

void KDChart::RadarCoordinatePlane::setTextAttributes(const KDChart::TextAttributes& attr)
{
    d->textAttributes = attr;
}

const KDChart::TextAttributes RadarCoordinatePlane::textAttributes() const
{
    return d->textAttributes;

}

void RadarCoordinatePlane::init()
{
    // this bloc left empty intentionally
}
