/*
 *  Copyright (c) 2007 Jan Hambrecht <jaham@gmx.net>
 *  Copyright (c) 2020 Sharaf Zaman <sharafzaz121@gmail.com>
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
#include "SvgMeshPatch.h"

#include <math.h>
#include <QDebug>
#include <KoPathPoint.h>
#include <KoPathSegment.h>
#include <kis_global.h>


SvgMeshPatch::SvgMeshPatch(QPointF startingPoint)
    : m_newPath(true)
    , m_startingPoint(startingPoint)
    , m_path(new KoPathShape)
    , m_parametricCoords({{0, 0}, {1, 0}, {1, 1}, {0, 1}})
{
}

SvgMeshPatch::SvgMeshPatch(const SvgMeshPatch& other)
    : m_newPath(other.m_newPath)
    , m_startingPoint(other.m_startingPoint)
    , m_nodes(other.m_nodes)
    , m_path(static_cast<KoPathShape*>(other.m_path->cloneShape()))
    , m_parametricCoords({{0, 0}, {1, 0}, {1, 1}, {0, 1}})
{
}

SvgMeshStop SvgMeshPatch::getStop(SvgMeshPatch::Type type) const
{
    if (m_nodes.find(type) == m_nodes.end())
        return SvgMeshStop();

    return *m_nodes.find(type);
}

KoPathSegment SvgMeshPatch::getPathSegment(Type type) const
{
    KoPathPointIndex index(0, type - 1);
    return m_path->segmentByIndex(index);
}

KoPathShape* SvgMeshPatch::getPath() const
{
    return m_path.get();
}

QSizeF SvgMeshPatch::size() const
{
    return m_path->size();
}

KoPathSegment SvgMeshPatch::getMidCurve(bool isVertical) const
{
    QList<QPointF> curvedBoundary0;
    QList<QPointF> curvedBoundary1;

    QPointF midpointRuled0;
    QPointF midpointRuled1;

    if (isVertical) {
        curvedBoundary0 = getPathSegment(Right).controlPoints();
        curvedBoundary1 = getPathSegment(Left).controlPoints();

        midpointRuled0 = getPathSegment(Top).pointAt(0.5);
        midpointRuled1 = getPathSegment(Bottom).pointAt(0.5);
    } else {
        curvedBoundary0 = getPathSegment(Top).controlPoints();
        curvedBoundary1 = getPathSegment(Bottom).controlPoints();

        midpointRuled0 = getPathSegment(Left).pointAt(0.5);
        midpointRuled1 = getPathSegment(Right).pointAt(0.5);
    }

    // we have to reverse it, cB1 & cB2 are in opposite direction
    std::reverse(curvedBoundary1.begin(), curvedBoundary1.end());

    // Sum of two Bezier curve is a Bezier curve
    QList<QPointF> midCurved = {
        (curvedBoundary0[0] + curvedBoundary1[0]) / 2,
        (curvedBoundary0[1] + curvedBoundary1[1]) / 2,
        (curvedBoundary0[2] + curvedBoundary1[2]) / 2,
        (curvedBoundary0[3] + curvedBoundary1[3]) / 2,
    };

    // line cutting the bilinear surface in middle
    KoPathSegment midBilinear = KoPathSegment(midpointRuled0, midpointRuled1).toCubic();
    QPointF x_2_1 = midBilinear.pointAt(1.0 / 3);
    QPointF x_2_2 = midBilinear.pointAt(2.0 / 3);

    // line cutting rulled surface in middle
    KoPathSegment midRuled3 = KoPathSegment(midCurved[0], midCurved[3]).toCubic();
    QPointF x_3_1 = midRuled3.pointAt(1.0 / 3);
    QPointF x_3_2 = midRuled3.pointAt(2.0 / 3);

    QPointF p[4];

    p[0] = midpointRuled0;

    // X_1 = x_1_1 + x_2_1 - x_3_1
    p[1] = midCurved[1] + x_2_1 - x_3_1;

    // X_2 = x_1_2 + x_2_2 - x_3_2
    p[2] = midCurved[2] + x_2_2 - x_3_2;

    p[3] = midpointRuled1;

    return KoPathSegment(p[0], p[1], p[2], p[3]);
}

QPointF lerp(QPointF p1, QPointF p2, qreal t)
{
    return (1 - t) * p1 + t * p2;
}

QPointF SvgMeshPatch::pointAt(qreal u, qreal v) const
{
    KoPathSegment C1 = getPathSegment(Top);
    KoPathSegment C2 = getPathSegment(Bottom);

    QPointF S_c = (1 - v) * getPathSegment(Top).pointAt(u) + v * getPathSegment(Bottom).pointAt(u);
    QPointF S_d = (1 - u) * getPathSegment(Left).pointAt(v) + u * getPathSegment(Right).pointAt(v);
    QPointF S_b = lerp(lerp(C1.pointAt(0), C1.pointAt(1), u), lerp(C2.pointAt(0), C2.pointAt(1), u), v);

    return S_c + S_d - S_b;
}

void SvgMeshPatch::subdivide(QVector<SvgMeshPatch*>& subdivided, const QVector<QColor>& colors) const
{
    KIS_ASSERT(colors.size() == 5);

    // The orientation is left to right and top to bottom, which means
    // Eg. the first part of splitTop is TopLeft and the second part is TopRight
    // Similarly the first part of splitRight is RightTop, but the first part of
    // splitLeft is splitLeft.second (once again, in Top to Bottom  convention)
    QPair<KoPathSegment, KoPathSegment> splitTop    = getPathSegment(Top).splitAt(0.5);
    QPair<KoPathSegment, KoPathSegment> splitRight  = getPathSegment(Right).splitAt(0.5);
    QPair<KoPathSegment, KoPathSegment> splitBottom = getPathSegment(Bottom).splitAt(0.5);
    QPair<KoPathSegment, KoPathSegment> splitLeft   = getPathSegment(Left).splitAt(0.5);

    // The way the curve and the colors at the corners are arranged before and after subdivision
    //
    //              midc12
    //       c1       +       c2
    //        +---------------+
    //        |       |       |
    //        |       | midVer|
    //        |       | <     |
    // midc41 +---------------+ midc23
    //        |  ^    |       |
    //        | midHor|       |
    //        |       |       |
    //        +---------------+
    //       c4       +       c3
    //              midc43
    //
    QPair<KoPathSegment, KoPathSegment> midHor = getMidCurve(/*isVertical = */ false).splitAt(0.5);
    QPair<KoPathSegment, KoPathSegment> midVer = getMidCurve(/*isVertical = */ true).splitAt(0.5);

    // middle curve is shared among the two, so we need both directions
    QList<QPointF> reversedMidHorFirst = midHor.first.controlPoints();
    std::reverse(reversedMidHorFirst.begin(), reversedMidHorFirst.end());
    QList<QPointF> reversedMidHorSecond = midHor.second.controlPoints();
    std::reverse(reversedMidHorSecond.begin(), reversedMidHorSecond.end());

    QList<QPointF> reversedMidVerFirst = midVer.first.controlPoints();
    std::reverse(reversedMidVerFirst.begin(), reversedMidVerFirst.end());
    QList<QPointF> reversedMidVerSecond = midVer.second.controlPoints();
    std::reverse(reversedMidVerSecond.begin(), reversedMidVerSecond.end());

    QColor c1 = getStop(Top).color;
    QColor c2 = getStop(Right).color;
    QColor c3 = getStop(Bottom).color;
    QColor c4 = getStop(Left).color;
    QColor midc12 = colors[0];
    QColor midc23 = colors[1];
    QColor midc34 = colors[2];
    QColor midc41 = colors[3];
    QColor center = colors[4];

    // mid points in parametric space
    QPointF midTopP     = getMidpointParametric(Top);
    QPointF midRightP   = getMidpointParametric(Right);
    QPointF midBottomP  = getMidpointParametric(Bottom);
    QPointF midLeftP    = getMidpointParametric(Left);
    QPointF centerP     = 0.5 * (midTopP + midBottomP);

    // patch 1: TopLeft/NorthWest
    SvgMeshPatch *patch = new SvgMeshPatch(splitTop.first.first()->point());
    patch->addStop(splitTop.first.controlPoints(), c1, Top);
    patch->addStop(midVer.first.controlPoints(), midc12, Right);
    patch->addStop(reversedMidHorFirst, center, Bottom);
    patch->addStop(splitLeft.second.controlPoints(), midc41, Left);
    patch->m_parametricCoords = {
        m_parametricCoords[0],
        midTopP,
        centerP,
        midLeftP
    };
    subdivided.append(patch);

    // patch 2: TopRight/NorthRight
    patch = new SvgMeshPatch(splitTop.second.first()->point());
    patch->addStop(splitTop.second.controlPoints(), midc12, Top);
    patch->addStop(splitRight.first.controlPoints(), c2, Right);
    patch->addStop(reversedMidHorSecond, midc23, Bottom);
    patch->addStop(reversedMidVerFirst, center, Left);
    patch->m_parametricCoords = {
        midTopP,
        m_parametricCoords[1],
        midRightP,
        centerP
    };
    subdivided.append(patch);

    // patch 3: BottomLeft/SouthWest
    patch = new SvgMeshPatch(midHor.first.first()->point());
    patch->addStop(midHor.first.controlPoints(), midc41, Top);
    patch->addStop(midVer.second.controlPoints(), center, Right);
    patch->addStop(splitBottom.second.controlPoints(), midc34, Bottom);
    patch->addStop(splitLeft.first.controlPoints(), c4, Left);
    patch->m_parametricCoords = {
        midLeftP,
        centerP,
        midBottomP,
        m_parametricCoords[3]
    };
    subdivided.append(patch);

    // patch 4: BottomRight/SouthEast
    patch = new SvgMeshPatch(midHor.second.first()->point());
    patch->addStop(midHor.second.controlPoints(), center, Top);
    patch->addStop(splitRight.second.controlPoints(), midc23, Right);
    patch->addStop(splitBottom.first.controlPoints(), c3, Bottom);
    patch->addStop(reversedMidVerSecond, midc34, Left);
    patch->m_parametricCoords = {
        centerP,
        midRightP,
        m_parametricCoords[2],
        midBottomP
    };
    subdivided.append(patch);
}

void SvgMeshPatch::addStop(const QString& pathStr,
                           QColor color,
                           Type edge,
                           bool pathIncomplete,
                           QPointF lastPoint)
{
    SvgMeshStop node(color, m_startingPoint);
    m_nodes.insert(edge, node);

    m_startingPoint = parseMeshPath(pathStr, pathIncomplete, lastPoint);
}

void SvgMeshPatch::addStop(const QList<QPointF>& pathPoints, QColor color, Type edge)
{
    SvgMeshStop stop(color, pathPoints.first());
    m_nodes.insert(edge, stop);

    if (edge == SvgMeshPatch::Top) {
        m_path->moveTo(pathPoints.first());
        m_newPath = false;
    }

    // if path is a line
    if (pathPoints.size() == 2) {
        // we convert lines to cubic curve
        KoPathSegment cubicCurve(m_startingPoint, pathPoints.last());
        m_path->curveTo(cubicCurve.pointAt(1.0 / 3),
                        cubicCurve.pointAt(2.0 / 3),
                        cubicCurve.second()->point());
    } else if (pathPoints.size() == 4) {
        // if path is a Bezier curve
        m_path->curveTo(pathPoints[1], pathPoints[2], pathPoints[3]);
    }

    m_startingPoint = pathPoints.last();
}

void transformShape(const QTransform& matrix, QScopedPointer<KoPathShape>& shape)
{
    // apparently, you can't transform KoPathShape
    for (int i = 0; i < shape->pointCount(); ++i) {
        KoPathPointIndex index(0, i);
        KoPathPoint *point = shape->pointByIndex(index);

        QPointF previous = point->point();
        QPointF cp1 = point->controlPoint1();
        QPointF cp2 = point->controlPoint2();

        point->setPoint(matrix.map(previous));
        point->setControlPoint1(matrix.map(cp1));
        point->setControlPoint2(matrix.map(cp2));
    }
}

void SvgMeshPatch::setTransform(const QTransform& matrix)
{
    transformShape(matrix, m_path);
    m_startingPoint = matrix.map(m_startingPoint);
    for (int i = 1; i < Size; ++i) {
        m_nodes[static_cast<Type>(i)].point = matrix.map(m_nodes[static_cast<Type>(i)].point);
    }
}

int SvgMeshPatch::countPoints() const
{
    return m_nodes.size();
}


QRectF SvgMeshPatch::boundingRect() const
{
    return m_path->absoluteOutlineRect();
}

QPointF SvgMeshPatch::parseMeshPath(const QString& s, bool pathIncomplete, const QPointF lastPoint)
{
    // bits and pieces from KoPathShapeLoader, see the copyright above
    if (!s.isEmpty()) {
        QString d = s;
        d.replace(',', ' ');
        d = d.simplified();

        const QByteArray buffer = d.toLatin1();
        const char *ptr = buffer.constData();
        qreal curx = m_startingPoint.x();
        qreal cury = m_startingPoint.y();
        qreal tox, toy, x1, y1, x2, y2;
        bool relative = false;
        char command = *(ptr++);

        if (m_newPath) {
            m_path->moveTo(m_startingPoint);
            m_newPath = false;
        }

       while (*ptr == ' ')
           ++ptr;

       switch (command) {
       case 'l':
           relative = true;
           Q_FALLTHROUGH();
       case 'L': {
           ptr = getCoord(ptr, tox);
           ptr = getCoord(ptr, toy);

           if (relative) {
               tox = curx + tox;
               toy = cury + toy;
           }

           if (pathIncomplete) {
               tox = lastPoint.x();
               toy = lastPoint.y();
           }

           // we convert lines to cubic curve
           KoPathSegment cubicCurve(m_startingPoint, {tox, toy});
           m_path->curveTo(cubicCurve.pointAt(1.0 / 3),
                           cubicCurve.pointAt(2.0 / 3),
                           cubicCurve.second()->point());
           break;
       }
       case 'c':
           relative = true;
           Q_FALLTHROUGH();
       case 'C': {
           ptr = getCoord(ptr, x1);
           ptr = getCoord(ptr, y1);
           ptr = getCoord(ptr, x2);
           ptr = getCoord(ptr, y2);
           ptr = getCoord(ptr, tox);
           ptr = getCoord(ptr, toy);

           if (relative) {
               x1  = curx + x1;
               y1  = cury + y1;
               x2  = curx + x2;
               y2  = cury + y2;
               tox = curx + tox;
               toy = cury + toy;
           }

           if (pathIncomplete) {
               tox = lastPoint.x();
               toy = lastPoint.y();
           }

           m_path->curveTo(QPointF(x1, y1), QPointF(x2, y2), QPointF(tox, toy));
           break;
       }

       default: {
           qWarning() << "SvgMeshPatch::parseMeshPath: Bad command \"" << command << "\"";
           return QPointF();
       }
       }
       return {tox, toy};
    }
    return QPointF();
}

const char* SvgMeshPatch::getCoord(const char* ptr, qreal& number)
{
    // copied from KoPathShapeLoader, see the copyright above
    int integer, exponent;
    qreal decimal, frac;
    int sign, expsign;

    exponent = 0;
    integer = 0;
    frac = 1.0;
    decimal = 0;
    sign = 1;
    expsign = 1;

    // read the sign
    if (*ptr == '+')
        ++ptr;
    else if (*ptr == '-') {
        ++ptr;
        sign = -1;
    }

    // read the integer part
    while (*ptr != '\0' && *ptr >= '0' && *ptr <= '9')
        integer = (integer * 10) + *(ptr++) - '0';
    if (*ptr == '.') { // read the decimals
        ++ptr;
        while (*ptr != '\0' && *ptr >= '0' && *ptr <= '9')
            decimal += (*(ptr++) - '0') * (frac *= 0.1);
    }

    if (*ptr == 'e' || *ptr == 'E') { // read the exponent part
        ++ptr;

        // read the sign of the exponent
        if (*ptr == '+')
            ++ptr;
        else if (*ptr == '-') {
            ++ptr;
            expsign = -1;
        }

        exponent = 0;
        while (*ptr != '\0' && *ptr >= '0' && *ptr <= '9') {
            exponent *= 10;
            exponent += *ptr - '0';
            ++ptr;
        }
    }
    number = integer + decimal;
    number *= sign * pow((qreal)10, qreal(expsign * exponent));

    // skip the following space
    if (*ptr == ' ')
        ++ptr;

    return ptr;
}
