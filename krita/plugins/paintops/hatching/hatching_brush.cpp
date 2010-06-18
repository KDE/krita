/*
 *  Copyright (c) 2008,2009,2010 Lukáš Tvrdý <lukast.dev@gmail.com>
 *  Copyright (c) 2010 José Luis Vergara <pentalis@gmail.com>
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

#include "hatching_brush.h"

#include <KoColor.h>
#include <KoColorSpace.h>
#include <KoColorTransformation.h>

#include <QVariant>
#include <QHash>

#include "kis_random_accessor.h"
#include <cmath>
#include <time.h>


void inline myround (double *x) {
    *x = ((*x - floor(*x)) >= 0.5) ? ceil(*x) : floor(*x);
}

HatchingBrush::HatchingBrush(const KisHatchingPaintOpSettings* settings)
{
    m_settings = new KisHatchingPaintOpSettings();
    m_settings = settings;
    //settings->initializeTwin(m_settings);
    std::clog << "AERSH " << m_settings->crosshatchingstyle << "\n";
}


HatchingBrush::~HatchingBrush()
{
}

void HatchingBrush::init()
{
}

void HatchingBrush::paint(KisPaintDeviceSP dev, qreal x, qreal y, const KoColor &color)
{
    m_painter.begin(dev);
    m_painter.setFillStyle(KisPainter::FillStyleForegroundColor);
    m_painter.setPaintColor(color);
    m_painter.setBackgroundColor(color);
    
    //std::clog << m_settings->proeba() << " ";

    double naiveangle, angle, s, h, w, xcoor, ycoor, b, p, dx, dy, last_b, cursor_b;
    int thickness;
    //*****PSEUDOSETTINGS****
    naiveangle = m_settings->angle;
    /*naiveangle is what I receive from the controls. For now, I have no use for angles
    belonging to the 2nd and 3rd quadrant, the controls were then made such that the user
    sees the least ambiguous possible settings (no angles from the 2nd and 3rd quadrant)*/
    angle = naiveangle;
    thickness = m_settings->thickness;
    s = m_settings->separation;
    h = fabs(m_settings->origin_y);
    w = fabs(m_settings->origin_x);
    xcoor = 0; //m_settings->origin_x;
    ycoor = 0; //m_settings->origin_y;
    dx = dy = b = p = last_b = cursor_b = 0;    //inicializar
    
    m_painter.setMaskImageSize(w, h);
    QRect limits(QPoint(0,0), QPoint(w, h));
    m_painter.setBounds(limits);
    
    //</PSEUDOSETTINGS>

    //****DESCRIBING THE FIRST (BASE) LINE****
    //QPoint origin(xcoor, ycoor);

    //****dx and dy are the separation between lines in the x and y axis
    //dx = s / sin(angle*M_PI/180);    // csc = 1/sin(angle)
    dy = fabs(s / cos(angle*M_PI/180));    // sec = 1/cos(angle), ABSOLUTE VALUE please
    //always positive because I don't need negatives confusing everything later

    if (!m_settings->subpixelprecision)
        modf(dy, &dy);
    
    //****EXCEPTION FOR VERTICAL LINES, FOR WHICH A TANGENT DOES NOT EXIST****
    if ((angle == 90) || (angle == -90))
    {
        //vertical line procedure
    }
    else
    {
        //****TURN ANGLE+POINT INTO AN ALGEBRAIC LINE****
        p = tan(angle*M_PI/180);     //angle into slope
        b = ycoor - p*xcoor;         //slope and point into intercept
        cursor_b = y - p*x;
        //printf ("The tangent of %lf degrees is %lf.\n", angle, p );     //debug line
        //    %    is the modulus operator, fmod is the float modulus operator

        last_b = fmod((b - cursor_b), dy);     // last_b is the historical name for the b to start iterating

        //printf ("The result of my last_b = fmod(b, dy) is last_b = %lf, b = %lf and dy = %lf.\n", last_b, b, dy ); //debug
        //std::clog << "p is worth " << p << " and b is worth " << b << "\n";
        //std::clog << "dx is worth " << dx << " and dy is worth " << dy << "\n";

        //I tried to make this cleaner but there's too many possibilities to be worth
        //the micromanagement to optimize
        iteratelines (thickness, h, w, p, dy, last_b, 1, false);    //forward, include base line
        iteratelines (thickness, h, w, p, dy, last_b, 0, true); //do the line between both iterations
        iteratelines (thickness, h, w, p, -dy, last_b, 1, false);    //backward
    }
}


void HatchingBrush::iteratelines (int thickness, double h, double w, double p, double dy, double last_b, int lineindex, bool oneline)
{
    double b;
    
    //Declarations before the loop
    double xdraw[2] = {0, 0};
    double ydraw[2] = {0, 0};
    int append_index = 0;
    bool remaininginnerlines = true;
    QPointF A, B;    //points A and B of the segments to trace
    
    while (remaininginnerlines)
    { 
    //****INTERSECTION POINT VERIFICATION****
    
    //--Preamble
    append_index = 0;
    remaininginnerlines = false; //we assume there's no more lines unless proven contrary
    
    //b will now represent the intercept of the current line
    b = last_b + dy*lineindex;
    //std::clog << "b is NOW worth " << b << "and lineindex is..." << lineindex << "\n";
    lineindex++;
    //we're descending vertically out of convenience, see blog entry at pentalis.org/kritablog
    
    /*explanation: only 2 out of the 4 segments can include limit values
    to verify intersections, otherwise we could encounter a situation where
    our new lines intersect with all 4 segments and is still considered an
    inner line (for example, a line that goes from corner to corner), thus
    triggering an error. The idea is of the algorithm is that only 2 intersections
    at most are considered at a time. Graphically this is indistinguishable, it's
    just there to avoid making unnecesary control structures (like additional "ifs").
    */
        
        if ((b >= 0) && (b <= h)) {
            xdraw[append_index] = 0;    //interseccion at left
            ydraw[append_index] = b;
            remaininginnerlines = true;
            append_index++;
            //std::clog << "INTERSECTION LEFT \n" ;
        }
        
        if ((p*w + b <= h) && (p*w + b >= 0)) {
            xdraw[append_index] = w;
            ydraw[append_index] = b + p*w; //interseccion at right
            remaininginnerlines = true;
            append_index++;
            //std::clog << "INTERSECTION RIGHT \n" ;
        }
        
        if ((-b/p > 0) && (-b/p < w)) {
            xdraw[append_index] = -b/p;
            ydraw[append_index] = 0; //interseccion at top
            remaininginnerlines = true;
            append_index++;
            //std::clog << "INTERSECTION TOP \n" ;
        }
            
        if (((h-b)/p > 0) && ((h-b)/p < w)) {
            xdraw[append_index] = (h-b)/p;
            ydraw[append_index] = h;     //interseccion at bottom
            remaininginnerlines = true;
            append_index++;
            //std::clog << "INTERSECTION BOTTOM \n" ;
        }
        
        if (!remaininginnerlines) {
            //std::clog << "I AM EMO \n";
            break;
        }
        
        //Testing subpixel precision OFF
        /*
        modf(xdraw[0], &xdraw[0]);
        modf(xdraw[1], &xdraw[1]);
        modf(ydraw[0], &ydraw[0]);
        modf(ydraw[1], &ydraw[1]);
        */
        
        if (!m_settings->subpixelprecision)
        {
            myround(&xdraw[0]);
            myround(&xdraw[1]);
            myround(&ydraw[0]);
            myround(&ydraw[1]);
        }
        
        //std::clog << "a modf(xdraw[0]) le entra " << xdraw[0] << "\n";
        A.setX(xdraw[0]);

        //std::clog << "a modf(ydraw[0]) le entra " << ydraw[0] << "\n";
        A.setY(ydraw[0]);
        
        /*this control structure is here to handle special situations, like
        lines that intersect only at 1 point right in the corners*/
        if (append_index == 2) {
            //std::clog << "a modf(xdraw[1]) le entra " << xdraw[1] << "\n";
            B.setX(xdraw[1]);
            
            //std::clog << "a modf(ydraw[1]) le entra " << ydraw[1] << "\n";
            B.setY(ydraw[1]);
            
            if (m_settings->antialias)
                m_painter.drawThickLine(A, B, thickness, thickness);
            else        
                m_painter.drawDDALine(A, B);    //testing no subpixel;
            
            if (oneline) {
                //std::clog << "I AM STOPID \n";
                break;
            }
        }
        else
        {
            /*Drawing points at the vertices causes incosistent results due to
            floating point calculations not being quite in sync with algebra,
            therefore if I have only 1 intersection (= corner), don't draw*/
            continue;
        }
        printf ("Punto A: %f, %f . Punto B: %f, %f. \n", xdraw[0], ydraw[0], xdraw[1], ydraw[1]);
        
    }  //endwhile
}
