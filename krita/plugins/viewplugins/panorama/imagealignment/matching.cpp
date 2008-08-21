/*
 * matching.cpp -- Part of Krita
 *
 * Copyright (c) 2005-2006 Cyrille Berger (cberger@cberger.net)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2 of the License.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA. */

#include "matching.h"


#include <map>

#include <kis_debug.h>

#include <kis_convolution_painter.h>
#include <kis_generic_colorspace.h>
#include <kis_iterators_pixel.h>
#include <kis_layer.h>
#include <kis_transaction.h>
#include <kis_paint_device.h>

#define ABS(x) ((x) < 0 ? -(x) : (x))

// #define THRESHOLD_FAST_MATCH 0.9
// #define THRESHOLD_MATCH 0.9

double distancePoint(KisInterestPoint* point, KisInterestPoint* point2)
{
  double dx = point->x() - point2->x();
  double dy = point->y() - point2->y();
  return dx*dx + dy*dy;
}

void prepareGroups(lInterestPoints& pointsref, double maxDistance)
{
  maxDistance *= maxDistance;
  for( lInterestPoints::iterator point = pointsref.begin();
       point != pointsref.end(); ++point)
  {
    for( lInterestPoints::iterator point2 = pointsref.begin();
         point2 != pointsref.end(); ++point2)
    {
      if( *point != *point2 && distancePoint(*point, *point2) < maxDistance )
      {
        (*point)->appendNeighbourgh( *point2 );
      }
    }
  }
}

const KisInterestPoint* bestMatchFor( const KisInterestPoint* ip, const lInterestPoints& pointsref, const lInterestPoints& pointsmatch, double& score, double thresholdFastMatch, double thresholdMatch )
{
    double bestScore = 0.0;
    const KisInterestPoint* bestPoint = 0;
    for(lInterestPoints::const_iterator it_match = pointsmatch.begin(); it_match != pointsmatch.end(); it_match++)
    {
        if( ip->fastCompare(*it_match) > thresholdFastMatch)
        {
            double score = ip->compare(*it_match);
            if(score > thresholdMatch && score > bestScore)
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
            if(bestPoint->fastCompare( *it_ref2) > thresholdFastMatch)
            {
                double score = bestPoint->compare(*it_ref2);
                if(score >= bestScore && score > bestScore2)
                {
                    bestPoint2 = *it_ref2;
                    bestScore2 = score;
                }
            }
        }
        if(bestPoint2 == ip)
        {
          score = bestScore;
          return bestPoint;
        }
    }
    return 0;
}

#if 0

void matchingPropag( QList<const KisInterestPoint*>& allreadyMatchedRef, QList<const KisInterestPoint*>& allreadyMatchedMatch , lMatches& matchPoints, const lInterestPoints& pointsref, const lInterestPoints& pointsmatch, double thresholdFastMatch, double thresholdMatch )
{
//     dbgKrita << "matchingPropag " << thresholdFastMatch << " " << thresholdMatch;
    for(lInterestPoints::const_iterator it_ref = pointsref.begin(); it_ref != pointsref.end(); it_ref++)
    {
      if( not allreadyMatchedRef.contains( *it_ref ) )
      {
        double score;
        const KisInterestPoint* bestPoint = bestMatchFor(*it_ref, pointsref, pointsmatch, score, thresholdFastMatch, thresholdMatch );
        if(bestPoint)
        {
            KisMatch m;
            m.ref = *it_ref;
            m.match = bestPoint;
            m.strength = score;
            matchPoints.push_back( m );
            allreadyMatchedRef.append( m.ref );
            allreadyMatchedMatch.append( m.match );
//             dbgKrita << m.ref->neighbourghood().size();
            matchingPropag( allreadyMatchedRef, allreadyMatchedMatch, matchPoints, m.ref->neighbourghood(), m.match->neighbourghood(), 0.6, 0.6 );
        }
      }
    }

}

lMatches matching(const lInterestPoints& pointsref, const lInterestPoints& pointsmatch)
{
    lMatches matchPoints;

    QList<const KisInterestPoint*> allreadyMatchedRef;
    QList<const KisInterestPoint*> allreadyMatchedMatch;

    // Simple matching
      matchingPropag( allreadyMatchedRef, allreadyMatchedMatch, matchPoints, pointsref,  pointsmatch, 0.8, 0.8 );

    dbgPlugins << matchPoints.size() <<" points were matched";
    return matchPoints;
}
#endif

#if 1

inline double diffAngle(double a1, double a2 )
{
    double d = a1 - a2;
    if(d <= - M_PI ) d += 2* M_PI;
    else if(d > M_PI) d -= 2* M_PI;
    return d;
}

struct MatchHypothesis
{
    double angle;
    QList<const KisInterestPoint*> allreadyMatchedRef;
    QList<const KisInterestPoint*> allreadyMatchedMatch;
    lMatches matches;
    double sumScore;
};

struct MatchPreHypothesis {
    MatchPreHypothesis(const KisInterestPoint* _ref, const KisInterestPoint* _match, double _angle,
    double _score) : ref(_ref), match(_match), angle(_angle), score(_score)
    {
    }
    const KisInterestPoint* ref;
    const KisInterestPoint* match;
    double angle;
    double score;
};

MatchHypothesis findSeed( const KisInterestPoint* ref, const lInterestPoints& pointsmatch, const QList<const KisInterestPoint*>& allreadyMatchedRef, const QList<const KisInterestPoint*>& allreadyMatchedMatch)
{
    MatchHypothesis bestHypo;
    bestHypo.sumScore = 0.0;
    for(lInterestPoints::const_iterator it_match = pointsmatch.begin();
        it_match != pointsmatch.end(); it_match++)
    {
        if( !allreadyMatchedMatch.contains( *it_match ) )
        {
            if( (*it_match)->fastCompare( ref ) > 0.9)
            {
                double score = (*it_match)->compare( ref );
                if( score > 0.9)
                {
                    QList<MatchPreHypothesis> preHs;
                    QList<double > angles;
                    for(lInterestPoints::const_iterator it_ref_group = ref->neighbourghood().begin();
                        it_ref_group != ref->neighbourghood().end();
                        ++it_ref_group)
                    {
                        if( !allreadyMatchedRef.contains( *it_ref_group) )
                        {
                            double score;
                            const KisInterestPoint* ip = bestMatchFor( *it_ref_group, ref->neighbourghood(), (*it_match)->neighbourghood(), score, 0.6, 0.6);
                            if( ip && !allreadyMatchedMatch.contains( ip ) )
                            {
                                double angle = diffAngle( atan2( (*it_ref_group)->y() - ref->y(), (*it_ref_group)->x() - ref->x() ), atan2( ip->y() - (*it_match)->y(), ip->x() - (*it_match)->x() ) );
                                preHs.append( MatchPreHypothesis( *it_ref_group, ip, angle, score) );
                                angles.append( angle);
                            }
                        }
                    }
                    if(!angles.empty())
                    {
                        qSort(angles);
                        double median = angles[ angles.size() / 2 ];
                        dbgPlugins << " preHypothesis : " << preHs.size() << " median = " << median;
                        MatchHypothesis currentHypo;
                        currentHypo.sumScore = 0.0;
                        foreach( const MatchPreHypothesis & mph, preHs )
                        {
                            if( diffAngle(mph.angle, median ) < 0.1)
                            {
                                currentHypo.matches.push_back( KisMatch( mph.ref, mph.match, mph.score ) );
                                currentHypo.allreadyMatchedRef.append( mph.ref );
                                currentHypo.allreadyMatchedMatch.append( mph.match );
                                currentHypo.sumScore += mph.score;
                                dbgKrita << currentHypo.sumScore << " " << mph.score;
                            }
                        }
                        // Possible match
                        currentHypo.allreadyMatchedRef.append( ref );
                        currentHypo.allreadyMatchedMatch.append( *it_match );
                        currentHypo.matches.push_back( KisMatch(ref, *it_match, score ) );
                        currentHypo.sumScore += score;
                        dbgKrita << currentHypo.sumScore << " " << score;
                        currentHypo.angle = median;
                        if(currentHypo.sumScore > bestHypo.sumScore)
                        {
                            bestHypo = currentHypo;
                        }
                    } else {
                        dbgPlugins << " no preHypothesis : ";
                    }
                }
            }
        }
    }
    return bestHypo;
}

void propageMatch( MatchHypothesis& hypo, const KisMatch& seedMatch, const QList<const KisInterestPoint*>& allreadyMatchedRef, const QList<const KisInterestPoint*>& allreadyMatchedMatch )
{
    for(lInterestPoints::const_iterator it_ref_group = seedMatch.ref->neighbourghood().begin();
    it_ref_group != seedMatch.ref->neighbourghood().end();
    ++it_ref_group)
    {
        if( !allreadyMatchedRef.contains( *it_ref_group) && !hypo.allreadyMatchedRef.contains( *it_ref_group)  )
        {
            double score;
            const KisInterestPoint* ip = bestMatchFor( *it_ref_group, seedMatch.ref->neighbourghood(), seedMatch.match->neighbourghood(), score, 0.6, 0.6);
            if( ip && !allreadyMatchedMatch.contains( ip ) && !hypo.allreadyMatchedMatch.contains( ip ) )
            {
                KisMatch m( *it_ref_group, ip, score );
                hypo.matches.push_back( m );
                hypo.allreadyMatchedRef.append( *it_ref_group );
                hypo.allreadyMatchedMatch.append( ip );
                hypo.sumScore += score;
                propageMatch(hypo, m, allreadyMatchedRef, allreadyMatchedMatch );
            }
        }
    }
}

lMatches matching(const lInterestPoints& pointsref, const lInterestPoints& pointsmatch)
{
    lMatches matchPoints;

    QList<const KisInterestPoint*> allreadyMatchedRef;
    QList<const KisInterestPoint*> allreadyMatchedMatch;
    for(lInterestPoints::const_iterator it_ref = pointsref.begin(); it_ref != pointsref.end(); it_ref++)
    {
        if( !allreadyMatchedRef.contains( *it_ref ) )
        {
            MatchHypothesis hypo = findSeed(*it_ref, pointsmatch, allreadyMatchedRef, allreadyMatchedMatch );
            if( hypo.sumScore > (0.6 * 3) && fabs(hypo.angle) < 0.1 )
            {
                dbgPlugins << "Kept : " << hypo.angle << " " << hypo.sumScore << " " << hypo.matches.size();
                double score = hypo.sumScore;
                lMatches seedMatches = hypo.matches;
                for(lMatches::iterator it = seedMatches.begin();
                    it != seedMatches.end();
                    it++)
                {
                    propageMatch( hypo, *it, allreadyMatchedRef, allreadyMatchedMatch );
                }
                dbgPlugins << "After propagation : " << hypo.angle << " " << hypo.sumScore << " " << hypo.matches.size();
                // Merge
                allreadyMatchedRef += hypo.allreadyMatchedRef;
                allreadyMatchedMatch += hypo.allreadyMatchedMatch;
                for(lMatches::iterator it = hypo.matches.begin();
                    it != hypo.matches.end();
                    it++)
                {
                    matchPoints.push_back( *it );
                }
            }
        }
    }
    dbgKrita << "Nb of matches : " << matchPoints.size();
    return matchPoints;
}

#endif
