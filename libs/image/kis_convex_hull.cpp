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
    int defaultMin = rect.x() + rect.width() + 1;
    int defaultMax = rect.x() - 1;
    QVector<int> minX(rect.height(), defaultMin);
    QVector<int> maxX(rect.height(), defaultMax);
    int base = rect.top();
    if (!skip.isEmpty()) {
        for (int y = skip.top(); y <= skip.bottom(); y++) {
            minX[y - base] = skip.left();
            maxX[y - base] = skip.right();
        }
    }

    int pixelSize = device->pixelSize();
    KisRandomConstAccessorSP accessor = device->createRandomConstAccessorNG();
    for (int y = rect.top(); y <= rect.bottom();) {
        int rows = accessor->numContiguousRows(y);
        for (int x = rect.left(); x <= rect.right();) {
            int columns = accessor->numContiguousColumns(x);
            accessor->moveTo(x, y);
            int strideBytes = accessor->rowStride(x, y);
            const quint8 *data = accessor->rawDataConst();
            for (int r = 0; r < rows; r++) {
                for (int c = 0; c < columns; c++) {
                    if (!compareOp.isPixelEmpty(data + c * pixelSize)) {
                        int index = y + r - base;
                        minX[index] = std::min(minX[index], x + c);
                        maxX[index] = std::max(maxX[index], x + c);
                    }
                }
                data += strideBytes;
            }
            x += columns;
        }
        y += rows;
    }

    for (int y = rect.top(); y <= rect.bottom(); y++) {
        int index = y - base;
        if (minX[index] < defaultMin) {
            points << QPoint(minX[index], y);
            points << QPoint(minX[index], y + 1);
        }
        if (maxX[index] > defaultMax) {
            points << QPoint(maxX[index] + 1, y);
            points << QPoint(maxX[index] + 1, y + 1);
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