/*
 *  Copyright (c) 2000 Clara Chan
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include <QPainter>

#include <KoColor.h>

#include <kis_vec.h>

#include "brush.h"
#include "bristle.h"
#include "stroke.h"
#include "sample.h"


// Constructor
Stroke::Stroke (Brush * br )
{
    brush = br;
    numBristles = brush->numberOfBristles();
    oldPathx = new vector<double> [numBristles];
    oldPathy = new vector<double> [numBristles];
    valid = new vector<int> [numBristles];
}

Stroke::~Stroke()
{
    sampleV.erase ( sampleV.begin(), sampleV.end() );
}


void Stroke::storeOldPath ( double x1, double y1 )
{
    int i;

    for ( i=0; i<numBristles; i++ ) {
        oldPathx[i].push_back ( x1 );
        oldPathy[i].push_back ( y1 );
        valid[i].push_back ( 1 );
    }
}

// draw the stroke by drawing the old paths, then the new segment
void Stroke::draw (QPainter & gc)
{
    int i;
    int x, y;
    double tiltx, tilty;
    double pre;

    numBristles = brush->numberOfBristles();

    // get info from the last sample
    if ( sampleV.size() >= 1 ) {
        i = sampleV.size()-1;
        pre = (double)sampleV[i]->pressure();
        x = sampleV[i]->x();
        y = sampleV[i]->y();
        tiltx = sampleV[i]->tiltX();
        tilty = sampleV[i]->tiltY();
    }

    // using pressure info to reposition bristles
    brush->repositionBristles( pre );

    // draw the new segment
    for ( i = 0; i < numBristles; i++ ) {
        if ( testThreshold ( i, pre, tiltx, tilty ) ) {
            if ( valid[i][oldPathx[i].size()-1] ) {
                gc.setPen( QPen( m_color.toQColor(), brush->m_bristles[i].GetThickness() ) );
                gc.drawLine( oldPathx[i][oldPathx[i].size() - 1], oldPathy[i][oldPathy[i].size() - 1],
                             brush->m_bristles[i].GetX() + x, brush->m_bristles[i].GetY() + y );
                brush->m_bristles[i].depleteInk ( 1 ); //remove one unit of ink from bristle
            }
            valid[i].push_back ( 1 );
        }
        else
            valid[i].push_back ( 0 );

        // store new positions in oldPaths
        oldPathx[i].push_back ( brush->m_bristles[i].GetX()+x );
        oldPathy[i].push_back ( brush->m_bristles[i].GetY()+y );
    }
}


void Stroke::setColor ( const KoColor & color )
{
    m_color = color;
}

bool Stroke::testThreshold ( int i, double pre, double tx, double ty )
{
    if ( ( brush->m_bristles[i].GetPreThres() < pre )
         && brush->m_bristles[i].GetInkAmount() > 0 )
        return 1;
    else
        return 0;
}










