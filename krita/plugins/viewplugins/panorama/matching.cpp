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

#if 1

#include "matching.h"

#include <map>

#include <kdebug.h>

#include <kis_autobrush_resource.h>
#include <kis_convolution_painter.h>
#include <kis_generic_colorspace.h>
#include <kis_iterators_pixel.h>
#include <kis_layer.h>
#include <kis_transaction.h>

#define ABS(x) ((x) < 0 ? -(x) : (x))

#define THRESHOLD_FAST_MATCH 0.9
#define THRESHOLD_MATCH 0.9

lMatches matching(const lInterestPoints& pointsref, const lInterestPoints& pointsmatch)
{
    lMatches matchPoints;
    
    // Simple matching
    
    for(lInterestPoints::const_iterator it_ref = pointsref.begin(); it_ref != pointsref.end(); it_ref++)
    {
        double bestScore = 0.0;
        const KisInterestPoint* bestPoint = 0;
        for(lInterestPoints::const_iterator it_match = pointsmatch.begin(); it_match != pointsmatch.end(); it_match++)
        {
            if( (*it_ref)->fastCompare(*it_match) > THRESHOLD_FAST_MATCH)
            {
                double score = (*it_ref)->compare(*it_match);
                if(score > THRESHOLD_MATCH and score > bestScore)
                {
                    bestScore = score;
                    bestPoint = *it_match;
                }
            }
        }
        if(bestPoint)
        {
            double bestScore2 = 0.0;
            const KisInterestPoint* bestPoint2 = 0;
            for(lInterestPoints::const_iterator it_ref2 = pointsref.begin(); it_ref2 != pointsref.end(); it_ref2++)
            {
                if(bestPoint->fastCompare( *it_ref2) > THRESHOLD_FAST_MATCH)
                {
                    double score = bestPoint->compare(*it_ref2);
                    if(score >= bestScore and score > bestScore2)
                    {
                        bestPoint2 = *it_ref2;
                        bestScore2 = score;
                    }
                }
            }
            if(bestPoint2 == *it_ref)
            {
                KisMatch m;
                m.ref = *it_ref;
                m.match = bestPoint;
                m.strength = bestScore;
                matchPoints.push_back( m );
            }
        }
    }
    kDebug() << matchPoints.size() << " points were matched" << endl;
    return matchPoints;
}

#endif
