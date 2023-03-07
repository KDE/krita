/*
 *  SPDX-FileCopyrightText: Copyright (c) 2011 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KoPolygonUtils.h"
#include <boost/polygon/polygon.hpp>
#include <QPolygon>
#include <QList>

namespace boost { namespace polygon {
    // QPoint wrapper
    template <>
    struct geometry_concept<QPoint> {
        typedef point_concept type;
    };

    template <>
    struct point_traits<QPoint> {
        typedef int coordinate_type;

        static inline coordinate_type get(const QPoint& point,
                                          orientation_2d orient) {
            if(orient == HORIZONTAL)
                return point.x();
            return point.y();
        }
    };

    template <>
    struct point_mutable_traits<QPoint> {
        static inline void set(QPoint& point, orientation_2d orient, int value) {
            if(orient == HORIZONTAL)
                point.rx() = value;
            else
                point.ry() = value;
        }
        static inline QPoint construct(int x_value, int y_value) {
            QPoint retval;
            retval.rx() = x_value;
            retval.ry() = y_value;
            return retval;
        }
    };

    // QPolygon wrapper
    template <>
    struct geometry_concept<QPolygon>{ typedef polygon_concept type; };

    template <>
    struct polygon_traits<QPolygon> {
        typedef int coordinate_type;
        typedef QPolygon::const_iterator iterator_type;
        typedef QPoint point_type;

        static inline iterator_type begin_points(const QPolygon& t) {
            return t.begin();
        }

        static inline iterator_type end_points(const QPolygon& t) {
            return t.end();
        }

        static inline std::size_t size(const QPolygon& t) {
            return t.size();
        }

        static inline winding_direction winding(const QPolygon& t) {
            Q_UNUSED(t);
            return unknown_winding;
        }
    };

    template <>
    struct polygon_mutable_traits<QPolygon> {
        template <typename iT>
        static inline QPolygon& set_points(QPolygon& t,
                                           iT input_begin, iT input_end) {
            t.clear();

            for(iT iter = input_begin; iter != input_end; iter++) {
                t.push_back(QPoint(iter->x(), iter->y()));
            }
            return t;
        }

    };
} }

typedef std::vector<QPolygon> PolygonSet;



QPolygon KoPolygonUtils::offsetPolygon(const QPolygon &polygon, int offset, bool rounded, int circleSegments)
{
    PolygonSet polygonSet;
    polygonSet.push_back(polygon);
    boost::polygon::resize(polygonSet, offset, rounded, circleSegments);
    return polygonSet[0];
}

QList<QPolygon> KoPolygonUtils::offsetPolygons(const QList<QPolygon> polygons, int offset, bool rounded, int circleSegments)
{
    PolygonSet polygonSet;
    Q_FOREACH(QPolygon polygon, polygons) {
        polygonSet.push_back(polygon);
    }
    boost::polygon::resize(polygonSet, offset, rounded, circleSegments);

    QList<QPolygon> finalPolygons;
    for (int i=0; i < int(polygonSet.size()); i++) {
        finalPolygons.append(polygonSet.at(i));
    }
    return finalPolygons;
}

