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

#include "KisMagneticGraph.h"

struct DistanceMap {
    typedef VertexDescriptor key_type;
    typedef double data_type;
    typedef std::pair<key_type, data_type> value_type;

    explicit DistanceMap(double const& dval)
        : m_default(dval)
    { }

    data_type &operator [] (key_type const& k)
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

    PredecessorMap(PredecessorMap const& that) = default;

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

class AStarHeuristic : public boost::astar_heuristic<KisMagneticGraph, double> {
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

class AStarGoalVisitor : public boost::default_astar_visitor {
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

    data_type& operator [] (key_type const& k)
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

KisMagneticWorker::KisMagneticWorker(const KisPaintDeviceSP& dev, qreal radius)
{
    m_dev = KisPainter::convertToAlphaAsGray(dev);
    KisPainter::copyAreaOptimized(dev->exactBounds().topLeft(), dev, m_dev, dev->exactBounds());
    KisGaussianKernel::applyTightLoG(m_dev, m_dev->exactBounds(), radius, -1.0, QBitArray(), nullptr);
    KisLazyFillTools::normalizeAlpha8Device(m_dev, m_dev->exactBounds());

    m_graph = new KisMagneticGraph(m_dev);
}

QVector<QPointF> KisMagneticWorker::computeEdge(int radius, QPoint begin, QPoint end)
{
    QRect rect;
    KisAlgebra2D::accumulateBounds(QVector<QPoint> { begin, end }, &rect);
    rect = kisGrowRect(rect, radius);

    VertexDescriptor goal(end);
    VertexDescriptor start(begin);
    m_graph->m_rect = rect;

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

    try{
        boost::astar_search_no_init(
            *m_graph, start, heuristic,
            boost::visitor(AStarGoalVisitor(goal))
            .distance_map(boost::associative_property_map<DistanceMap>(dmap))
            .predecessor_map(boost::ref(pmap))
            .weight_map(boost::associative_property_map<WeightMap>(wmap))
            .vertex_index_map(boost::associative_property_map<std::map<VertexDescriptor, double> >(imap))
            .rank_map(boost::associative_property_map<std::map<VertexDescriptor, double> >(rmap))
            .color_map(boost::associative_property_map<std::map<VertexDescriptor, boost::default_color_type> >(cmap))
            .distance_combine(std::plus<double>())
            .distance_compare(std::less<double>())
            );
    }catch (GoalFound const&) {
        for (VertexDescriptor u = goal; u != start; u = pmap[u]) {
            result.push_front(QPointF(u.x, u.y));
        }
    }

    result.push_front(QPoint(start.x, start.y));

    return result;
} // KisMagneticWorker::computeEdge

void KisMagneticWorker::saveTheImage(vQPointF points)
{
    QImage img = m_dev->convertToQImage(0, m_dev->exactBounds());

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
    gc.drawEllipse(points[points.count() -1], 2, 2);

    img.save("result.png");
}

quint8 KisMagneticWorker::intensity(QPoint pt)
{
    return m_graph->getIntensity(VertexDescriptor(pt));
}
