/* This file is part of the KDE project
   Copyright (C) 2006 Laurent Montel <montel@kde.org>

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

KoGuidesData::KoGuidesData()
 :m_bShowGuideLines(true)
{
}

void KoGuidesData::horizontalGuideLines( const QList<double> &lines )
{
    m_hGuideLines = lines;
}

void KoGuidesData::verticalGuideLines( const QList<double> &lines )
{
    m_vGuideLines = lines;
}

void KoGuidesData::guideLines( const QList<double> &horizontalLines, const QList<double> &verticalLines)
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

