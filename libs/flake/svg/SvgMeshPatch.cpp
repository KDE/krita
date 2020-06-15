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


SvgMeshPatch::SvgMeshPatch(QPointF startingPoint)
    : m_newPath(true)
    , m_startingPoint(startingPoint)
    , m_path(new KoPathShape)
{
}

SvgMeshPatch::~SvgMeshPatch()
{
    for (auto &node: m_nodes.values()) {
        delete node;
    }
}

SvgMeshStop* SvgMeshPatch::getStop(SvgMeshPatch::Type type) const
{
    if (m_nodes.find(type) == m_nodes.end())
        return nullptr;

    return *m_nodes.find(type);
}

KoPathSegment SvgMeshPatch::getPath(Type type) const
{
    KoPathPointIndex index(0, type - 1);
    return m_path->segmentByIndex(index);
}

void SvgMeshPatch::addStop(const QString& pathStr, QColor color, Type edge, bool pathIncomplete, QPointF lastPoint)
{
    SvgMeshStop *node = new SvgMeshStop(color, m_startingPoint);
    m_nodes.insert(edge, node);

    m_startingPoint = parseMeshPath(pathStr, pathIncomplete, lastPoint);
}

void SvgMeshPatch::addStop(const QList<QPointF>& pathPoints, QColor color, Type edge)
{
    SvgMeshStop *stop = new SvgMeshStop(color, pathPoints.first());
    m_nodes.insert(edge, stop);

    if (edge == SvgMeshPatch::Top) {
        m_path->moveTo(pathPoints.first());
        m_newPath = false;
    }

    // if path is a line
    if (pathPoints.size() == 2) {
        m_path->lineTo(pathPoints.last());
    } else if (pathPoints.size() == 4) {
        // if path is a Bezier curve
        m_path->curveTo(pathPoints[1], pathPoints[2], pathPoints[3]);
    }

    m_startingPoint = pathPoints.last();
}

int SvgMeshPatch::countPoints() const
{
    return m_nodes.size();
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

           m_path->lineTo(QPointF(tox, toy));
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
