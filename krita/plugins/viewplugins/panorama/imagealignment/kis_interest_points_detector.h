/*
 * kis_interest_points_detector.h -- Part of Krita
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

#ifndef _KIS_INTEREST_POINTS_DETECTOR_H_
#define _KIS_INTEREST_POINTS_DETECTOR_H_

#include <QList>
#include <QRect>
#include <list>

#include <kis_types.h>

class KisInterestPoint {
    public:
        KisInterestPoint(double x, double y);
        virtual ~KisInterestPoint();
        virtual double fastCompare(const KisInterestPoint* ip) const =0;
        virtual double compare(const KisInterestPoint* ip) const =0;
        virtual QString toString() const = 0;
        inline double x() const { return m_x; }
        inline double y() const { return m_y; }
    private:
        double m_x, m_y;
};

typedef std::list<KisInterestPoint*> lInterestPoints;

class KisInterestPointsDetector {
    public:
        virtual ~KisInterestPointsDetector();
        virtual lInterestPoints computeInterestPoints(KisPaintDeviceSP device, const QRect& area) =0;
    public:
        inline static KisInterestPointsDetector* interestPointDetector()
        {
            return s_interestPointDetector;
        }
        static void setInterestPointDetector(int priority, KisInterestPointsDetector* interestPointDetector);
    private:
        static KisInterestPointsDetector* s_interestPointDetector;
        static int s_interestPointDetectorPriority;
};

#endif
