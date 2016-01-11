/* This file is part of the KDE project
   Copyright (C) 2008 Fela Winkelmolen <fela.kde@gmail.com>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "KarbonSimplifyPath.h"

#include <KoCurveFit.h>
#include <KoPathShape.h>
#include <KoPathPoint.h>
#include <QDebug>

/*
the algorithm proceeds as following:

1. divide the paths wherever it's not smooth
2. for each of the resulting paths that has at least three points, add points
   recursively wherever the path is too "complicated"(*). TODO
3. apply bezierFit on the resulting points
4. remerge the paths

(*) TODO: write definition of too complicated here

FIXME: bezier fit seems to crash when getting to many points in input,
       if there are to many point in one of the subpaths, split it
*/

namespace KarbonSimplifyPath
{
const qreal SUBDIVISION_COEFF = 100; // use error instead?
const int MAX_RECURSIVE_DEPTH = 1024;
int recursiveDepth;

void removeDuplicates(KoPathShape *path);

QList<KoSubpath *> split(const KoPathShape &path);

// subdivides the path adding additional points where to "complicated"
void subdivide(KoSubpath *subpath);
// returns the points that needs to be inserted between p1 and p2
KoSubpath subdivideAux(KoPathPoint *p1, KoPathPoint *p2);
// auxiliary function
bool isSufficentlyFlat(QPointF curve[4]);

// after this call the points _are_ owned by the subpaths
void simplifySubpaths(QList<KoSubpath *> *subpaths, qreal error);
// auxiliary function for the above
void simplifySubpath(KoSubpath *subpath, qreal error);

// put the result into path
void mergeSubpaths(QList<KoSubpath *> subpaths, KoPathShape *path);
}

using namespace KarbonSimplifyPath;

// TODO: rename to simplify subpath
void karbonSimplifyPath(KoPathShape *path, qreal error)
{
    if (path->pointCount() == 0) {
        return;
    }

    removeDuplicates(path);

    bool isClosed = path->isClosedSubpath(0);
    if (isClosed) {
        // insert a copy of the first point at the end
        KoPathPoint *firstPoint = path->pointByIndex(KoPathPointIndex(0, 0));
        KoPathPointIndex end(0, path->pointCount());
        path->insertPoint(new KoPathPoint(*firstPoint), end);
    }

    QList<KoSubpath *> subpaths = split(*path);
    Q_FOREACH (KoSubpath *subpath, subpaths) {
        subdivide(subpath);
    }

    simplifySubpaths(&subpaths, error);
    mergeSubpaths(subpaths, path);

    while (! subpaths.isEmpty()) {
        KoSubpath *subpath = subpaths.takeLast();
        qDeleteAll(*subpath);
        delete subpath;
    }

    if (isClosed) {
        path->closeMerge();
    }
}

void KarbonSimplifyPath::removeDuplicates(KoPathShape *path)
{
    // NOTE: works because path has only has one subshape, if this ever moves in
    //       KoPathPoint it should be changed
    for (int i = 1; i < path->pointCount(); ++i) {
        KoPathPoint *p = path->pointByIndex(KoPathPointIndex(0, i));
        KoPathPoint *prev = path->pointByIndex(KoPathPointIndex(0, i - 1));
        QPointF diff = p->point() - prev->point();
        // if diff = 0 remove point
        if (qFuzzyCompare(diff.x() + 1, 1) && qFuzzyCompare(diff.y() + 1, 1)) {
            if (prev->activeControlPoint1()) {
                p->setControlPoint1(prev->controlPoint1());
            } else {
                p->removeControlPoint1();
            }
            delete path->removePoint(KoPathPointIndex(0, i - 1));
            --i;
        }
    }
}

QList<KoSubpath *> KarbonSimplifyPath::split(const KoPathShape &path)
{
    QList<KoSubpath *> res;
    KoSubpath *subpath = new KoSubpath;
    res.append(subpath);

    for (int i = 0; i < path.pointCount(); ++i) {
        KoPathPoint *p = path.pointByIndex(KoPathPointIndex(0, i));
        // if the path separates two subpaths
        // (if it isn't smooth nor the first or last point)
        if (i != 0  &&  i != path.pointCount() - 1) {
            KoPathPoint *prev = path.pointByIndex(KoPathPointIndex(0, i - 1));
            KoPathPoint *next = path.pointByIndex(KoPathPointIndex(0, i + 1));
            if (!p->isSmooth(prev, next)) {
                // create a new subpath
                subpath->append(new KoPathPoint(*p));
                subpath = new KoSubpath;
                res.append(subpath);
            }
        }
        subpath->append(new KoPathPoint(*p));
    }

    return res;
}

void KarbonSimplifyPath::subdivide(KoSubpath *subpath)
{
    for (int i = 1; i < subpath->size(); ++i) {
        recursiveDepth = 0;
        KoSubpath newPoints = subdivideAux((*subpath)[i - 1], (*subpath)[i]);
        Q_FOREACH (KoPathPoint *p, newPoints) {
            subpath->insert(i, p);
            ++i;
        }
    }
}

KoSubpath KarbonSimplifyPath::subdivideAux(KoPathPoint *p1,
        KoPathPoint *p2)
{
    if (!p1->activeControlPoint1() && !p2->activeControlPoint2()) {
        return QList<KoPathPoint *>();
    }

    QPointF curve[4] = {
        p1->point(),
        p1->activeControlPoint2() ? p1->controlPoint2() : p1->point(),
        p2->activeControlPoint1() ? p2->controlPoint1() : p2->point(),
        p2->point()
    };

    // if there is no need to add points do nothing
    if (isSufficentlyFlat(curve)) {
        return QList<KoPathPoint *>();
    }

    ++recursiveDepth;
    if (recursiveDepth >= MAX_RECURSIVE_DEPTH) {
        qDebug() << "reached MAX_RECURSIVE_DEPTH";
        --recursiveDepth;
        return QList<KoPathPoint *>();
    }

    // calculate the new point using the de Casteljau algorithm
    QPointF p[3];

    for (unsigned short j = 1; j <= 3; ++j) {
        for (unsigned short i = 0; i <= 3 - j; ++i) {
            curve[i] = (curve[i] + curve[i + 1]) / 2.0;
        }
        // modify the new segment.
        p[j - 1] = curve[0];
    }

    KoPathPoint *pm = new KoPathPoint(0, p[2]);
    pm->setControlPoint1(p[1]);
    pm->setControlPoint2(curve[1]);
    p1->setControlPoint2(p[0]);
    p2->setControlPoint1(curve[2]);

    KoSubpath res;
    res << subdivideAux(p1, pm) << pm << subdivideAux(pm, p2);
    --recursiveDepth;
    return res;
}

bool KarbonSimplifyPath::isSufficentlyFlat(QPointF curve[4])
{
    qreal ux = 3 * curve[1].x() - 2 * curve[0].x() - curve[3].x();
    qreal uy = 3 * curve[1].y() - 2 * curve[0].y() - curve[3].y();
    qreal vx = 3 * curve[2].x() - 2 * curve[3].x() - curve[0].x();
    qreal vy = 3 * curve[2].x() - 2 * curve[3].x() - curve[0].x();

    // calculate the square of the distance between the points
    qreal dx = curve[0].x() - curve[3].y();
    qreal dy = curve[0].y() - curve[3].y();
    qreal dist2 = dx * dx + dy * dy;
    qreal max2 = qMax(ux * ux, vx * vx) + qMax(uy * uy, vy * vy);
    max2 *= (SUBDIVISION_COEFF * SUBDIVISION_COEFF);

    return max2 <= dist2;
}

void KarbonSimplifyPath::simplifySubpaths(QList<KoSubpath *> *subpaths,
        qreal error)
{
    Q_FOREACH (KoSubpath *subpath, *subpaths) {
        if (subpath->size() > 2) {
            simplifySubpath(subpath, error);
        }
    }
}

void KarbonSimplifyPath::simplifySubpath(KoSubpath *subpath, qreal error)
{
    QList<QPointF> points;

    for (int i = 0; i < subpath->size(); ++i) {
        points.append((*subpath)[i]->point());
    }

    KoPathShape *simplified = bezierFit(points, error);

    qDeleteAll(*subpath);
    subpath->clear();

    for (int i = 0; i < simplified->pointCount(); ++i) {
        KoPathPointIndex index(0, i);
        subpath->append(new KoPathPoint(*simplified->pointByIndex(index)));
    }
    //res->setPosition( position() );
    delete simplified;
}

void KarbonSimplifyPath::mergeSubpaths(QList<KoSubpath *> subpaths, KoPathShape *path)
{
    path->clear();
    path->moveTo(subpaths.first()->first()->point());

    // TODO: to make the code more readable use foreach and explicit
    //       counters with full name
    // si: subpath index, pi: point index
    for (int si = 0; si < subpaths.size(); ++si) {
        for (int pi = 1; pi < subpaths[si]->size(); ++pi) {
            KoPathPoint *point = (*subpaths[si])[pi];
            path->lineTo(point->point());

            // set the first control point
            KoPathPointIndex index(0, path->pointCount() - 1);
            KoPathPoint *p = path->pointByIndex(index);
            if (point->activeControlPoint1()) {
                p->setControlPoint1(point->controlPoint1());
            }

            // set the second control point of the previous point
            index = KoPathPointIndex(0, path->pointCount() - 2);
            p = path->pointByIndex(index);
            KoPathPoint *prev = (*subpaths[si])[pi - 1];
            if (prev->activeControlPoint2()) {
                p->setControlPoint2(prev->controlPoint2());
            }
        }
    }
}
