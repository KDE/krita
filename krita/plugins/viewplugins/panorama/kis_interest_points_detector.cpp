/*
 * kis_interest_points_detector.cpp -- Part of Krita
 *
 * Copyright (c) 2007 Cyrille Berger (cberger@cberger.net)
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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA. */

#include "kis_interest_points_detector.h"

KisInterestPointsDetector* KisInterestPointsDetector::s_interestPointDetector = 0;
int KisInterestPointsDetector::s_interestPointDetectorPriority = -1;

void KisInterestPointsDetector::setInterestPointDetector(int priority, KisInterestPointsDetector* interestPointDetector)
{
    if(not s_interestPointDetector)
    {
        s_interestPointDetector = interestPointDetector;
        s_interestPointDetectorPriority = priority;
    } else if(priority > s_interestPointDetectorPriority) {
        delete s_interestPointDetector;
        s_interestPointDetector = interestPointDetector;
        s_interestPointDetectorPriority = priority;
    } else {
        delete interestPointDetector;
    }
}

KisInterestPoint::~KisInterestPoint()
{
}

KisInterestPointsDetector::~KisInterestPointsDetector()
{
}

KisInterestPoint::KisInterestPoint(double x, double y) : m_x(x), m_y(y)
{

}
