/*
 * matching.cpp -- Part of Krita
 *
 * Copyright (c) 2005-2006 Cyrille Berger (cberger@cberger.net)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Library General Public License as published by
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

#ifndef _HARRIS_DETECTOR_HPP_
#define _HARRIS_DETECTOR_HPP_

#include <kis_types.h>

#include "kis_interest_points_detector.h"

class HarrisPointDetector : public KisInterestPointsDetector {
    public:
        virtual ~HarrisPointDetector() { }
        virtual lInterestPoints computeInterestPoints(KisPaintDeviceSP device, const QRect& area);
};


#if 0
struct HarrisPoint {
    HarrisPoint(int ni, int nj, float nint, float nl1, float nl2) : i(ni), j(nj), intensity(nint), lambda1(nl1), lambda2(nl2) { }
    int i,j;
    float intensity, lambda1, lambda2;
};

typedef std::list<HarrisPoint> lHarrisPoints;

/**
 * Compute the harris point for the device, it is assumed that the device is grayscale.
 */
lHarrisPoints computeHarrisPoints(KisPaintDeviceSP device, QRect area);

struct Match {
    const HarrisPoint* ref,* match;
    Q_INT32 l1Ref, l2Ref, l1Match, l2Match;
    double strength;
};
typedef std::vector<Match> vMatches;
vMatches matching(KisPaintDeviceSP layerRef, const lHarrisPoints& pointsref, KisPaintDeviceSP layerMatch, const lHarrisPoints& pointsmatch);
#endif

#endif
