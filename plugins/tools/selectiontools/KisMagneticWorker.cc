/*
 *  Copyright (c) 2019 Kuntal Majumder <hellozee@disroot.org>
 *
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2.1 of the License.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "KisMagneticWorker.h"

#include <kis_gaussian_kernel.h>
#include <lazybrush/kis_lazy_fill_tools.h>
#include <kis_algebra_2d.h>
#include <kis_painter.h>

#include <QtCore>
#include <QPolygon>
#include <QPainter>

#include <boost/graph/astar_search.hpp>
#include <krita_utils.h>

#include "KisMagneticGraph.h"

struct DistanceMap {
    typedef VertexDescriptor key_type;
    typedef double data_type;
    typedef std::pair<key_type, data_type> value_type;

    explicit DistanceMap(double const &dval)
        : m_default(dval)
    { }

    data_type &operator [] (key_type const &k)
    {
        if (m.find(k) == m.end())
            m[k] = m_default;
        return m[k];
    }

private:
    std::map<key_type, data_type> m;
    data_type const               m_default;
};

struct PredecessorMap {
    PredecessorMap() = default;

    PredecessorMap(PredecessorMap const &that) = default;

    typedef VertexDescriptor key_type;
    typedef VertexDescriptor value_type;
    typedef boost::read_write_property_map_tag category;

    VertexDescriptor &operator [] (VertexDescriptor v)
    {
        return m_map[v];
    }

    std::map<VertexDescriptor, VertexDescriptor> m_map;
};

VertexDescriptor get(PredecessorMap const &m, VertexDescriptor v)
{
    auto found = m.m_map.find(v);
    return found != m.m_map.end() ? found->second : v;
}

void put(PredecessorMap &m, VertexDescriptor key, VertexDescriptor value)
{
    m.m_map[key] = value;
}

double EuclideanDistance(VertexDescriptor p1, VertexDescriptor p2)
{
    return std::sqrt(std::pow(p1.y - p2.y, 2) + std::pow(p1.x - p2.x, 2));
}

class AStarHeuristic : public boost::astar_heuristic<KisMagneticGraph, double>
{
private:
    VertexDescriptor m_goal;

public:
    explicit AStarHeuristic(VertexDescriptor goal) :
        m_goal(goal)
    { }

    double operator () (VertexDescriptor v)
    {
        return EuclideanDistance(v, m_goal);
    }
};

struct GoalFound { };

class AStarGoalVisitor : public boost::default_astar_visitor
{
public:
    explicit AStarGoalVisitor(VertexDescriptor goal) : m_goal(goal){ }

    void examine_vertex(VertexDescriptor u, KisMagneticGraph const &g)
    {
        Q_UNUSED(g)
        if (u == m_goal) {
            throw GoalFound();
        }
    }

private:
    VertexDescriptor m_goal;
};

struct WeightMap {
    typedef std::pair<VertexDescriptor, VertexDescriptor> key_type;
    typedef double data_type;
    typedef std::pair<key_type, data_type> value_type;

    WeightMap() = default;

    explicit WeightMap(const KisMagneticGraph &g) :
        m_graph(g)
    { }

    data_type &operator [] (key_type const &k)
    {
        if (m_map.find(k) == m_map.end()) {
            double edge_gradient = (m_graph.getIntensity(k.first) + m_graph.getIntensity(k.second)) / 2;
            m_map[k] = EuclideanDistance(k.first, k.second) + 255.0 - edge_gradient;
        }
        return m_map[k];
    }

private:
    std::map<key_type, data_type> m_map;
    KisMagneticGraph              m_graph;
};

KisMagneticLazyTiles::KisMagneticLazyTiles(KisPaintDeviceSP dev)
{
    m_dev = KisPainter::convertToAlphaAsGray(dev);
    QSize s = m_dev->exactBounds().size();
    m_tileSize    = KritaUtils::optimalPatchSize();
    m_tilesPerRow = (int) std::ceil((double) s.width() / (double) m_tileSize.width());
    int tilesPerColumn = (int) std::ceil((double) s.height() / (double) m_tileSize.height());
    m_dev->setDefaultBounds(dev->defaultBounds());

    for (int i = 0; i < tilesPerColumn; i++) {
        for (int j = 0; j < m_tilesPerRow; j++) {
            int width  = std::min(m_dev->exactBounds().width() - j * m_tileSize.width(), m_tileSize.width());
            int height = std::min(m_dev->exactBounds().height() - i * m_tileSize.height(), m_tileSize.height());
            QRect temp(j *m_tileSize.width(), i *m_tileSize.height(), width, height);
            m_tiles.push_back(temp);
        }
    }
    m_radiusRecord = QVector<qreal>(m_tiles.size(), -1);
}

void KisMagneticLazyTiles::filter(qreal radius, QRect &rect)
{
    auto divide = [](QPoint p, QSize s){
                      return QPoint(p.x() / s.width(), p.y() / s.height());
                  };

    QPoint firstTile = divide(rect.topLeft(), m_tileSize);
    QPoint lastTile  = divide(rect.bottomRight(), m_tileSize);
    for (int i = firstTile.y(); i <= lastTile.y(); i++) {
        for (int j = firstTile.x(); j <= lastTile.x(); j++) {
            int currentTile = i * m_tilesPerRow + j;
            if (radius != m_radiusRecord[currentTile]) {
                QRect bounds = m_tiles[currentTile];
                KisGaussianKernel::applyTightLoG(m_dev, bounds, radius, -1.0, QBitArray(), nullptr);
                KisLazyFillTools::normalizeAlpha8Device(m_dev, bounds);
                m_radiusRecord[currentTile] = radius;
            }
        }
    }
}

KisMagneticWorker::KisMagneticWorker(const KisPaintDeviceSP &dev) :
    m_lazyTileFilter(dev)
{ }

QVector<QPointF> KisMagneticWorker::computeEdge(int bounds, QPoint begin, QPoint end, qreal radius)
{
    QRect rect;
    KisAlgebra2D::accumulateBounds(QVector<QPoint> { begin, end }, &rect);
    rect = kisGrowRect(rect, bounds) & m_lazyTileFilter.device()->exactBounds();

    m_lazyTileFilter.filter(radius, rect);

    QPoint maxPoint = rect.bottomRight();

    begin.setX(std::min(begin.x(), maxPoint.x()));
    begin.setY(std::min(begin.y(), maxPoint.y()));
    end.setX(std::min(end.x(), maxPoint.x()));
    end.setY(std::min(end.y(), maxPoint.y()));

    VertexDescriptor goal(end);
    VertexDescriptor start(begin);

    m_graph = new KisMagneticGraph(m_lazyTileFilter.device(), rect);

    // How many maps does it require?
    // Take a look here, if it doesn't make sense, https://www.boost.org/doc/libs/1_70_0/libs/graph/doc/astar_search.html
    PredecessorMap pmap;
    DistanceMap dmap(std::numeric_limits<double>::max());
    dmap[start] = 0;
    std::map<VertexDescriptor, double> rmap;
    std::map<VertexDescriptor, boost::default_color_type> cmap;
    std::map<VertexDescriptor, double> imap;
    WeightMap wmap(*m_graph);
    AStarHeuristic heuristic(goal);
    QVector<QPointF> result;

    try {
        boost::astar_search_no_init(
            *m_graph, start, heuristic,
            boost::visitor(AStarGoalVisitor(goal))
            .distance_map(boost::associative_property_map<DistanceMap>(dmap))
            .predecessor_map(boost::ref(pmap))
            .weight_map(boost::associative_property_map<WeightMap>(wmap))
            .vertex_index_map(boost::associative_property_map<std::map<VertexDescriptor, double> >(imap))
            .rank_map(boost::associative_property_map<std::map<VertexDescriptor, double> >(rmap))
            .color_map(boost::associative_property_map<std::map<VertexDescriptor, boost::default_color_type> >
                           (cmap))
            .distance_combine(std::plus<double>())
            .distance_compare(std::less<double>())
            );
    } catch (GoalFound const &) {
        for (VertexDescriptor u = goal; u != start; u = pmap[u]) {
            result.push_front(QPointF(u.x, u.y));
        }
    }

    result.push_front(QPoint(start.x, start.y));

    return result;
} // KisMagneticWorker::computeEdge

qreal KisMagneticWorker::intensity(QPoint pt)
{
    return m_graph->getIntensity(VertexDescriptor(pt));
}

void KisMagneticWorker::saveTheImage(vQPointF points)
{
    QImage img = m_lazyTileFilter.device()->convertToQImage(nullptr, m_lazyTileFilter.device()->exactBounds());

    const QPointF offset = m_lazyTileFilter.device()->exactBounds().topLeft();
    for (QPointF &pt : points) {
        pt -= offset;
    }

    img = img.convertToFormat(QImage::Format_ARGB32);
    QPainter gc(&img);

    QPainterPath path;

    for (int i = 0; i < points.size(); i++) {
        if (i == 0) {
            path.moveTo(points[i]);
        } else {
            path.lineTo(points[i]);
        }
    }

    gc.setPen(Qt::blue);
    gc.drawPath(path);

    gc.setPen(Qt::green);
    gc.drawEllipse(points[0], 3, 3);
    gc.setPen(Qt::red);
    gc.drawEllipse(points[points.count() - 1], 2, 2);

    for (QRect &r : m_lazyTileFilter.tiles() ) {
        gc.drawRect(r);
    }

    img.save("result.png");
} // KisMagneticWorker::saveTheImage
