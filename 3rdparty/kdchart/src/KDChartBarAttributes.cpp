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

#include "KDChartBarAttributes.h"
#include <QtGlobal>

#include <KDABLibFakes>

#define d d_func()


using namespace KDChart;

class BarAttributes::Private
{
    friend class BarAttributes;
public:
    Private();

private:
    qreal datasetGap;
    bool useFixedDatasetGap;
    qreal valueBlockGap;
    bool useFixedValueBlockGap;
    qreal barWidth;
    bool useFixedBarWidth;
    bool drawSolidExcessArrows;
    qreal groupGapFactor;
    qreal barGapFactor;
};


BarAttributes::Private::Private()
    :datasetGap( 6 ),
    useFixedDatasetGap( false ),
    valueBlockGap( 24 ),
    useFixedValueBlockGap( false ),
    barWidth( -1 ),
    useFixedBarWidth( false ),
    drawSolidExcessArrows( false ),
    groupGapFactor( 1.0 ),
    barGapFactor( 0.5 )
{
}


BarAttributes::BarAttributes()
    : _d( new Private() )
{
}

BarAttributes::BarAttributes( const BarAttributes& r )
    : _d( new Private( *r.d ) )
{
}

BarAttributes& BarAttributes::operator= ( const BarAttributes& r )
{
    if( this == &r )
        return *this;

    *d = *r.d;

    return *this;
}

BarAttributes::~BarAttributes()
{
    delete _d; _d = 0;
}


bool BarAttributes::operator==( const BarAttributes& r ) const
{
    if( fixedDataValueGap() == r.fixedDataValueGap() &&
        useFixedDataValueGap() == r.useFixedDataValueGap() &&
        fixedValueBlockGap() == r.fixedValueBlockGap() &&
        useFixedValueBlockGap() == r.useFixedValueBlockGap() &&
        fixedBarWidth() == r.fixedBarWidth() &&
        useFixedBarWidth() == r.useFixedBarWidth() &&
        groupGapFactor() == r.groupGapFactor() &&
        barGapFactor() == r.barGapFactor() &&
        drawSolidExcessArrows() == r.drawSolidExcessArrows() )
        return true;
    else
        return false;
}


void BarAttributes::setFixedDataValueGap( qreal gap )
{
    d->datasetGap = gap;
}

qreal BarAttributes::fixedDataValueGap() const
{
    return d->datasetGap;
}

void BarAttributes::setUseFixedDataValueGap( bool gapIsFixed )
{
    d->useFixedDatasetGap = gapIsFixed;
}

bool BarAttributes::useFixedDataValueGap() const
{
    return d->useFixedDatasetGap;
}

void BarAttributes::setFixedValueBlockGap( qreal gap )
{
    d->valueBlockGap = gap;
}

qreal BarAttributes::fixedValueBlockGap() const
{
    return d->valueBlockGap;
}

void BarAttributes::setUseFixedValueBlockGap( bool gapIsFixed )
{
    d->useFixedValueBlockGap = gapIsFixed;
}

bool BarAttributes::useFixedValueBlockGap() const
{
    return d->useFixedValueBlockGap;
}

void BarAttributes::setFixedBarWidth( qreal width )
{
    d->barWidth = width;
}

qreal BarAttributes::fixedBarWidth() const
{

    return d->barWidth;
}

void BarAttributes::setUseFixedBarWidth( bool useFixedBarWidth )
{
    d->useFixedBarWidth = useFixedBarWidth;
}

bool BarAttributes::useFixedBarWidth() const
{
    return d->useFixedBarWidth;
}

void BarAttributes::setGroupGapFactor( qreal gapFactor )
{
    d->groupGapFactor = gapFactor;
}

qreal BarAttributes::groupGapFactor() const
{
    return d->groupGapFactor;
}

void BarAttributes::setBarGapFactor( qreal gapFactor )
{
    d->barGapFactor = gapFactor;
}

qreal BarAttributes::barGapFactor() const
{
    return d->barGapFactor;
}

void BarAttributes::setDrawSolidExcessArrows( bool solidArrows )
{
    d->drawSolidExcessArrows = solidArrows;
}

bool BarAttributes::drawSolidExcessArrows() const
{
    return d->drawSolidExcessArrows;
}

