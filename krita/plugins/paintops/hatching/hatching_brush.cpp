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
}


HatchingBrush::~HatchingBrush()
{
    delete m_settings;
}

void HatchingBrush::init()
{
}

void HatchingBrush::hatch(KisPaintDeviceSP dev, qreal x, qreal y, double width, double height, double givenangle, const KoColor &color)
{
    m_painter.begin(dev);
    m_painter.setFillStyle(KisPainter::FillStyleForegroundColor);
    m_painter.setPaintColor(color);
    m_painter.setBackgroundColor(color);
    
    angle = givenangle;
    thickness = m_settings->thickness * m_settings->thicknessSensorValue;
    separation = separationAsFunctionOfParameter(m_settings->separationSensorValue, m_settings->separation);
    height_ = height;
    width_ = width;
    origin_x = m_settings->origin_x;
    origin_y = m_settings->origin_y;
    dx = dy = scanIntercept = baseLineIntercept = slope = hotIntercept = cursorLineIntercept = 0;    // Initializing
    
    m_painter.setMaskImageSize(width_, height_);
    QRect limits(QPoint(0,0), QPoint(width_, height_));
    m_painter.setBounds(limits);
    
    /*  dx and dy are the separation between lines in the x and y axis
    dx = separation / sin(angle*M_PI/180);     csc = 1/sin(angle)  */
    dy = fabs(separation / cos(angle*M_PI/180));    // sec = 1/cos(angle)
    // I took the absolute value to avoid confusions with negative numbers

    if (!m_settings->subpixelprecision)
        modf(dy, &dy);
    
    // Exception for vertical lines, for which a tangent does not exist
    if ((angle == 90) || (angle == -90))
    {
        verticalHotX = fmod((origin_x - x), separation);
        
        iterateVerticalLines (true, 1, false);   // Forward
        iterateVerticalLines (true, 0, true);    // In Between both
        iterateVerticalLines (false, 1, false);  // Backward
    }
    else
    {
        // Turn Angle + Point into Slope + Intercept
        slope = tan(angle*M_PI/180);                    // Angle into slope
        baseLineIntercept = origin_y - slope*origin_x;  // Slope and Point of the Base Line into Intercept
        cursorLineIntercept = y - slope*x;
        hotIntercept = fmod((baseLineIntercept - cursorLineIntercept), dy);  // This hotIntercept belongs to a line that intersects with the hatching area

        iterateLines (true, 1, false);   // Forward
        iterateLines (true, 0, true);    // In Between both
        iterateLines (false, 1, false);  // Backward
        // I tried to make this cleaner but there's too many possibilities to be
        // worth the micromanagement to optimize
    }
}


void HatchingBrush::iterateLines(bool forward, int lineindex, bool oneline)
{
    //---Preparations before the loop---
    
    double xdraw[2] = {0, 0};
    double ydraw[2] = {0, 0};
    //points A and B of the segments to trace
    QPointF A, B;             
    int append_index = 0;
    bool remaininginnerlines = true;
    
    while (remaininginnerlines) { 
        
        //---------START INTERSECTION POINT VERIFICATION--------
        
        append_index = 0;
        remaininginnerlines = false; // We assume there's no more lines unless proven contrary
        if (forward)
            scanIntercept = hotIntercept + dy*lineindex;   // scanIntercept will represent the Intercept of the current line
        else
            scanIntercept = hotIntercept - dy*lineindex;    // scanIntercept will represent the Intercept of the current line

        lineindex++; // We are descending vertically out of convenience, see blog entry at pentalis.org/kritablog
        
        /*
        Explanation: only 2 out of the 4 segments can include limit values
        to verify intersections, otherwise we could encounter a situation where
        our new lines intersect with all 4 segments and is still considered an
        inner line (for example, a line that goes from corner to corner), thus
        triggering an error. The idea is of the algorithm is that only 2 intersections
        at most are considered at a time. Graphically this is indistinguishable, it's
        just there to avoid making unnecesary control structures (like additional "ifs").
        */
        
        if ((scanIntercept >= 0) && (scanIntercept <= height_)) {
            xdraw[append_index] = 0;
            ydraw[append_index] = scanIntercept;       //interseccion at left
            remaininginnerlines = true;
            append_index++;
        }
        
        if ((slope*width_ + scanIntercept <= height_) && (slope*width_ + scanIntercept >= 0)) {
            xdraw[append_index] = width_;
            ydraw[append_index] = scanIntercept + slope*width_; //interseccion at right
            remaininginnerlines = true;
            append_index++;
        }
        
        if ((-scanIntercept/slope > 0) && (-scanIntercept/slope < width_)) {
            xdraw[append_index] = -scanIntercept/slope;
            ydraw[append_index] = 0;       //interseccion at top
            remaininginnerlines = true;
            append_index++;
        }

        if (((height_-scanIntercept)/slope > 0) && ((height_-scanIntercept)/slope < width_)) {
            xdraw[append_index] = (height_-scanIntercept)/slope;
            ydraw[append_index] = height_;       //interseccion at bottom
            remaininginnerlines = true;
            append_index++;
        }
        //--------END INTERSECTION POINT VERIFICATION---------
        
        if (!remaininginnerlines)
            break;
        
        if (!m_settings->subpixelprecision) {
            myround(&xdraw[0]);
            myround(&xdraw[1]);
            myround(&ydraw[0]);
            myround(&ydraw[1]);
        }
        
        A.setX(xdraw[0]);
        A.setY(ydraw[0]);
        
        // If 2 lines intersect with the dab square
        if (append_index == 2) {
            B.setX(xdraw[1]);
            B.setY(ydraw[1]);
            
            if (m_settings->antialias)
                m_painter.drawThickLine(A, B, thickness, thickness);
            else        
                m_painter.drawDDALine(A, B);    //testing no subpixel;
            
            if (oneline)
                break;
        }
        else
        {
            continue;
            /*Drawing points at the vertices causes incosistent results due to
            floating point calculations not being quite in sync with algebra,
            therefore if I have only 1 intersection (= corner = this case),
            don't draw*/
        }
    }
}

void HatchingBrush::iterateVerticalLines(bool forward, int lineindex, bool oneline)
{
    //---Preparations before the loop---
    
    double xdraw = 0;
    double ydraw[2] = {0, height_};
    //points A and B of the segments to trace
    QPointF A, B;
    bool remaininginnerlines = true;
    
    while (remaininginnerlines) { 
        
        //---------START INTERSECTION POINT VERIFICATION--------
        remaininginnerlines = false;     // We assume there's no more lines unless proven contrary
        if (forward)
            verticalScanX = verticalHotX + separation*lineindex;
        else
            verticalScanX = verticalHotX - separation*lineindex;
        
        lineindex++;
        
        /*Read the explanation in HatchingBrush::iterateLines for more information*/
        
        if ((verticalScanX >= 0) && (verticalScanX <= width_)) {
            xdraw = verticalScanX;
            remaininginnerlines = true;
        }
        //--------END INTERSECTION POINT VERIFICATION---------
        
        if (!remaininginnerlines)
            break;
        
        if (!m_settings->subpixelprecision) {
            myround(&xdraw);
            myround(&ydraw[1]);
        }
        
        A.setX(xdraw);
        A.setY(ydraw[0]);
        B.setX(xdraw);
        B.setY(ydraw[1]);
        
        if (m_settings->antialias)
            m_painter.drawThickLine(A, B, thickness, thickness);
        else        
            m_painter.drawDDALine(A, B);    //testing no subpixel;
            
        if (oneline)
            break;
        else
            continue;
    }
}

double HatchingBrush::separationAsFunctionOfParameter(double parameter, double separation)
{
    if ((parameter >= 0) && (parameter < 0.2))
        return (separation * 4); 
    else if ((parameter >= 0.2) && (parameter < 0.4))
        return (separation * 2);
    else if ((parameter >= 0.4) && (parameter < 0.6))
        return (separation);
    else if ((parameter >= 0.6) && (parameter < 0.8))
        return (separation / 2);
    else if ((parameter >= 0.8) && (parameter <= 1.0))
        return (separation / 4);
    else {
        qDebug() << "Fix your function \n";
        return separation;
    }
}
;
