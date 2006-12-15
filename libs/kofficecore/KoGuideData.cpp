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

#include "KoGuideData.h"

KoGuideData::KoGuideData()
 :m_bShowGuideLines(true)
{
}

void KoGuideData::horizontalGuideLines( const QList<double> &lines )
{
    m_hGuideLines = lines;
}

void KoGuideData::verticalGuideLines( const QList<double> &lines )
{
    m_vGuideLines = lines;
}


void KoGuideData::addGuideLine( Qt::Orientation o, double pos )
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

bool KoGuideData::showGuideLines() const 
{ 
  return m_bShowGuideLines; 
}

void KoGuideData::setShowGuideLines( bool show )
{
  m_bShowGuideLines=show;
}

