/* This file is part of the KDE project
   Copyright (C) 2006 Laurent Montel <montel@kde.org>
   Copyright (C) 2008 Jan Hambrecht <jaham@gmx.net>

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

#include "KoGuidesData.h"
#include "KoViewConverter.h"

#include <QtGui/QPainter>

class KoGuidesData::Private
{
public:
    Private() : showGuideLines(true), guidesColor( Qt::lightGray ) {}
    /// list of positions of horizontal guide lines
    QList<double> horzGuideLines;
    /// list of positions of vertical guide lines
    QList<double> vertGuideLines;
    bool showGuideLines;
    QColor guidesColor;
};

KoGuidesData::KoGuidesData()
 : d( new Private() )
{
}

KoGuidesData::~KoGuidesData()
{
    delete d;
}

void KoGuidesData::setHorizontalGuideLines( const QList<double> &lines )
{
    d->horzGuideLines = lines;
}

void KoGuidesData::setVerticalGuideLines( const QList<double> &lines )
{
    d->vertGuideLines = lines;
}

void KoGuidesData::setGuideLines( const QList<double> &horizontalLines, const QList<double> &verticalLines)
{
    d->horzGuideLines = horizontalLines;
    d->vertGuideLines = verticalLines;
}

void KoGuidesData::addGuideLine( Qt::Orientation o, double pos )
{
    if ( o == Qt::Horizontal )
    {
        d->horzGuideLines.append( pos );
    }
    else
    {
        d->vertGuideLines.append( pos );
    }
}

bool KoGuidesData::showGuideLines() const 
{ 
  return d->showGuideLines; 
}

void KoGuidesData::setShowGuideLines( bool show )
{
  d->showGuideLines=show;
}

QList<double> KoGuidesData::horizontalGuideLines() const
{
    return d->horzGuideLines;
}

QList<double> KoGuidesData::verticalGuideLines() const
{
    return d->vertGuideLines;
}

void KoGuidesData::paintGuides(QPainter &painter, const KoViewConverter &converter, const QRectF &area) const
{
    if( ! showGuideLines() )
        return;

    painter.setPen( d->guidesColor );
    foreach( double guide, d->horzGuideLines )
    {
        if( guide < area.top() || guide > area.bottom() )
            continue;
        painter.drawLine( converter.documentToView( QPointF( area.left(), guide ) ),
                          converter.documentToView( QPointF( area.right(), guide ) ) );
    }
    foreach( double guide, d->vertGuideLines )
    {
        if( guide < area.left() || guide > area.right() )
            continue;
        painter.drawLine( converter.documentToView( QPointF( guide, area.top() ) ),
                          converter.documentToView( QPointF( guide, area.bottom() ) ) );
    }
}

void KoGuidesData::setGuidesColor( const QColor &color )
{
    d->guidesColor = color;
}

QColor KoGuidesData::guidesColor() const
{
    return d->guidesColor;
}
