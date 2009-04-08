/*
 *  Copyright (c) 2007 Cyrille Berger (cberger@cberger.net)
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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "kis_control_points.h"


#include "kis_control_point.h"
#include "kis_interest_points_detector.h"

struct KisControlPoints::Private {
    QList<KisControlPoint> points;
    int framesCount;
};

KisControlPoints::KisControlPoints(int framesCount) : d(new Private)
{
    d->framesCount = framesCount;
}

KisControlPoints::~KisControlPoints()
{
    delete d;
}

void KisControlPoints::addControlPoint(const KisControlPoint& cp)
{
    d->points.append(cp);
}

bool KisControlPoints::findControlPoint(const KisInterestPoint* ip, int imageNb, KisControlPoint& result)
{
    for (QList<KisControlPoint>::iterator it = d->points.begin(); it != d->points.end(); ++it) {
        if (it->frames.contains(imageNb)) {
            if (it->interestPoints[ imageNb ] == ip) {
                result = *it;
                return true;
            }
        }
    }
    return false;
}

const QList<KisControlPoint>& KisControlPoints::controlPoints() const
{
    return d->points;
}

void KisControlPoints::addMatches(const lMatches& matches, int frameRef, int frameMatch)
{
    foreach(const KisMatch & m, matches) {
        KisControlPoint cp(d->framesCount);
        if (findControlPoint(m.ref, frameRef, cp)) {
            dbgKrita << "bouuh " << cp.interestPoints[0] << " == " << m.ref << " " << cp.interestPoints[1] << " == " << m.match;
            cp.frames.append(frameMatch);
            cp.positions[frameMatch] = QPointF(m.match->x(), m.match->y());
            cp.interestPoints[frameMatch] = m.match;
        } else if (findControlPoint(m.match, frameMatch, cp)) {
            dbgKrita << "pas mieux";
            cp.frames.append(frameRef);
            cp.positions[frameRef] = QPointF(m.ref->x(), m.ref->y());
            cp.interestPoints[frameRef] = m.ref;
        } else {
            cp.frames.append(frameRef);
            cp.positions[frameRef] = QPointF(m.ref->x(), m.ref->y());
            dbgKrita << frameRef << " " << cp.positions[frameRef];
            cp.interestPoints[frameRef] = m.ref;
            cp.frames.append(frameMatch);
            cp.positions[frameMatch] = QPointF(m.match->x(), m.match->y());
            dbgKrita << frameMatch << " " << cp.positions[frameMatch];
            cp.interestPoints[frameMatch] = m.match;
            addControlPoint(cp);
        }
    }

}

