#include "kis_convex_hull.h"

#include "kis_image.h"
#include "kis_random_accessor_ng.h"
#include "KoColorSpace.h"
#include "KoColor.h"

#include <boost/geometry.hpp>

#include <QElapsedTimer>

namespace boost
{
    namespace geometry
    {
        namespace traits
        {
            // Adapt QPoint to Boost.Geometry

            template<> struct tag<QPoint>
            { typedef point_tag type; };

            template<> struct coordinate_type<QPoint>
            { typedef int type; };

            template<> struct coordinate_system<QPoint>
            { typedef cs::cartesian type; };

            template<> struct dimension<QPoint> : boost::mpl::int_<2> {};

            template<>
            struct access<QPoint, 0>
            {
                static int get(QPoint const& p)
                {
                    return p.x();
                }

                static void set(QPoint& p, int const& value)
                {
                    p.rx() = value;
                }
            };

            template<>
            struct access<QPoint, 1>
            {
                static int get(QPoint const& p)
                {
                    return p.y();
                }

                static void set(QPoint& p, int const& value)
                {
                    p.ry() = value;
                }
            };

            // Adapt QPolygon to Boost.Geometry as Linestring

            template<> struct tag<QPolygon>
            { typedef linestring_tag type; };
            
        }
    }

    template <>
    struct range_iterator<QPolygon>
    { typedef QPolygon::iterator type; };

    template<>
    struct range_const_iterator<QPolygon>
    { typedef QPolygon::const_iterator type; };
} // namespace boost::geometry::traits

namespace {

inline QPolygon::iterator range_begin(QPolygon& p)
{return QPolygon::iterator(p.begin());}

inline QPolygon::const_iterator range_begin(const QPolygon& p)
{return QPolygon::const_iterator(p.begin());}

inline QPolygon::iterator range_end(QPolygon& p)
{return QPolygon::iterator(p.end());}

inline QPolygon::const_iterator range_end(const QPolygon& p)
{return QPolygon::const_iterator(p.end());}

QPolygon convexHull(const QVector<QPoint> &points)
{
    QPolygon hull;
    boost::geometry::convex_hull(QPolygon(points), hull);
    return hull;
}

// From libs/image/kis_paint_device.cc
struct CheckFullyTransparent {
    CheckFullyTransparent(const KoColorSpace *colorSpace)
        : m_colorSpace(colorSpace)
    {
    }

    bool isPixelEmpty(const quint8 *pixelData)
    {
        return m_colorSpace->opacityU8(pixelData) == OPACITY_TRANSPARENT_U8;
    }

private:
    const KoColorSpace *m_colorSpace;
};

struct CheckNonDefault {
    CheckNonDefault(int pixelSize, const quint8 *defaultPixel)
        : m_pixelSize(pixelSize),
          m_defaultPixel(defaultPixel)
    {
    }

    bool isPixelEmpty(const quint8 *pixelData)
    {
        return memcmp(m_defaultPixel, pixelData, m_pixelSize) == 0;
    }

private:
    int m_pixelSize;
    const quint8 *m_defaultPixel;
};

template <class ComparePixelOp>
QVector<QPoint> retrieveAllBoundaryPointsImpl(const KisPaintDevice *device, const QRect &rect, const QRect &skip, ComparePixelOp compareOp)
{
    QVector<QPoint> points;

    KisRandomConstAccessorSP accessor = device->createRandomConstAccessorNG();
    for (qint32 y = rect.top(); y <= rect.bottom(); y++) {
        qint32 maxRight = !skip.isEmpty() && y >= skip.top() && y <= skip.bottom() ? skip.left() : rect.right();
        qint32 x;
        for (x = rect.left(); x <= maxRight; x++) {
            accessor->moveTo(x, y);
            if (!compareOp.isPixelEmpty(accessor->rawDataConst())) {
                points << QPoint(x, y);
                points << QPoint(x, y + 1);
                break;
            }
        }
        
        if (x == rect.right()) continue; // Row empty, don't need to search it backwards

        qint32 minLeft = !skip.isEmpty() && y >= skip.top() && y <= skip.bottom() ? skip.right() : rect.left();
        for (qint32 x = rect.right(); x >= minLeft; x--) {
            accessor->moveTo(x, y);
            if (!compareOp.isPixelEmpty(accessor->rawDataConst())) {
                points << QPoint(x + 1, y);
                points << QPoint(x + 1, y + 1);
                break;
            }
        }
    }
    
    return points;
}
// This matches the behavior of KisPaintDevice::calculateExactBounds(false), whose result is returned by KisPaintDevice::exactBounds()
QVector<QPoint> retrieveAllBoundaryPoints(const KisPaintDevice *device) {
    QRect rect = device->extent();

    quint8 defaultOpacity = device->defaultPixel().opacityU8();
    QVector<QPoint> points;
    if (defaultOpacity != OPACITY_TRANSPARENT_U8) {
        QRect skip = device->defaultBounds()->bounds();
        const KoColor defaultPixel = device->defaultPixel();
        CheckNonDefault compareOp(device->pixelSize(), defaultPixel.data());

        points = retrieveAllBoundaryPointsImpl(device, rect, skip, compareOp);
        if (!skip.isEmpty()) {
            int x, y, w, h;
            skip.getRect(&x, &y, &w, &h);
            points << QPoint(x, y) << QPoint(x + w, y) << QPoint(x + w, y + h) << QPoint(x, y + h);
        }
    } else {
        CheckFullyTransparent compareOp(device->colorSpace());
        points = retrieveAllBoundaryPointsImpl(device, rect, QRect(), compareOp);
    }
    return points;
}
}

namespace KisConvexHull {

QPolygon findConvexHull(const QVector<QPoint> &points)
{
    ENTER_FUNCTION() << ppVar(points.size());
    QPolygon hull = convexHull(points);
    ENTER_FUNCTION() << ppVar(hull.size());
    return hull;
}

QPolygon findConvexHull(KisPaintDeviceSP device)
{
    QElapsedTimer timer;
    timer.start();
    auto ps = retrieveAllBoundaryPoints(device);
    ENTER_FUNCTION() << "found boundary points in" << timer.nsecsElapsed() / 1000;
    auto p = findConvexHull(ps);
    ENTER_FUNCTION() << "found hull in" << timer.nsecsElapsed() / 1000;
    return p;
}

}