/*
 *  SPDX-FileCopyrightText: 2008, 2009, 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
 *  SPDX-FileCopyrightText: 2010 José Luis Vergara <pentalis@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "hatching_brush.h"

#include <KoColor.h>
#include <KoColorTransformation.h>

#include <QVariant>
#include <QHash>

#include "kis_random_accessor_ng.h"
#include <cmath>
#include <time.h>


void inline myround(double *x)
{
    *x = ((*x - floor(*x)) >= 0.5) ? ceil(*x) : floor(*x);
}

HatchingBrush::HatchingBrush(KisHatchingPaintOpSettingsSP settings)
  : m_settings(settings)
  , separation(m_settings->separation)
  , origin_x(m_settings->origin_x)
  , origin_y(m_settings->origin_y)
{
}


HatchingBrush::~HatchingBrush()
{
}

void HatchingBrush::init()
{
}

void HatchingBrush::hatch(KisPaintDeviceSP dev, qreal x, qreal y, double width, double height, double givenangle, const KoColor &color, qreal additionalScale)
{
    m_painter.begin(dev);
    m_painter.setFillStyle(KisPainter::FillStyleForegroundColor);
    m_painter.setPaintColor(color);
    m_painter.setBackgroundColor(color);

    angle = givenangle;
    double tempthickness = m_settings->thickness * m_settings->thicknesssensorvalue;
    thickness = qMax(1, qRound(additionalScale * tempthickness));
    separation = additionalScale *
        (m_settings->enabledcurveseparation ?
         separationAsFunctionOfParameter(m_settings->separationsensorvalue, m_settings->separation, m_settings->separationintervals) :
         m_settings->separation);

    height_ = height;
    width_ = width;

    m_painter.setMaskImageSize(width_, height_);

    /*  dx and dy are the separation between lines in the x and y axis
    dx = separation / sin(angle*M_PI/180);     csc = 1/sin(angle)  */
    dy = fabs(separation / cos(angle * M_PI / 180)); // sec = 1/cos(angle)
    // I took the absolute value to avoid confusions with negative numbers

    if (!m_settings->subpixelprecision)
        modf(dy, &dy);

    // Exception for vertical lines, for which a tangent does not exist
    if ((angle == 90) || (angle == -90)) {
        verticalHotX = fmod((origin_x - x), separation);

        iterateVerticalLines(true, 1, false);    // Forward
        iterateVerticalLines(true, 0, true);     // In Between both
        iterateVerticalLines(false, 1, false);   // Backward
    }
    else {
        // Turn Angle + Point into Slope + Intercept
        slope = tan(angle * M_PI / 180);                // Angle into slope
        baseLineIntercept = origin_y - slope * origin_x; // Slope and Point of the Base Line into Intercept
        cursorLineIntercept = y - slope * x;
        hotIntercept = fmod((baseLineIntercept - cursorLineIntercept), dy);  // This hotIntercept belongs to a line that intersects with the hatching area

        iterateLines(true, 1, false);    // Forward
        iterateLines(true, 0, true);     // In Between both
        iterateLines(false, 1, false);   // Backward
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
            scanIntercept = hotIntercept + dy * lineindex; // scanIntercept will represent the Intercept of the current line
        else
            scanIntercept = hotIntercept - dy * lineindex;  // scanIntercept will represent the Intercept of the current line

        lineindex++; // We are descending vertically out of convenience, see blog entry at pentalis.org/kritablog

        /*
        Explanation: only 2 out of the 4 segments can include limit values
        to verify intersections, otherwise we could encounter a situation where
        our new lines intersect with all 4 segments and is still considered an
        inner line (for example, a line that goes from corner to corner), thus
        triggering an error. The idea is of the algorithm is that only 2 intersections
        at most are considered at a time. Graphically this is indistinguishable, it's
        just there to avoid making unnecessary control structures (like additional "ifs").
        */

        if ((scanIntercept >= 0) && (scanIntercept <= height_)) {
            xdraw[append_index] = 0;
            ydraw[append_index] = scanIntercept;       //interseccion at left
            remaininginnerlines = true;
            append_index++;
        }

        if ((slope * width_ + scanIntercept <= height_) && (slope * width_ + scanIntercept >= 0)) {
            xdraw[append_index] = width_;
            ydraw[append_index] = scanIntercept + slope * width_; //interseccion at right
            remaininginnerlines = true;
            append_index++;
        }

        if ((-scanIntercept / slope > 0) && (-scanIntercept / slope < width_)) {
            xdraw[append_index] = -scanIntercept / slope;
            ydraw[append_index] = 0;       //interseccion at top
            remaininginnerlines = true;
            append_index++;
        }

        if (((height_ - scanIntercept) / slope > 0) && ((height_ - scanIntercept) / slope < width_)) {
            xdraw[append_index] = (height_ - scanIntercept) / slope;
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
                m_painter.drawLine(A, B, thickness, false);    //testing no subpixel;

            if (oneline)
                break;
        }
        else {
            continue;
            /*Drawing points at the vertices causes inconsistent results due to
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
            verticalScanX = verticalHotX + separation * lineindex;
        else
            verticalScanX = verticalHotX - separation * lineindex;

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
            m_painter.drawLine(A, B, thickness, false);    //testing no subpixel;

        if (oneline)
            break;
        else
            continue;
    }
}

double HatchingBrush::separationAsFunctionOfParameter(double parameter, double separation, int numintervals)
{
    if ((numintervals < 2) || (numintervals > 7)) {
        dbgKrita << "Fix your function" << numintervals << "<> 2-7" ;
        return separation;
    }

    double sizeinterval = 1 / double(numintervals);
    double lowerlimit = 0;
    double upperlimit = 0;
    double factor = 0;

    int basefactor = numintervals / 2;
    // Make the base separation factor tend to greater instead of lesser numbers when numintervals is even
    if ((numintervals % 2) == 0)
        basefactor--;

    for (quint8 currentinterval = 0; currentinterval < numintervals; currentinterval++) {
        lowerlimit = upperlimit;
        upperlimit += sizeinterval;
        if (currentinterval == (numintervals - 1))
            upperlimit = 1;
        if ((parameter >= lowerlimit) && (parameter <= upperlimit)) {
            factor = pow(2.0, (basefactor - currentinterval));
            //dbgKrita << factor;
            return (separation * factor);
        }
    }

    dbgKrita << "Fix your function" << parameter << ">" << upperlimit ;
    return separation;
}
