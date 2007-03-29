/*
 * Copyright (c) 2000 Clara Chan
 * Copyright (c) 2007 Boudewijn Rempt
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#include "brush.h"
#include "bristle.h"
#include "gauss.h"


#define RADIUS_INCRE 1.5
#define DX .8
#define DY .8
#define MAXPRESSURE 1023
#define MAXTX 100
#define MAXTY 100
#define MAXINK 200  //max units of ink on a bristle
#define SPREAD 1/350


Brush::Brush ( int s )
{
    m_bristles = new Bristle[s];
    setSize ( s );
    for ( int i = 0; i < 3; i++ )
        addInk();
}

Brush::~Brush ()
{
    delete m_bristles;
}

void Brush::setInitialPosition ( double x, double y )
{
    int i;

    for ( i=0; i < m_numBristles; i++ ) {
        m_bristles[i].setInitialPosition ( x, y );
    }
}


void Brush::setSize ( int s )
{
    m_size = s;
    setBristlesPos();
}


void Brush::repositionBristles ( double pre )
{
    double x, y, dx, dy, px, py, maxradius;
    int i;

    if ( pre <= 1.0 ) pre = 1.0;  // make sure pressure is at least 1
    m_radius = m_size * RADIUS_INCRE * ( 1 + pre * SPREAD );
    maxradius = 6 * RADIUS_INCRE * ( 1 + MAXPRESSURE * SPREAD );
    dx = DX * ( 1 + pre*SPREAD );
    dy = DY * ( 1 + pre*SPREAD );

    x = y = -1.0 * m_radius;
    i = 0;
    while ( x < m_radius && i < m_numBristles ) {
        while ( y < m_radius ) {
            if ( x*x + y*y < m_radius * m_radius ) {
                px = gauss::gaussian ( x, .5*DX, 1 );
                py = gauss::gaussian ( y, .5*DY, 1 );
                if ( fabs (px) > x+DX )
                    px = x;
                if ( fabs (py) > y+DY )
                    py = y;
                m_bristles[i++].SetPos ( px, py );
            }
            y += dy;
        }
        y = -1.0 * m_radius;
        x += dx;
    }

    // Create ink stealing
    int indx, i1, i2, i3, i4;
    double rn;

    if ( m_size > 1 ) {
        for ( i=0; i< m_numBristles * 0.01 * m_size; i++ ) {
            rn = ( double ) rand ();
            rn = rn / RAND_MAX * m_numBristles;
            indx = ( int ) rn;
            if ( indx > m_numBristles - 1 )
                indx = m_numBristles / 2;
            if ( indx > m_numBristles - 2 )
                indx = indx - 2;
            if ( indx < 2 )
                indx = 2;

            if ( m_bristles[indx-2].GetInkAmount() - m_size < 0 )
                i1 = m_bristles[indx-2].GetInkAmount();
            else
                i1 = m_size;
            if ( m_bristles[indx-1].GetInkAmount() - m_size < 0 )
                i2 = m_bristles[indx-1].GetInkAmount();
            else
                i2 =  m_size;
            if ( m_bristles[indx+1].GetInkAmount() - m_size < 0 )
                i3 = m_bristles[indx+1].GetInkAmount();
            else
                i3 =  m_size;
            if ( m_bristles[indx+2].GetInkAmount() - m_size < 0 )
                i4 = m_bristles[indx+2].GetInkAmount();
            else
                i4 =  m_size;
            m_bristles[indx-2].SetInkAmount ( m_bristles[indx-2].GetInkAmount()-i1 );
            m_bristles[indx-1].SetInkAmount ( m_bristles[indx-2].GetInkAmount()-i2 );
            m_bristles[indx+1].SetInkAmount ( m_bristles[indx+1].GetInkAmount()-i3 );
            m_bristles[indx+2].SetInkAmount ( m_bristles[indx+2].GetInkAmount()-i4 );
            m_bristles[indx].SetInkAmount ( m_bristles[indx].GetInkAmount()+i1+i2+i3+i4 );
        }
    }
}


void Brush::setBristlesPos ()
{
    double x, y, p, tx, ty, maxradius, px, py, xxyy;
    int i, j;

    m_numBristles = 0;
    m_radius =  m_size * RADIUS_INCRE * ( 1 + 1/500.0 );
    maxradius = 6 * RADIUS_INCRE * ( 1 + MAXPRESSURE/500.0 );
    i = ( int )( pow ( m_radius / DX * m_radius / DY, 2 ) * 4 );
    if ( m_bristles )
        delete ( m_bristles );
    m_bristles = new Bristle[i];

    x = y = -1.0 * m_radius;
    while ( x <  m_radius ) {
        while ( y <  m_radius ) {
            if ( (xxyy=(x*x + y*y)) <  m_radius* m_radius ) {
                px = gauss::gaussian ( x, .5*DX, 1 );
                if ( fabs (px) > x+DX )
                    px = x;
                py = gauss::gaussian ( y, .5*DY, 1 );
                if ( fabs (py) > y+DY )
                    py = y;
                m_bristles[m_numBristles].SetPos ( px, py );
                p = sqrt(px * px + py * py)/( m_radius) * MAXPRESSURE;
                m_bristles[m_numBristles].SetPreThres ( p );

                tx = (px /  m_radius) * (px /  m_radius) * 60.0;
                ty = (py /  m_radius) * (py /  m_radius) * 60.0;

                m_bristles[m_numBristles].SetTXThres ( tx );
                m_bristles[m_numBristles].SetTYThres ( ty );
                m_numBristles++;
            }
            y += DY;
        }
        y = -1.0 *  m_radius;
        x += DX;
    }

    for ( j = 0; j < m_numBristles; j++ ) {
        m_bristles[j].initializeThickness ( m_size );
    }
}

void Brush::addInk ()
{
    int totalInk = m_numBristles * MAXINK * 0.2;
    for ( int i = 0; i < totalInk; i++ ) {

        double rn = ( double ) rand ();
        rn = rn / RAND_MAX * m_numBristles;
        int indx = ( int ) rn;

        if ( indx > m_numBristles - 1 )
            indx = m_numBristles / 2;

        if ( m_bristles[indx].GetInkAmount() < MAXINK ) {
            double ink = m_bristles[indx].DistanceFromCenter() / m_radius;
            ink = 10.0 / ink + gauss::gaussian ( 10.0, 5.0, 0 );
            double ink2 = ink;
            if ( ink2 > 200 ) ink2 = 200;
            m_bristles[indx].addInk ( ink2 );
            totalInk -= ink2;
        }
    }
}


void Brush::removeInk ()
{
    int totalInk = m_numBristles * MAXINK * 0.2;
    for ( int i = 0; i < totalInk; i++ ) {

        double rn = ( double ) rand ();
        rn = rn / RAND_MAX * m_numBristles;
        int indx = ( int ) rn;

        if ( indx > m_numBristles-1 )
            indx = m_numBristles / 2;

        if ( m_bristles[indx].GetInkAmount() > 0 ) {
            m_bristles[indx].depleteInk(1);
            totalInk--;
        }
    }
}
