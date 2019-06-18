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

#include <QtCore>
#include <QPolygon>

#include <boost/graph/astar_search.hpp>

#include "KisMagneticGraph.h"

struct DistanceMap {
    typedef VertexDescriptor key_type;
    typedef double data_type;
    typedef std::pair<key_type, data_type> value_type;

    DistanceMap(double const& dval)
            : m_default(dval)
    { }

    data_type &operator[](key_type const& k) {
        if (m.find(k) == m.end())
            m[k] = m_default;
        return m[k];
    }

private:
    std::map<key_type, data_type> m;
    data_type const m_default;
};

struct PredecessorMap{
    PredecessorMap()
    { }

    PredecessorMap(PredecessorMap const& that):
    m_map(that.m_map)
    { }

    typedef VertexDescriptor key_type;
    typedef VertexDescriptor value_type;
    typedef VertexDescriptor & reference_type;
    typedef boost::read_write_property_map_tag category;

    VertexDescriptor &operator[](VertexDescriptor v){
        return m_map[v];
    }

    std::map<VertexDescriptor, VertexDescriptor> m_map;
};

VertexDescriptor get(PredecessorMap const &m, VertexDescriptor v){
    typename std::map<VertexDescriptor, VertexDescriptor>::const_iterator found = m.m_map.find(v);
    return found != m.m_map.end() ? found->second : v;
}

void put(PredecessorMap &m, VertexDescriptor key, VertexDescriptor value){
    m.m_map[key] = value;
}

double EuclideanDistance(VertexDescriptor p1, VertexDescriptor p2){
    return std::sqrt(std::pow(p1.y-p2.y, 2) + std::pow(p1.x-p2.x, 2));
}

class AStarHeuristic : public boost::astar_heuristic<KisMagneticGraph, double> {
    private:
        VertexDescriptor m_goal;
        double coeff_a, coeff_b;

    public:
        AStarHeuristic(VertexDescriptor goal, double a, double b):
            m_goal(goal), coeff_a(a), coeff_b(b)
        { }

        AStarHeuristic(VertexDescriptor goal):
            m_goal(goal), coeff_a(0.5), coeff_b(0.5)
        { }

        double operator()(VertexDescriptor v){
            return EuclideanDistance(v,m_goal);
        }
};

struct GoalFound {};

class AStarGoalVisitor : public boost::default_astar_visitor {
    public:
        AStarGoalVisitor(VertexDescriptor goal) : m_goal(goal) { }

        void examine_vertex(VertexDescriptor u, KisMagneticGraph const &g) {
            Q_UNUSED(g)
            if(u == m_goal){
                throw GoalFound();
            }
        }

    private:
        VertexDescriptor m_goal;
};

struct WeightMap{
    typedef std::pair<VertexDescriptor, VertexDescriptor> key_type;
    typedef double data_type;
    typedef std::pair<key_type, data_type> value_type;

    WeightMap() { }

    WeightMap(KisMagneticGraph g):
        m_graph(g)
    { }

    data_type& operator[](key_type const& k) {
        if (m_map.find(k) == m_map.end()) {
            double edge_gradient = (m_graph.getIntensity(k.first) + m_graph.getIntensity(k.second))/2;
            m_map[k] = EuclideanDistance(k.first, k.second) * (edge_gradient + 1);
        }
        return m_map[k];
    }

private:
    std::map<key_type, data_type> m_map;
    KisMagneticGraph m_graph;
};

QRect KisMagneticWorker::calculateRect(QPoint p1, QPoint p2, int radius) const {
    // I am sure there is a simpler version of it which exists but well

    double slope = (p2.y() - p1.y())/(p2.x() - p1.x());
    QPoint a,b,c,d;
    if(slope != 0){
        slope = -1/slope;
        double denom = 2 * std::sqrt(slope*slope+1);
        double numer = radius/denom;
        double fac1 = numer/denom;
        denom = 2 * denom;
        numer = 3 * slope * numer;
        double fac2 = numer/denom;
        a = QPoint(p1.x() - fac1, p1.y() - fac2);
        b = QPoint(p1.x() + fac1, p1.y() + fac2);
        c = QPoint(p2.x() - fac1, p2.y() - fac2);
        d = QPoint(p2.x() + fac1, p2.y() + fac2);
    }else{
        double fac = radius/2;
        a = QPoint(p1.x() - fac, p1.y() - fac);
        b = QPoint(p1.x() + fac, p1.y() + fac);
        c = QPoint(p2.x() - fac, p2.y() - fac);
        d = QPoint(p2.x() + fac, p2.y() + fac);
    }

    QPolygon p(QVector<QPoint>{a,b,c,d});
    return p.boundingRect();
}

QVector<QPointF> KisMagneticWorker::computeEdge(KisPaintDeviceSP dev, int radius, QPoint begin, QPoint end) {

    QRect rect = calculateRect(begin, end, radius);
    KisGaussianKernel::applyLoG(dev, rect, 2, 1.0, QBitArray(), 0);
    KisLazyFillTools::normalizeAndInvertAlpha8Device(dev, rect);

    VertexDescriptor goal(end);
    VertexDescriptor start(begin);

    KisMagneticGraph g(dev, rect);

    // How many maps does it require?
    PredecessorMap pmap;
    DistanceMap dmap(std::numeric_limits<double>::max());
    dmap[start] = 0;
    std::map<VertexDescriptor, double> rmap;
    std::map<VertexDescriptor, boost::default_color_type> cmap;
    std::map<VertexDescriptor, double> imap;
    WeightMap wmap(g);
    AStarHeuristic heuristic(goal);
    QVector<QPointF> result;

    try{
        boost::astar_search_no_init(
                    g, start, heuristic
                    ,boost::visitor(AStarGoalVisitor(goal))
                    .distance_map(boost::associative_property_map<DistanceMap>(dmap))
                    .predecessor_map(boost::ref(pmap))
                    .weight_map(boost::associative_property_map<WeightMap>(wmap))
                    .vertex_index_map(boost::associative_property_map<std::map<VertexDescriptor, double>>(imap))
                    .rank_map(boost::associative_property_map<std::map<VertexDescriptor, double>>(rmap))
                    .color_map(boost::associative_property_map<std::map<VertexDescriptor, boost::default_color_type>>(cmap))
                    .distance_combine(std::plus<double>())
                    .distance_compare(std::less<double>())
                    );

    }catch(GoalFound const&){
        for(VertexDescriptor u=goal; u!=start; u = pmap[u]){
            result.push_back(QPointF(u.x,u.y));
            //qDebug() << g.getIntensity(u);
        }
    }

    result.push_back(QPoint(start.x,start.y));

    return result;
}
