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

#ifndef _MATCHING_HPP_
#define _MATCHING_HPP_
#include <list>
#include <vector>
#include <QRect>

#include <kis_types.h>

#include "kis_interest_points_detector.h"

struct KisMatch {
    const KisInterestPoint* ref;
    const KisInterestPoint* match;
    double strength;
    inline bool operator==(const KisMatch& m) const {
        return ref == m.ref and match == m.match;
    }
};

typedef std::vector<KisMatch> lMatches;
lMatches matching(const lInterestPoints& pointsref, const lInterestPoints& pointsmatch);
#endif
