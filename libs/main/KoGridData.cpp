/* This file is part of the KDE project
   Copyright (C) 2006 Laurent Montel <montel@kde.org>
   Copyright (C) 2007 Thomas Zander <zander@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#include "KoGridData.h"
#include "KoUnit.h"
#include "KoViewConverter.h"

#include <QPainter>
#include <QRectF>

class KoGridData::Private {
public:
    Private()
     :snapToGrid(false),
      showGrid(false),
      gridX(MM_TO_POINT(5.0)),
      gridY(MM_TO_POINT(5.0)),
      gridColor(Qt::lightGray)
    {
    }

    bool snapToGrid;
    bool showGrid;
    double gridX, gridY;
    QColor gridColor;
};

KoGridData::KoGridData()
    : d(new Private())
{
}

KoGridData::~KoGridData() {
    delete d;
}

double KoGridData::gridX() const
{
   return d->gridX;
}

double KoGridData::gridY() const
{
  return d->gridY;
}

void KoGridData::setGrid(double x, double y)
{
   d->gridX = x;
   d->gridY = y;
}

bool KoGridData::snapToGrid() const
{
   return d->snapToGrid;
}

void KoGridData::setSnapToGrid(bool on)
{
   d->snapToGrid = on;
}

QColor KoGridData::gridColor() const
{
  return d->gridColor;
}

void KoGridData::setGridColor( const QColor & color)
{
  d->gridColor=color;
}

bool KoGridData::showGrid() const
{
  return d->showGrid;
}

void KoGridData::setShowGrid ( bool showGrid )
{
  d->showGrid = showGrid;
}

void KoGridData::paintGrid(QPainter &painter, const KoViewConverter &converter, const QRectF &area) const
{
    if( ! showGrid() )
        return;

    painter.setPen( gridColor() );

    double x = 0.0;
    do {
        painter.drawLine( converter.documentToView( QPointF( x, area.top() ) ),
                          converter.documentToView( QPointF( x, area.bottom() ) ) );
        x += gridX();
    } while( x <= area.right() );

    x = - gridX();
    while( x >= area.left() )
    {
        painter.drawLine( converter.documentToView( QPointF( x, area.top() ) ),
                          converter.documentToView( QPointF( x, area.bottom() ) ) );
        x -= gridX();
    };

    double y = 0.0;
    do {
        painter.drawLine( converter.documentToView( QPointF( area.left(), y ) ),
                          converter.documentToView( QPointF( area.right(), y ) ) );
        y += gridY();
    } while( y <= area.bottom() );

    y = - gridY();
    while( y >= area.top() )
    {
        painter.drawLine( converter.documentToView( QPointF( area.left(), y ) ),
                          converter.documentToView( QPointF( area.right(), y ) ) );
        y -= gridY();
    };
}

