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

#include <kis_paint_device.h>
#include <kis_point.h>
#include <kis_vec.h>

#include "stroke.h"

// Constructor
Stroke::Stroke (Brush * br )
{
    brush = br;
    newStroke = 0;
    numBristles = brush->GetNumBristles();
    oldPathx = new vector<double> [numBristles];
    oldPathy = new vector<double> [numBristles];
    valid = new vector<int> [numBristles];
}


void Stroke::StoreOldPath ( double x1, double y1 )
{
    int i;

    for ( i=0; i<numBristles; i++ ) {
        oldPathx[i].push_back ( x1 );
        oldPathy[i].push_back ( y1 );
        valid[i].push_back ( 1 );
    }
}


// called when "Undo" button is hit
void Stroke::Redraw ()
{
#if 0
    int i, j;

    //glDisable ( GL_DITHER );
    glColor4ub ( r, g, b, a/2+9 );
    numBristles = brush->GetNumBristles();

    // draw previous paths
    for ( i=0; i<numBristles; i++ ) {
        glLineWidth ( brush->bristle[i].GetThickness() );
        if ( oldPathx[i].size() > 1 ) {
            glBegin ( GL_LINE_STRIP );
            for ( j=0; j<oldPathx[i].size(); j++ ) {
                if ( valid[i][j] ) {
                    glVertex2f ( oldPathx[i][j], oldPathy[i][j] );
                }
                else {
                    glEnd();
                    glBegin ( GL_LINE_STRIP );
                }
            }
            glEnd ();
        }
    }
}
#endif
}


void drawLine( KisPaintDeviceSP dev, double x1, double y1, double x2, double y2, double width, const KisColor & color )
{

    if (!dev) return;

    KisPoint pos1 = KisPoint( x1, y1 );
    KisPoint pos2 = KisPoint( x2, y2 );

    KisVector2D end(pos2);
    KisVector2D start(pos1);

    KisVector2D dragVec = end - start;
    KisVector2D movement = dragVec;

    double xScale = 1;
    double yScale = 1;

    double dist = dragVec.length();

    if (dist < 1) {
        return;
    }

    dragVec.normalize();

    KisVector2D step(0, 0);

    kdDebug() << "drawLine " << x1 << ", " << y1 << " : " << x2 << ", " << y2 << ". Width: " << width << ", dist: " << dist << endl;

    while (dist >= 1) {

        step += dragVec;

        KisPoint p(start.x() + (step.x() / xScale), start.y() + (step.y() / yScale));
        kdDebug() << "paint at: " << p.roundX() << ", " << p.roundY() << endl;
        dev->setPixel(p.roundX(), p.roundY(), Qt::blue, OPACITY_OPAQUE);
        dist -= 1;
    }
}

// draw the stroke by drawing the old paths, then the new segment
void Stroke::Draw (KisPaintDeviceSP dev)
{
    int i;
    int x, y;
    double tiltx, tilty;
    double pre;

    numBristles = brush->GetNumBristles();

#if 0
    if ( !newStroke ) {
        for ( i=0; i<numBristles; i++ ) {
            glLineWidth ( brush->bristle[i].GetThickness() );
            if ( oldPathx[i].size() > 1 ) {
                for ( int j=1; j<oldPathx[i].size(); j++ ) {
                    if ( valid[i][j-1] && valid[i][j]) {
                        glBegin ( GL_LINES );
                        glVertex2f ( oldPathx[i][j-1], oldPathy[i][j-1] );
                        glVertex2f ( oldPathx[i][j], oldPathy[i][j] );
                        glEnd ();
                    }
                }
            }
        }
    }
#endif

    // get info from the last sample
    if ( sampleV.size() >= 1 ) {
        i = sampleV.size()-1;
        pre = (double)sampleV[i]->GetPressure();
        x = sampleV[i]->GetX();
        y = sampleV[i]->GetY();
        tiltx = sampleV[i]->GetTX();
        tilty = sampleV[i]->GetTY();
    }

    // using pressure info to reposition bristles
    brush->RepositionBristles ( pre );

    // draw the new segment
    for ( i=0; i<numBristles; i++ ) {
        if ( testThreshold ( i, pre, tiltx, tilty ) ) {
            if ( valid[i][oldPathx[i].size()-1] ) {
                drawLine( dev,
                          oldPathx[i][oldPathx[i].size()-1],
                          oldPathy[i][oldPathy[i].size()-1],
                          brush->bristle[i].GetX()+x,
                          brush->bristle[i].GetY()+y,
                          brush->bristle[i].GetThickness(),
                          m_color);
                brush->bristle[i].MinusInk ( 1 ); //remove one unit of ink from bristle
            }
            valid[i].push_back ( 1 );
        }
        else
            valid[i].push_back ( 0 );

        // store new positions in oldPaths
        oldPathx[i].push_back ( brush->bristle[i].GetX()+x );
        oldPathy[i].push_back ( brush->bristle[i].GetY()+y );
    }
}


void Stroke::FreeSamples ()
{
    sampleV.erase ( sampleV.begin(), sampleV.end() );
}


void Stroke::StoreColor ( const KisColor & color )
{
    m_color = color;
}


void Stroke::ResetBrush ()
{
    newStroke = 1;
}


// test whether a bristle is touching the paper or not
//   by testing the pressure threshold and test if there is any ink
int Stroke::testThreshold ( int i, double pre, double tx, double ty )
{
    if ( ( brush->bristle[i].GetPreThres() < pre )
         && brush->bristle[i].GetInkAmount() > 0 )
        return 1;
    else
        return 0;
}










