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

KoGuidesData::KoGuidesData()
 :m_bShowGuideLines(true)
{
}

void KoGuidesData::setHorizontalGuideLines( const QList<double> &lines )
{
    m_hGuideLines = lines;
}

void KoGuidesData::setVerticalGuideLines( const QList<double> &lines )
{
    m_vGuideLines = lines;
}

void KoGuidesData::setGuideLines( const QList<double> &horizontalLines, const QList<double> &verticalLines)
{
    m_hGuideLines = horizontalLines;
    m_vGuideLines = verticalLines;
}

void KoGuidesData::addGuideLine( Qt::Orientation o, double pos )
{
    if ( o == Qt::Horizontal )
    {
        m_hGuideLines.append( pos );
    }
    else
    {
        m_vGuideLines.append( pos );
    }
}

bool KoGuidesData::showGuideLines() const 
{ 
  return m_bShowGuideLines; 
}

void KoGuidesData::setShowGuideLines( bool show )
{
  m_bShowGuideLines=show;
}

QList<double> KoGuidesData::horizontalGuideLines() const
{
    return m_hGuideLines;
}

QList<double> KoGuidesData::verticalGuideLines() const
{
    return m_vGuideLines;
}

void KoGuidesData::paintGuides(QPainter &painter, const KoViewConverter &converter, const QRectF &area) const
{
    painter.setPen( Qt::lightGray ); /// TODO: make member of guides data?
    foreach( double guide, m_hGuideLines )
    {
        if( guide < area.top() || guide > area.bottom() )
            continue;
        painter.drawLine( converter.documentToView( QPointF( area.left(), guide ) ),
                          converter.documentToView( QPointF( area.right(), guide ) ) );
    }
    foreach( double guide, m_vGuideLines )
    {
        if( guide < area.left() || guide > area.right() )
            continue;
        painter.drawLine( converter.documentToView( QPointF( guide, area.top() ) ),
                          converter.documentToView( QPointF( guide, area.bottom() ) ) );
    }
}
