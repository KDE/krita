/*
 *  SPDX-FileCopyrightText: 2016 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_lazy_brush_test.h"

#include <simpletest.h>
#include "kis_debug.h"

#include "kis_fill_painter.h"

#include <QImage>
#include <QPainter>

#include <boost/config.hpp>
#include <iostream>
#include <string>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/boykov_kolmogorov_max_flow.hpp>
#include <boost/graph/read_dimacs.hpp>
#include <boost/graph/graph_utility.hpp>

#include "lazybrush/kis_lazy_fill_graph.h"



#if 0

int
doSomething()
{
  using namespace boost;

  typedef adjacency_list_traits < vecS, vecS, directedS > Traits;
  typedef adjacency_list < vecS, vecS, directedS,
    property < vertex_name_t, std::string,
    property < vertex_index_t, long,
    property < vertex_color_t, boost::default_color_type,
    property < vertex_distance_t, long,
    property < vertex_predecessor_t, Traits::edge_descriptor > > > > >,

    property < edge_capacity_t, long,
    property < edge_residual_capacity_t, long,
    property < edge_reverse_t, Traits::edge_descriptor > > > > Graph;

  Graph g;
  property_map < Graph, edge_capacity_t >::type
      capacity = get(edge_capacity, g);
  property_map < Graph, edge_residual_capacity_t >::type
      residual_capacity = get(edge_residual_capacity, g);
  property_map < Graph, edge_reverse_t >::type rev = get(edge_reverse, g);
  Traits::vertex_descriptor s, t;
  read_dimacs_max_flow(g, capacity, rev, s, t);

  std::vector<default_color_type> color(num_vertices(g));
  std::vector<long> distance(num_vertices(g));
  long flow = boykov_kolmogorov_max_flow(g ,s, t);

  std::cout << "c  The total flow:" << std::endl;
  std::cout << "s " << flow << std::endl << std::endl;

  std::cout << "c flow values:" << std::endl;
  graph_traits < Graph >::vertex_iterator u_iter, u_end;
  graph_traits < Graph >::out_edge_iterator ei, e_end;
  for (boost::tie(u_iter, u_end) = vertices(g); u_iter != u_end; ++u_iter) {
      qDebug() << ppVar(get(vertex_color, g)[*u_iter]);


      for (boost::tie(ei, e_end) = out_edges(*u_iter, g); ei != e_end; ++ei) {
          if (capacity[*ei] > 0) {
              std::cout << "f " << *u_iter << " " << target(*ei, g) << " "
                        << (capacity[*ei] - residual_capacity[*ei]) << std::endl;
          }
      }
  }

  return EXIT_SUCCESS;
}

#include <boost/graph/grid_graph.hpp>
#include <boost/graph/boykov_kolmogorov_max_flow.hpp>
#include <boost/graph/iteration_macros.hpp>

template <class Graph>
class ImageToCapacityMap
{
    typedef typename boost::graph_traits<Graph>::vertex_descriptor VertexDescriptor;
    typedef typename boost::graph_traits<Graph>::edge_descriptor EdgeDescriptor;

public:
    typedef EdgeDescriptor key_type;
    typedef float value_type;
    typedef const float& reference;
    typedef boost::readable_property_map_tag category;

    ImageToCapacityMap(Graph &graph, const QImage &image)
        : m_graph(graph), m_image(image)
    {
        m_bits = reinterpret_cast<const QRgb*>(m_image.constBits());
        m_minIntensity = std::numeric_limits<int>::max();
        m_maxIntensity = std::numeric_limits<int>::min();

        int totalInt = 0;
        const int numPixels = m_image.width() * m_image.height();
        m_intensities.resize(numPixels);
        for (int i = 0; i < numPixels; i++) {
            const int v = qGray(m_bits[i]);
            m_intensities[i] = v;
            qDebug() << ppVar(i) << ppVar(v);
            totalInt += v;

            if (m_minIntensity > v) m_minIntensity = v;
            if (m_maxIntensity < v) m_maxIntensity = v;
        }

        qDebug() << ppVar(totalInt);

        m_pixelSize = 4;
        m_rowStride = m_image.width() * m_pixelSize;

        qDebug() << ppVar(m_rowStride) << ppVar(m_image.size()) << ppVar(m_image.bytesPerLine());
    }
    Graph &m_graph;
    QImage m_image;
    QVector<quint8> m_intensities;
    const QRgb *m_bits;
    int m_rowStride;
    int m_pixelSize;
    int m_minIntensity;
    int m_maxIntensity;
};

template <class Graph>
typename ImageToCapacityMap<Graph>::value_type
get(const ImageToCapacityMap<Graph> &map,
    const typename ImageToCapacityMap<Graph>::key_type &key)
{
    typedef typename boost::graph_traits<Graph>::vertex_descriptor VertexDescriptor;

    VertexDescriptor src = source(key, map.m_graph);
    VertexDescriptor tgt = target(key, map.m_graph);
    const int x0 = src[0];
    const int y0 = src[1];
    const int x1 = tgt[0];
    const int y1 = tgt[1];

    const int k  = 2 * (map.m_image.width() + map.m_image.height());
    const int maxDimension = qMax(map.m_image.width(), map.m_image.height());

    //if (x0 <= 5 && x1 <= 5 && y0 == 0 && y1 ==0) return 255;
    //if (x0 >= 5 && x1 >= 5 && y0 == 8 && y1 ==8) return 255;

    //const QRgb *p0 = map.m_bits + y0 * map.m_rowStride + x0 * map.m_pixelSize;
    //const QRgb *p1 = map.m_bits + y1 * map.m_rowStride + x1 * map.m_pixelSize;
    //float value = 255.0 - qAbs(qGray(*p0) - qGray(*p1));

    if ((!x0 && !y0) || (!x1 && !y1) ||
        (x0 == map.m_image.width() - 1 && y0 == map.m_image.height() - 1) ||
        (x1 == map.m_image.width() - 1 && y1 == map.m_image.height() - 1)) {

        qreal value = maxDimension * k;

        qDebug() << x0 << y0 << "->" << x1 << y1 << value;

        return value;
    }

    const int i0 = map.m_intensities[x0 + y0 * map.m_image.width()];
    const int i1 = map.m_intensities[x1 + y1 * map.m_image.width()];

    const int diff = qAbs(i0 - i1);
    qreal normDiff = qreal(diff) / (map.m_maxIntensity - map.m_minIntensity);

    float value = 1.0 + k * (1.0 - normDiff) + qMin(y0, y1);

    qDebug() << x0 << y0 << "->" << x1 << y1 << value;//  << ppVar(normDiff);

    return value;
}

template <class Graph>
ImageToCapacityMap<Graph>
MakeImageToCapacityMap(Graph &graph, const QImage &image) {
    return ImageToCapacityMap<Graph>(graph, image);
}

int doSomethingElse()
{

    const unsigned int D = 2;
    typedef boost::grid_graph<D> Graph;
    typedef boost::graph_traits<Graph>::vertex_descriptor VertexDescriptor;
    typedef boost::graph_traits<Graph>::edge_descriptor EdgeDescriptor;//ADDED
    typedef boost::graph_traits<Graph>::vertices_size_type VertexIndex;
    typedef boost::graph_traits<Graph>::edges_size_type EdgeIndex;


    QImage image(QSize(9,9), QImage::Format_ARGB32);
    QPainter gc(&image);
    gc.fillRect(image.rect(), Qt::white);
    gc.fillRect(QRect(0,4,2,1), Qt::blue);
    //gc.fillRect(QRect(0,5,2,1), Qt::blue);
    gc.fillRect(QRect(6,4,3,1), Qt::blue);
    gc.end();

    image.save("graph_img.png");

    boost::array<std::size_t, D> lengths = { { image.width(), image.height() } };
    Graph graph(lengths, false);

    std::vector<int> groups(num_vertices(graph), 0);
    std::vector<float> residual_capacity(num_edges(graph), 0);
    auto capacityMap = MakeImageToCapacityMap(graph, image);

    float capSum = 0;

    BGL_FORALL_EDGES(e,graph,Graph) {
        VertexDescriptor src = source(e,graph);
        VertexDescriptor tgt = target(e,graph);
        //VertexIndex source_idx = get(boost::vertex_index,graph,src);
        //VertexIndex target_idx = get(boost::vertex_index,graph,tgt);
        //EdgeIndex edge_idx = get(boost::edge_index,graph,e);

        // qDebug() << "(" << src[0] << src[1] << ")"
        //          << "->"
        //          << "(" << tgt[0] << tgt[1] << ")"
        //          << get(capacityMap, e);
        //capSum += get(capacityMap, e);
    }

    //qDebug() << ppVar(capSum);

    BGL_FORALL_VERTICES(v,graph,Graph)
    {
        //qDebug() << ppVar(v[0]) << ppVar(v[1]);

/*      VertexDescriptor src = source(e,graph);
        VertexDescriptor tgt = target(e,graph);
        VertexIndex source_idx = get(boost::vertex_index,graph,src);
        VertexIndex target_idx = get(boost::vertex_index,graph,tgt);
        EdgeIndex edge_idx = get(boost::edge_index,graph,e);

        capacity[edge_idx] = 255.0f - fabs(pixel_intensity[source_idx]-pixel_intensity[target_idx]); //you should change this to your "gradiant or intensity or something"

        reverse_edges[edge_idx]=edge(tgt,src,graph).first;//ADDED
*/
    }


    const int t_index = image.width() * image.height() - 1;
    VertexDescriptor s=vertex(0,graph), t=vertex(t_index,graph);


    float maxFlow =
    boykov_kolmogorov_max_flow(graph,
        capacityMap,
        make_iterator_property_map(&residual_capacity[0], get(boost::edge_index, graph)),
        get(boost::edge_reverse, graph),
        make_iterator_property_map(&groups[0], get(boost::vertex_index, graph)),
        get(boost::vertex_index, graph),
        s,
        t
    );

    qDebug() << ppVar(maxFlow);

    const int cell = 10;
    const int half = cell / 2;
    QImage result(QSize(cell * lengths[0], cell * lengths[1]), QImage::Format_ARGB32);
    QPainter resultPainter(&result);

    BGL_FORALL_VERTICES(v, graph, Graph) {
        const int x = v[0];
        const int y = v[1];

        VertexIndex vertex_idx = get(boost::vertex_index, graph, v);
        int label = groups[vertex_idx];

        QColor color =
            label == 0 ? Qt::blue :
            label == 4 ? Qt::green :
            label == 1 ? Qt::gray :
            Qt::red;

        QRect rc(cell * x, cell * y, cell, cell);
        resultPainter.fillRect(rc, color);
    }

   BGL_FORALL_EDGES(e,graph,Graph) {
        EdgeIndex egdeIndex = get(boost::edge_index, graph, e);
        const int cap = residual_capacity[egdeIndex];


        QColor color(Qt::black);
        if (cap != 0) {
            const int fullCap = get(capacityMap, e);
            const int gray = qreal(cap) / fullCap * 50.0;
            color.setAlpha(gray);
        }

        VertexDescriptor src = source(e,graph);
        VertexDescriptor tgt = target(e,graph);

        QPoint p0(half + cell * src[0], half + cell * src[1]);
        QPoint p1(half + cell * tgt[0], half + cell * tgt[1]);

        resultPainter.setPen(QPen(color, 2));
        resultPainter.drawLine(p0, p1);

        // qDebug() << "(" << src[0] << src[1] << ")"
        //          << "->"
        //          << "(" << tgt[0] << tgt[1] << ")"
        //          << residual_capacity[egdeIndex];
    }

   result.save("result.png");

   return 0;
}

void KisLazyBrushTest::test()
{
    doSomethingElse();
}

#endif /*0*/

bool verifyNormalVertex(const KisLazyFillGraph::vertex_descriptor &v, int x, int y)
{
    bool result = v.type == KisLazyFillGraph::vertex_descriptor::NORMAL &&
        v.x == x && v.y == y;

    if (!result) {
        qDebug() << ppVar(v) << ppVar(x) << ppVar(y);
    }

    return result;
}

bool verifyVertexIndex(KisLazyFillGraph &g,
                       KisLazyFillGraph::vertex_descriptor v,
                       KisLazyFillGraph::vertices_size_type index)
{
    bool result = true;

    const KisLazyFillGraph::vertices_size_type actualIndex = g.index_of(v);
    const KisLazyFillGraph::vertex_descriptor actualVertex = g.vertex_at(index);

    if (index >= 0) {
        result &= v == actualVertex;
    }

    result &= index == actualIndex;

    if (!result) {
        qDebug() << "Vertex failed:";
        qDebug() << v << "->" << actualIndex << "( expected:" << index << ")";
        qDebug() << index << "->" << actualVertex << "( expected:" << v << ")";
    }

    return result;
}

bool verifyEdgeIndex(KisLazyFillGraph &g,
                       KisLazyFillGraph::edge_descriptor v,
                       KisLazyFillGraph::edges_size_type index)
{
    bool result = true;

    const KisLazyFillGraph::edges_size_type actualIndex = g.index_of(v);
    const KisLazyFillGraph::edge_descriptor actualEdge = g.edge_at(index);

    if (index >= 0) {
        result &= v == actualEdge;
    }

    result &= index == actualIndex;

    if (!result) {
        qDebug() << "Edge failed:";
        qDebug() << v << "->" << actualIndex << "( expected:" << index << ")";
        qDebug() << index << "->" << actualEdge << "( expected:" << v << ")";
    }

    return result;
}

void KisLazyBrushTest::testGraph()
{
    QRect mainRect(10, 10, 100, 100);
    QRect aLabelRect(30, 20, 20, 20);
    QRect bLabelRect(60, 60, 20, 20);

    KisLazyFillGraph g(mainRect, aLabelRect, bLabelRect);

    const int numVertices = g.num_vertices();
    const int numEdges = g.num_edges();

    qDebug() << ppVar(numVertices);
    qDebug() << ppVar(numEdges);

    QCOMPARE(numVertices, 10002);
    QCOMPARE(numEdges, 41200);

    for (int i = 0; i < numVertices; i++) {
        KisLazyFillGraph::vertex_descriptor vertex = g.vertex_at(i);
        int newVertexIndex = g.index_of(vertex);
        QCOMPARE(newVertexIndex, i);
    }

    for (int i = 0; i < numEdges; i++) {
        KisLazyFillGraph::edge_descriptor edge = g.edge_at(i);
        int newEdgeIndex = g.index_of(edge);
        QCOMPARE(newEdgeIndex, i);
    }

    KisLazyFillGraph::vertex_descriptor v1(10, 10);
    KisLazyFillGraph::vertex_descriptor v2(11, 10);
    KisLazyFillGraph::vertex_descriptor v3(10, 11);
    KisLazyFillGraph::vertex_descriptor v4(11, 11);
    KisLazyFillGraph::vertex_descriptor v5(12, 10);
    KisLazyFillGraph::vertex_descriptor v6(31, 21);
    KisLazyFillGraph::vertex_descriptor v7(61, 61);
    KisLazyFillGraph::vertex_descriptor v8(9, 10);

    KisLazyFillGraph::vertex_descriptor vA(0, 0, KisLazyFillGraph::vertex_descriptor::LABEL_A);
    KisLazyFillGraph::vertex_descriptor vB(0, 0, KisLazyFillGraph::vertex_descriptor::LABEL_B);

    // Verify vertex index mapping

    QVERIFY(verifyVertexIndex(g, v1, 0));
    QVERIFY(verifyVertexIndex(g, v2, 1));
    QVERIFY(verifyVertexIndex(g, v3, 100));
    QVERIFY(verifyVertexIndex(g, v4, 101));
    QVERIFY(verifyVertexIndex(g, v5, 2));
    QVERIFY(verifyVertexIndex(g, v6, 1121));
    QVERIFY(verifyVertexIndex(g, v7, 5151));
    QVERIFY(verifyVertexIndex(g, v8, -1));

    QVERIFY(verifyVertexIndex(g, vA, numVertices - 2));
    QVERIFY(verifyVertexIndex(g, vB, numVertices - 1));

    // Verify edge index mapping

    QVERIFY(verifyEdgeIndex(g, std::make_pair(v1, v2), 0));
    QVERIFY(verifyEdgeIndex(g, std::make_pair(v3, v4), 99));

    QVERIFY(verifyEdgeIndex(g, std::make_pair(v1, v3), 19800));
    QVERIFY(verifyEdgeIndex(g, std::make_pair(v2, v4), 19801));

    QVERIFY(verifyEdgeIndex(g, std::make_pair(v2, v1), 9900));
    QVERIFY(verifyEdgeIndex(g, std::make_pair(v4, v3), 9999));

    QVERIFY(verifyEdgeIndex(g, std::make_pair(v3, v1), 29700));
    QVERIFY(verifyEdgeIndex(g, std::make_pair(v4, v2), 29701));

    QVERIFY(verifyEdgeIndex(g, std::make_pair(vA, vA), -1));
    QVERIFY(verifyEdgeIndex(g, std::make_pair(vB, vB), -1));
    QVERIFY(verifyEdgeIndex(g, std::make_pair(v1, v8), -1));
    QVERIFY(verifyEdgeIndex(g, std::make_pair(v1, v4), -1));
    QVERIFY(verifyEdgeIndex(g, std::make_pair(v4, v1), -1));
    QVERIFY(verifyEdgeIndex(g, std::make_pair(v1, v5), -1));

    QVERIFY(verifyEdgeIndex(g, std::make_pair(v6, vA), 39621));
    QVERIFY(verifyEdgeIndex(g, std::make_pair(vA, v6), 40021));

    QVERIFY(verifyEdgeIndex(g, std::make_pair(v1, vA), -1));
    QVERIFY(verifyEdgeIndex(g, std::make_pair(v7, vA), -1));
    QVERIFY(verifyEdgeIndex(g, std::make_pair(vA, vB), -1));
    QVERIFY(verifyEdgeIndex(g, std::make_pair(vB, vA), -1));

    QVERIFY(verifyEdgeIndex(g, std::make_pair(v7, vB), 40421));
    QVERIFY(verifyEdgeIndex(g, std::make_pair(vB, v7), 40821));


    QCOMPARE(g.out_degree(v1), long(2));
    QVERIFY(verifyNormalVertex(g.out_edge_at(v1, 0).second, 11, 10));
    QVERIFY(verifyNormalVertex(g.out_edge_at(v1, 1).second, 10, 11));

    QCOMPARE(g.out_degree(v4), long(4));
    QVERIFY(verifyNormalVertex(g.out_edge_at(v4, 0).second, 10, 11));
    QVERIFY(verifyNormalVertex(g.out_edge_at(v4, 1).second, 11, 10));
    QVERIFY(verifyNormalVertex(g.out_edge_at(v4, 2).second, 12, 11));
    QVERIFY(verifyNormalVertex(g.out_edge_at(v4, 3).second, 11, 12));

    QCOMPARE(g.out_degree(v6), long(5));
    QVERIFY(verifyNormalVertex(g.out_edge_at(v6, 0).second, 30, 21));
    QVERIFY(verifyNormalVertex(g.out_edge_at(v6, 1).second, 31, 20));
    QVERIFY(verifyNormalVertex(g.out_edge_at(v6, 2).second, 32, 21));
    QVERIFY(verifyNormalVertex(g.out_edge_at(v6, 3).second, 31, 22));
    QCOMPARE(g.out_edge_at(v6, 4).second, vA);

    QCOMPARE(g.out_degree(v7), long(5));
    QVERIFY(verifyNormalVertex(g.out_edge_at(v7, 0).second, 60, 61));
    QVERIFY(verifyNormalVertex(g.out_edge_at(v7, 1).second, 61, 60));
    QVERIFY(verifyNormalVertex(g.out_edge_at(v7, 2).second, 62, 61));
    QVERIFY(verifyNormalVertex(g.out_edge_at(v7, 3).second, 61, 62));
    QCOMPARE(g.out_edge_at(v7, 4).second, vB);

    QCOMPARE(g.out_degree(vA), long(400));
    QVERIFY(verifyNormalVertex(g.out_edge_at(vA, 0).second, 30, 20));
    QVERIFY(verifyNormalVertex(g.out_edge_at(vA, 1).second, 31, 20));
}

void KisLazyBrushTest::testGraphMultilabel()
{
    QRect mainRect(10, 10, 100, 100);

    QRect aLabelRect1(30, 20, 20, 20);
    QRect aLabelRect2(70, 30, 20, 20);

    QRect bLabelRect1(60, 60, 20, 20);
    QRect bLabelRect2(20, 80, 20, 20);
    QRect bLabelRect3(60, 40, 20, 30);

    QRegion aLabelRegion;
    aLabelRegion += aLabelRect1;
    aLabelRegion += aLabelRect2;

    QRegion bLabelRegion;
    bLabelRegion += bLabelRect1;
    bLabelRegion += bLabelRect2;
    bLabelRegion += bLabelRect3;

    KisLazyFillGraph g(mainRect, KisRegion::fromQRegion(aLabelRegion), KisRegion::fromQRegion(bLabelRegion));

    const int numVertices = g.num_vertices();
    const int numEdges = g.num_edges();

    qDebug() << ppVar(numVertices);
    qDebug() << ppVar(numEdges);

    QCOMPARE(numVertices, 10002);
    QCOMPARE(numEdges, 43600);

    for (int i = 0; i < numVertices; i++) {
        KisLazyFillGraph::vertex_descriptor vertex = g.vertex_at(i);
        int newVertexIndex = g.index_of(vertex);
        QCOMPARE(newVertexIndex, i);
    }

    for (int i = 0; i < numEdges; i++) {
        KisLazyFillGraph::edge_descriptor edge = g.edge_at(i);
        int newEdgeIndex = g.index_of(edge);
        if (i != newEdgeIndex) {
            qDebug() << ppVar(edge);
        }
        QCOMPARE(newEdgeIndex, i);
    }

    typedef KisLazyFillGraph::vertex_descriptor Vert;

    Vert v1(10, 10);
    Vert v2(11, 10);
    Vert v3(10, 11);
    Vert v4(11, 11);
    Vert v5(12, 10);
    Vert v6(31, 21);
    Vert v7(61, 61);
    Vert v8(9, 10);

    Vert vA(0, 0, Vert::LABEL_A);
    Vert vB(0, 0, Vert::LABEL_B);

    // Verify vertex index mapping

    QVERIFY(verifyVertexIndex(g, v1, 0));
    QVERIFY(verifyVertexIndex(g, v2, 1));
    QVERIFY(verifyVertexIndex(g, v3, 100));
    QVERIFY(verifyVertexIndex(g, v4, 101));
    QVERIFY(verifyVertexIndex(g, v5, 2));
    QVERIFY(verifyVertexIndex(g, v6, 1121));
    QVERIFY(verifyVertexIndex(g, v7, 5151));
    QVERIFY(verifyVertexIndex(g, v8, -1));

    QVERIFY(verifyVertexIndex(g, vA, numVertices - 2));
    QVERIFY(verifyVertexIndex(g, vB, numVertices - 1));

    // Verify edge index mapping

    QVERIFY(verifyEdgeIndex(g, std::make_pair(v1, v2), 0));
    QVERIFY(verifyEdgeIndex(g, std::make_pair(v3, v4), 99));

    QVERIFY(verifyEdgeIndex(g, std::make_pair(v1, v3), 19800));
    QVERIFY(verifyEdgeIndex(g, std::make_pair(v2, v4), 19801));

    QVERIFY(verifyEdgeIndex(g, std::make_pair(v2, v1), 9900));
    QVERIFY(verifyEdgeIndex(g, std::make_pair(v4, v3), 9999));

    QVERIFY(verifyEdgeIndex(g, std::make_pair(v3, v1), 29700));
    QVERIFY(verifyEdgeIndex(g, std::make_pair(v4, v2), 29701));

    QVERIFY(verifyEdgeIndex(g, std::make_pair(vA, vA), -1));
    QVERIFY(verifyEdgeIndex(g, std::make_pair(vB, vB), -1));
    QVERIFY(verifyEdgeIndex(g, std::make_pair(v1, v8), -1));
    QVERIFY(verifyEdgeIndex(g, std::make_pair(v1, v4), -1));
    QVERIFY(verifyEdgeIndex(g, std::make_pair(v4, v1), -1));
    QVERIFY(verifyEdgeIndex(g, std::make_pair(v1, v5), -1));

    QVERIFY(verifyEdgeIndex(g, std::make_pair(v6, vA), 39621));
    QVERIFY(verifyEdgeIndex(g, std::make_pair(vA, v6), 40421));

    QVERIFY(verifyEdgeIndex(g, std::make_pair(v1, vA), -1));
    QVERIFY(verifyEdgeIndex(g, std::make_pair(v7, vA), -1));
    QVERIFY(verifyEdgeIndex(g, std::make_pair(vA, vB), -1));
    QVERIFY(verifyEdgeIndex(g, std::make_pair(vB, vA), -1));

    QVERIFY(verifyEdgeIndex(g, std::make_pair(v7, vB), 41621));
    QVERIFY(verifyEdgeIndex(g, std::make_pair(vB, v7), 42821));

    QVERIFY(verifyEdgeIndex(g, std::make_pair(Vert(70, 30), vA), 40000));
    QVERIFY(verifyEdgeIndex(g, std::make_pair(vA, Vert(70, 30)), 40800));

    QVERIFY(verifyEdgeIndex(g, std::make_pair(Vert(70, 30), vB), -1));
    QVERIFY(verifyEdgeIndex(g, std::make_pair(vB, Vert(70, 30)), -1));

    QVERIFY(verifyEdgeIndex(g, std::make_pair(Vert(70, 49), vA), 40380));
    QVERIFY(verifyEdgeIndex(g, std::make_pair(vA, Vert(70, 49)), 41180));

    QVERIFY(verifyEdgeIndex(g, std::make_pair(Vert(70, 49), vB), 41390));
    QVERIFY(verifyEdgeIndex(g, std::make_pair(vB, Vert(70, 49)), 42590));

    QCOMPARE(g.out_degree(v1), long(2));
    QVERIFY(verifyNormalVertex(g.out_edge_at(v1, 0).second, 11, 10));
    QVERIFY(verifyNormalVertex(g.out_edge_at(v1, 1).second, 10, 11));

    QCOMPARE(g.out_degree(v4), long(4));
    QVERIFY(verifyNormalVertex(g.out_edge_at(v4, 0).second, 10, 11));
    QVERIFY(verifyNormalVertex(g.out_edge_at(v4, 1).second, 11, 10));
    QVERIFY(verifyNormalVertex(g.out_edge_at(v4, 2).second, 12, 11));
    QVERIFY(verifyNormalVertex(g.out_edge_at(v4, 3).second, 11, 12));

    QCOMPARE(g.out_degree(v6), long(5));
    QVERIFY(verifyNormalVertex(g.out_edge_at(v6, 0).second, 30, 21));
    QVERIFY(verifyNormalVertex(g.out_edge_at(v6, 1).second, 31, 20));
    QVERIFY(verifyNormalVertex(g.out_edge_at(v6, 2).second, 32, 21));
    QVERIFY(verifyNormalVertex(g.out_edge_at(v6, 3).second, 31, 22));
    QCOMPARE(g.out_edge_at(v6, 4).second, vA);

    QCOMPARE(g.out_degree(v7), long(5));
    QVERIFY(verifyNormalVertex(g.out_edge_at(v7, 0).second, 60, 61));
    QVERIFY(verifyNormalVertex(g.out_edge_at(v7, 1).second, 61, 60));
    QVERIFY(verifyNormalVertex(g.out_edge_at(v7, 2).second, 62, 61));
    QVERIFY(verifyNormalVertex(g.out_edge_at(v7, 3).second, 61, 62));
    QCOMPARE(g.out_edge_at(v7, 4).second, vB);

    QCOMPARE(g.out_degree(vA), long(800));
    QVERIFY(verifyNormalVertex(g.out_edge_at(vA, 0).second, 30, 20));
    QVERIFY(verifyNormalVertex(g.out_edge_at(vA, 1).second, 31, 20));

    // check second island
    QVERIFY(verifyNormalVertex(g.out_edge_at(vA, 400).second, 70, 30));
    QVERIFY(verifyNormalVertex(g.out_edge_at(vA, 401).second, 71, 30));
}

void KisLazyBrushTest::testGraphStandardIterators()
{
    QRect mainRect(10, 10, 100, 100);
    QRect aLabelRect(30, 20, 20, 20);
    QRect bLabelRect(60, 60, 20, 20);

    KisLazyFillGraph g(mainRect, aLabelRect, bLabelRect);

    BGL_FORALL_VERTICES(v, g, KisLazyFillGraph) {
        int index = g.index_of(v);
        QVERIFY(index >= 0);
        KisLazyFillGraph::vertex_descriptor newVertex = g.vertex_at(index);
        QCOMPARE(newVertex, v);
    }

    BGL_FORALL_EDGES(e, g, KisLazyFillGraph) {
        int index = g.index_of(e);
        QVERIFY(index >= 0);
        KisLazyFillGraph::edge_descriptor newEdge = g.edge_at(index);
        QCOMPARE(newEdge, e);
    }
}

void KisLazyBrushTest::testGraphConcepts()
{
    BOOST_CONCEPT_ASSERT(( VertexListGraphConcept<KisLazyFillGraph> )); //to have vertices(), num_vertices(),
    BOOST_CONCEPT_ASSERT(( EdgeListGraphConcept<KisLazyFillGraph> )); //to have edges()
    BOOST_CONCEPT_ASSERT(( IncidenceGraphConcept<KisLazyFillGraph> )); //to have source(), target() and out_edges()
    BOOST_CONCEPT_ASSERT(( AdjacencyGraphConcept<KisLazyFillGraph> )); // to have adjacent_vertices(v, g)
    BOOST_CONCEPT_ASSERT(( AdjacencyMatrixConcept<KisLazyFillGraph> )); // to have edge(u, v, g)
}

template <class Graph>
class ComplexCapacityMap
{
    typedef ComplexCapacityMap<Graph> type;
    typedef typename boost::graph_traits<Graph>::vertex_descriptor VertexDescriptor;
    typedef typename boost::graph_traits<Graph>::edge_descriptor EdgeDescriptor;

public:
    typedef EdgeDescriptor key_type;
    typedef float value_type;
    typedef const float& reference;
    typedef boost::readable_property_map_tag category;

    ComplexCapacityMap(Graph &graph,
                       const QImage &mainImage,
                       const QImage &aLabelImage,
                       const QImage &bLabelImage)
        : m_graph(graph),
          m_mainImage(mainImage),
          m_aLabelImage(aLabelImage),
          m_bLabelImage(bLabelImage)
    {
        const QRgb *bits = reinterpret_cast<const QRgb*>(m_mainImage.constBits());
        m_minIntensity = std::numeric_limits<int>::max();
        m_maxIntensity = std::numeric_limits<int>::min();

        const int numPixels = m_mainImage.width() * m_mainImage.height();
        m_intensities.resize(numPixels);
        for (int i = 0; i < numPixels; i++) {
            const int value = qGray(bits[i]);
            m_intensities[i] = value;

            if (m_minIntensity > value) m_minIntensity = value;
            if (m_maxIntensity < value) m_maxIntensity = value;
        }

        m_pixelSize = 4;
        m_rowStride = m_mainImage.width() * m_pixelSize;
    }

    friend value_type get(const type &map,
                          const key_type &key)
        {
            VertexDescriptor src = source(key, map.m_graph);
            VertexDescriptor dst = target(key, map.m_graph);

            bool srcLabelA = src.type == VertexDescriptor::LABEL_A;
            bool srcLabelB = src.type == VertexDescriptor::LABEL_B;
            bool dstLabelA = dst.type == VertexDescriptor::LABEL_A;
            bool dstLabelB = dst.type == VertexDescriptor::LABEL_B;

            if (srcLabelA || srcLabelB) {
                std::swap(src, dst);
                std::swap(srcLabelA, dstLabelA);
                std::swap(srcLabelB, dstLabelB);
            }

            Q_ASSERT(!srcLabelA && !srcLabelB);

            const int k  = 2 * (map.m_mainImage.width() + map.m_mainImage.height());
            //const int maxDimension = qMax(map.m_mainImage.width(), map.m_mainImage.height());

            float value = 0.0;

            if (dstLabelA) {
                const int xOffset = map.m_aLabelImage.offset().x();
                const int yOffset = map.m_aLabelImage.offset().y();

                const int x = src.x - xOffset;
                const int y = src.y - yOffset;

                //qDebug() << "Label A:"  << ppVar(src.x) << ppVar(src.y) << ppVar(xOffset) << ppVar(yOffset);

                QRgb pixel = map.m_aLabelImage.pixel(x, y);
                const int i0 = qAlpha(pixel);
                value = i0 / 255.0 * k;

            } else if (dstLabelB) {
                const int xOffset = map.m_bLabelImage.offset().x();
                const int yOffset = map.m_bLabelImage.offset().y();

                const int x = src.x - xOffset;
                const int y = src.y - yOffset;

                //qDebug() << "Label B:"  << ppVar(src.x) << ppVar(src.y) << ppVar(xOffset) << ppVar(yOffset);

                QRgb pixel = map.m_bLabelImage.pixel(x, y);
                const int i0 = qAlpha(pixel);
                value = i0 / 255.0 * k;

            } else {
                const int xOffset = map.m_mainImage.offset().x();
                const int yOffset = map.m_mainImage.offset().y();

                const int i0 = map.m_intensities[(src.x - xOffset) + (src.y - xOffset) * map.m_mainImage.width()];
                const int i1 = map.m_intensities[(dst.x - xOffset) + (dst.y - yOffset) * map.m_mainImage.width()];

                const int diff = qAbs(i0 - i1);
                qreal normDiff = qreal(diff) / (map.m_maxIntensity - map.m_minIntensity);

                value = 1.0 + k * (1.0 - normDiff);
            }

            return value;
        }
private:
    Graph &m_graph;
    QImage m_mainImage;
    QImage m_aLabelImage;
    QImage m_bLabelImage;

    QVector<quint8> m_intensities;
    int m_minIntensity;
    int m_maxIntensity;

    int m_rowStride;
    int m_pixelSize;
};

template <class Graph>
ComplexCapacityMap<Graph>
MakeComplexCapacityMap(Graph &graph,
                       const QImage &mainImage,
                       const QImage &aLabelImage,
                       const QImage &bLabelImage) {
    return ComplexCapacityMap<Graph>(graph, mainImage, aLabelImage, bLabelImage);
}

void KisLazyBrushTest::testCutOnGraph()
{
    BOOST_CONCEPT_ASSERT(( ReadablePropertyMapConcept<ComplexCapacityMap<KisLazyFillGraph>, KisLazyFillGraph::edge_descriptor> ));


    QRect mainRect(0, 0, 27, 27);

    const int scribbleASize = 18;
    const int scribbleBSize = 19;
    const int holeSize = 7;
    const int obstacleVOffset = -5;
    const int asymVOffset = 1;


    const int obstacleSize = (mainRect.width() - holeSize) / 2;
    QRect o1Rect(0, mainRect.height() / 2 + obstacleVOffset + asymVOffset, obstacleSize, 1);
    QRect o2Rect(mainRect.width() - obstacleSize, mainRect.height() / 2 + obstacleVOffset - asymVOffset, obstacleSize, 1);

    QRect aLabelRect(0, 0, scribbleASize, 1);
    QRect bLabelRect(3, mainRect.bottom(), scribbleBSize, 1);

    KisLazyFillGraph graph(mainRect, aLabelRect, bLabelRect);

    QImage mainImage(mainRect.size(), QImage::Format_ARGB32);
    {
        QPainter gc(&mainImage);
        gc.fillRect(mainRect, Qt::white);
        gc.fillRect(o1Rect, Qt::blue);
        gc.fillRect(o2Rect, Qt::blue);
    }

    QImage aLabelImage(aLabelRect.size(), QImage::Format_ARGB32);
    {
        QPainter gc(&aLabelImage);
        gc.fillRect(QRect(QPoint(), aLabelRect.size()), Qt::red);
        aLabelImage.setOffset(aLabelRect.topLeft());

        qDebug() << ppVar(aLabelImage.offset());
    }
    QImage bLabelImage(bLabelRect.size(), QImage::Format_ARGB32);
    {
        QPainter gc(&bLabelImage);
        gc.fillRect(QRect(QPoint(), bLabelRect.size()), Qt::red);
        bLabelImage.setOffset(bLabelRect.topLeft());
    }


    std::vector<int> groups(num_vertices(graph), 0);
    std::vector<float> residual_capacity(num_edges(graph), 0);
    auto capacityMap = MakeComplexCapacityMap(graph, mainImage, aLabelImage, bLabelImage);

    std::vector<typename graph_traits<KisLazyFillGraph>::vertices_size_type> distance_vec(num_vertices(graph), 0);
    std::vector<typename graph_traits<KisLazyFillGraph>::edge_descriptor> predecessor_vec(num_vertices(graph));

    auto vertexIndexMap = get(boost::vertex_index, graph);

    typedef KisLazyFillGraph::vertex_descriptor VertexDescriptor;

    VertexDescriptor s(0, 0, VertexDescriptor::LABEL_A);
    VertexDescriptor t(0, 0, VertexDescriptor::LABEL_B);

    float maxFlow =
        boykov_kolmogorov_max_flow(graph,
                                   capacityMap,
                                   make_iterator_property_map(&residual_capacity[0], get(boost::edge_index, graph)),
                                   get(boost::edge_reverse, graph),
                                   make_iterator_property_map(&predecessor_vec[0], vertexIndexMap),
                                   make_iterator_property_map(&groups[0], vertexIndexMap),
                                   make_iterator_property_map(&distance_vec[0], vertexIndexMap),
                                   vertexIndexMap,
                                   s,
                                   t);

    qDebug() << ppVar(maxFlow);

    const int cell = 10;
    const int half = cell / 2;
    QImage result(cell * mainRect.size(), QImage::Format_ARGB32);
    QPainter resultPainter(&result);

    BGL_FORALL_VERTICES(v, graph, KisLazyFillGraph) {
        long vertex_idx = get(boost::vertex_index, graph, v);
        int label = groups[vertex_idx];

        QColor color =
            label == 0 ? Qt::blue :
            label == 4 ? Qt::green :
            label == 1 ? Qt::gray :
            Qt::red;

        QRect rc(cell * v.x, cell * v.y, cell, cell);
        resultPainter.fillRect(rc, color);
    }

    BGL_FORALL_EDGES(e,graph,KisLazyFillGraph) {
        long egdeIndex = get(boost::edge_index, graph, e);
        const int cap = residual_capacity[egdeIndex];


        QColor color(Qt::black);
        if (cap != 0) {
            const int fullCap = get(capacityMap, e);
            const int gray = qreal(cap) / fullCap * 50.0;
            color.setAlpha(gray);
        }

        VertexDescriptor src = source(e,graph);
        VertexDescriptor tgt = target(e,graph);

        QPoint p0(half + cell * src.x, half + cell * src.y);
        QPoint p1(half + cell * tgt.x, half + cell * tgt.y);

        resultPainter.setPen(QPen(color, 2));
        resultPainter.drawLine(p0, p1);

        // qDebug() << "(" << src[0] << src[1] << ")"
        //          << "->"
        //          << "(" << tgt[0] << tgt[1] << ")"
        //          << residual_capacity[egdeIndex];
    }

    resultPainter.save();
    resultPainter.setTransform(QTransform::fromScale(cell, cell));
    resultPainter.setBrush(Qt::transparent);
    resultPainter.setPen(QPen(Qt::yellow, 0));
    resultPainter.drawRect(o1Rect);
    resultPainter.drawRect(o2Rect);
    resultPainter.setPen(QPen(Qt::red, 0));
    resultPainter.drawRect(aLabelRect);
    resultPainter.drawRect(bLabelRect);
    resultPainter.restore();

    result.save("result.png");
}

#include "lazybrush/kis_lazy_fill_capacity_map.h"
#include <KoColorSpaceRegistry.h>
#include <testutil.h>
#include <KoColor.h>


void writeColors(KisLazyFillGraph &graph, const std::vector<int> &groups, KisPaintDeviceSP dst)
{
    KisSequentialIterator dstIt(dst, graph.rect());

    KoColor blue(Qt::blue, dst->colorSpace());
    KoColor green(Qt::red, dst->colorSpace());
    KoColor red(Qt::red, dst->colorSpace());
    KoColor gray(Qt::gray, dst->colorSpace());
    const int pixelSize = dst->colorSpace()->pixelSize();

    while (dstIt.nextPixel()) {
        KisLazyFillGraph::vertex_descriptor v(dstIt.x(), dstIt.y());
        long vertex_idx = get(boost::vertex_index, graph, v);
        int label = groups[vertex_idx];

        KoColor &color =
            label == 0 ? blue :
            label == 4 ? green :
            label == 1 ? gray :
            red;

        quint8 *dstPtr = dstIt.rawData();
        memcpy(dstPtr, color.data(), pixelSize);
    }
}

void writeStat(KisLazyFillGraph &graph,
               const std::vector<int> &groups,
               const std::vector<float> &residual_capacity,
               KisLazyFillCapacityMap &capacityMap)
{

    typedef KisLazyFillGraph::vertex_descriptor VertexDescriptor;

    const int cell = 10;
    const int half = cell / 2;
    QImage result(cell * graph.size(), QImage::Format_ARGB32);
    QPainter resultPainter(&result);

    BGL_FORALL_VERTICES(v, graph, KisLazyFillGraph) {
        if (v.type != VertexDescriptor::NORMAL) continue;

        long vertex_idx = get(boost::vertex_index, graph, v);
        int label = groups[vertex_idx];

        QColor color =
            label == 0 ? Qt::blue :
            label == 4 ? Qt::green :
            label == 1 ? Qt::gray :
            Qt::red;

        QRect rc(cell * v.x, cell * v.y, cell, cell);
        resultPainter.fillRect(rc, color);
    }

    BGL_FORALL_EDGES(e,graph,KisLazyFillGraph) {
        long egdeIndex = get(boost::edge_index, graph, e);
        const int cap = residual_capacity[egdeIndex];
        const int fullCap = get(capacityMap, e);


        QColor color(Qt::red);
        if (cap > 0 || fullCap == 0) continue;
/*
        if (fullCap != 0) {
            const int gray = fullCap / capacityMap.maxCapacity() * 50.0;
            color.setAlpha(gray);
        } else {
            continue;
        }
*/
        VertexDescriptor src = source(e,graph);
        VertexDescriptor tgt = target(e,graph);

        if (src.type != VertexDescriptor::NORMAL ||
            tgt.type != VertexDescriptor::NORMAL) {
/*
            VertexDescriptor &v = src.type != VertexDescriptor::NORMAL ? tgt : src;
            QPoint p0(half + cell * v.x, half + cell * v.y);

            resultPainter.setPen(QPen(color, 4));
            resultPainter.drawEllipse(p0, 0.5 * half, 0.5 * half);
*/
        } else {
            QPoint p0(half + cell * src.x, half + cell * src.y);
            QPoint p1(half + cell * tgt.x, half + cell * tgt.y);

            resultPainter.setPen(QPen(color, 4));
            resultPainter.drawLine(p0, p1);
        }
    }

    BGL_FORALL_EDGES(e,graph,KisLazyFillGraph) {
        long egdeIndex = get(boost::edge_index, graph, e);
        const int cap = residual_capacity[egdeIndex];


        QColor color(Qt::black);
        if (cap != 0) {
            const int fullCap = get(capacityMap, e);
            const int gray = qreal(cap) / fullCap * 50.0;
            color.setAlpha(gray);
        } else {
            continue;
        }

        VertexDescriptor src = source(e,graph);
        VertexDescriptor tgt = target(e,graph);

        if (src.type != VertexDescriptor::NORMAL ||
            tgt.type != VertexDescriptor::NORMAL) {

            VertexDescriptor &v = src.type != VertexDescriptor::NORMAL ? tgt : src;
            QPoint p0(half + cell * v.x, half + cell * v.y);

            resultPainter.setPen(QPen(color, 2));
            resultPainter.drawEllipse(QPointF(p0), 0.5 * half, 0.5 * half);

        } else {
            QPoint p0(half + cell * src.x, half + cell * src.y);
            QPoint p1(half + cell * tgt.x, half + cell * tgt.y);

            resultPainter.setPen(QPen(color, 2));
            resultPainter.drawLine(p0, p1);
        }
    }

    resultPainter.save();
    resultPainter.setTransform(QTransform::fromScale(cell, cell));
    resultPainter.setBrush(Qt::transparent);
    //resultPainter.setPen(QPen(Qt::yellow, 0));
    //resultPainter.drawRect(o1Rect);
    //resultPainter.drawRect(o2Rect);
    //resultPainter.setPen(QPen(Qt::red, 0));
    //resultPainter.drawRect(aLabelRect);
    //resultPainter.drawRect(bLabelRect);
    resultPainter.restore();

    result.save("result.png");
}

#include "kis_paint_device_debug_utils.h"
#include "kis_gaussian_kernel.h"
#include "krita_utils.h"

KisPaintDeviceSP loadTestImage(const QString &name, bool convertToAlpha)
{
    QImage image(TestUtil::fetchDataFileLazy(name));
    KisPaintDeviceSP dev = new KisPaintDevice(KoColorSpaceRegistry::instance()->rgb8());
    dev->convertFromQImage(image, 0);

    if (convertToAlpha) {
        dev = KisPainter::convertToAlphaAsAlpha(dev);
    }

    return dev;
}

#include "lazybrush/kis_lazy_fill_tools.h"
void KisLazyBrushTest::testCutOnGraphDevice()
{
    BOOST_CONCEPT_ASSERT(( ReadablePropertyMapConcept<KisLazyFillCapacityMap, KisLazyFillGraph::edge_descriptor> ));

    KisPaintDeviceSP mainDev = loadTestImage("fill2_main.png", false);
    KisPaintDeviceSP aLabelDev = loadTestImage("fill2_a.png", true);
    KisPaintDeviceSP bLabelDev = loadTestImage("fill2_b.png", true);

    KisPaintDeviceSP filteredMainDev = KisPainter::convertToAlphaAsGray(mainDev);
    const QRect filterRect = filteredMainDev->exactBounds();
    // KisGaussianKernel::applyLoG(filteredMainDev,
    //                             filterRect,
    //                             5,
    //                             QBitArray(), 0);

    // KisLazyFillTools::normalizeAndInvertAlpha8Device(filteredMainDev, filterRect);

    KIS_DUMP_DEVICE_2(filteredMainDev, filterRect, "2filtered", "dd");

    KoColor color(Qt::red, mainDev->colorSpace());
    KisPaintDeviceSP resultColoring = new KisPaintDevice(mainDev->colorSpace());
    KisPaintDeviceSP maskDevice = new KisPaintDevice(KoColorSpaceRegistry::instance()->alpha8());

    maskDevice->fill(QRect(0,0,640,40), KoColor(Qt::gray, maskDevice->colorSpace()));

    KisLazyFillTools::cutOneWay(color,
                                filteredMainDev,
                                aLabelDev,
                                bLabelDev,
                                resultColoring,
                                maskDevice,
                                filterRect);

    KIS_DUMP_DEVICE_2(resultColoring, filterRect, "00result", "dd");
    KIS_DUMP_DEVICE_2(maskDevice, filterRect, "01mask", "dd");
    KIS_DUMP_DEVICE_2(mainDev, filterRect, "1main", "dd");
    KIS_DUMP_DEVICE_2(aLabelDev, filterRect, "3aLabel", "dd");
    KIS_DUMP_DEVICE_2(bLabelDev, filterRect, "4bLabel", "dd");

#if 0
    KisLazyFillCapacityMap capacityMap(filteredMainDev, aLabelDev, bLabelDev);
    KisLazyFillGraph &graph = capacityMap.graph();

    std::vector<int> groups(num_vertices(graph), 0);
    std::vector<float> residual_capacity(num_edges(graph), 0);

    std::vector<typename graph_traits<KisLazyFillGraph>::vertices_size_type> distance_vec(num_vertices(graph), 0);
    std::vector<typename graph_traits<KisLazyFillGraph>::edge_descriptor> predecessor_vec(num_vertices(graph));

    auto vertexIndexMap = get(boost::vertex_index, graph);

    typedef KisLazyFillGraph::vertex_descriptor VertexDescriptor;

    VertexDescriptor s(VertexDescriptor::LABEL_A);
    VertexDescriptor t(VertexDescriptor::LABEL_B);

    float maxFlow =
        boykov_kolmogorov_max_flow(graph,
                                   capacityMap,
                                   make_iterator_property_map(&residual_capacity[0], get(boost::edge_index, graph)),
                                   get(boost::edge_reverse, graph),
                                   make_iterator_property_map(&predecessor_vec[0], vertexIndexMap),
                                   make_iterator_property_map(&groups[0], vertexIndexMap),
                                   make_iterator_property_map(&distance_vec[0], vertexIndexMap),
                                   vertexIndexMap,
                                   s,
                                   t);

    qDebug() << ppVar(maxFlow);

    KisPaintDeviceSP resultColoring = new KisPaintDevice(*mainDev);
    writeColors(graph, groups, resultColoring);

    KIS_DUMP_DEVICE_2(resultColoring, graph.rect(), "0result", "dd");
    KIS_DUMP_DEVICE_2(mainDev, graph.rect(), "1main", "dd");
    KIS_DUMP_DEVICE_2(aLabelDev, graph.rect(), "3aLabel", "dd");
    KIS_DUMP_DEVICE_2(bLabelDev, graph.rect(), "4bLabel", "dd");

    writeStat(graph,
              groups,
              residual_capacity,
              capacityMap);
#endif
}

#include "lazybrush/kis_multiway_cut.h"
#include "testing_timed_default_bounds.h"

void KisLazyBrushTest::testCutOnGraphDeviceMulti()
{
    BOOST_CONCEPT_ASSERT(( ReadablePropertyMapConcept<KisLazyFillCapacityMap, KisLazyFillGraph::edge_descriptor> ));

    KisPaintDeviceSP mainDev = loadTestImage("fill4_main.png", false);
    KisPaintDeviceSP aLabelDev = loadTestImage("fill4_a.png", true);
    KisPaintDeviceSP bLabelDev = loadTestImage("fill4_b.png", true);
    KisPaintDeviceSP cLabelDev = loadTestImage("fill4_c.png", true);
    KisPaintDeviceSP dLabelDev = loadTestImage("fill4_d.png", true);
    KisPaintDeviceSP eLabelDev = loadTestImage("fill4_e.png", true);

    KisPaintDeviceSP filteredMainDev = KisPainter::convertToAlphaAsGray(mainDev);
    KisDefaultBoundsBaseSP bounds = new TestUtil::TestingTimedDefaultBounds(mainDev->exactBounds());
    mainDev->setDefaultBounds(bounds);
    filteredMainDev->setDefaultBounds(bounds);

    const QRect filterRect = filteredMainDev->exactBounds();
    KisGaussianKernel::applyLoG(filteredMainDev,
                                filterRect,
                                2,
                                1.0,
                                QBitArray(), 0);

    KisLazyFillTools::normalizeAndInvertAlpha8Device(filteredMainDev, filterRect);


    KisPaintDeviceSP resultColoring = new KisPaintDevice(mainDev->colorSpace());

    KisMultiwayCut cut(filteredMainDev, resultColoring, filterRect);

    cut.addKeyStroke(aLabelDev, KoColor(Qt::red, mainDev->colorSpace()));
    cut.addKeyStroke(bLabelDev, KoColor(Qt::green, mainDev->colorSpace()));
    cut.addKeyStroke(cLabelDev, KoColor(Qt::blue, mainDev->colorSpace()));
    cut.addKeyStroke(dLabelDev, KoColor(Qt::yellow, mainDev->colorSpace()));
    cut.addKeyStroke(eLabelDev, KoColor(Qt::magenta, mainDev->colorSpace()));

    cut.run();


    KIS_DUMP_DEVICE_2(resultColoring, filterRect, "00result", "dd");
    KIS_DUMP_DEVICE_2(mainDev, filterRect, "1main", "dd");
    KIS_DUMP_DEVICE_2(filteredMainDev, filterRect, "2filtered", "dd");
}

void KisLazyBrushTest::testLoG()
{
    QImage mainImage(TestUtil::fetchDataFileLazy("fill1_main.png"));
     QVERIFY(!mainImage.isNull());
     KisPaintDeviceSP mainDev = new KisPaintDevice(KoColorSpaceRegistry::instance()->rgb8());
     mainDev->convertFromQImage(mainImage, 0);
     QRect rect = mainDev->exactBounds();

    // KisPaintDeviceSP mainDev = new KisPaintDevice(KoColorSpaceRegistry::instance()->rgb8());
    // const QRect rect(0,0,10,10);
    // const QRect fillRect(0,0,5,10);
    // KoColor bg(Qt::white, mainDev->colorSpace());
    // KoColor fg(Qt::black, mainDev->colorSpace());
    // mainDev->fill(rect, bg);
    // mainDev->fill(fillRect, fg);

    KisPaintDeviceSP filteredMainDev = KisPainter::convertToAlphaAsGray(mainDev);

    KisDefaultBoundsBaseSP bounds = new TestUtil::TestingTimedDefaultBounds(mainDev->exactBounds());
    mainDev->setDefaultBounds(bounds);
    filteredMainDev->setDefaultBounds(bounds);

    KisGaussianKernel::applyLoG(filteredMainDev,
                                rect,
                                4.0,
                                1.0,
                                QBitArray(), 0);

    KisLazyFillTools::normalizeAndInvertAlpha8Device(filteredMainDev, rect);



    KIS_DUMP_DEVICE_2(mainDev, rect, "1main", "dd");
    KIS_DUMP_DEVICE_2(filteredMainDev, rect, "2filtered", "dd");
}

void KisLazyBrushTest::testSplitIntoConnectedComponents()
{
    const QRect rc1(10, 10, 10, 10);
    const QRect rc2(30, 10, 10, 10);
    const QRect boundingRect(0,0,100,100);

    KisPaintDeviceSP dev = new KisPaintDevice(KoColorSpaceRegistry::instance()->rgb8());

    dev->fill(rc1, KoColor(Qt::red, dev->colorSpace()));
    dev->fill(rc2, KoColor(Qt::green, dev->colorSpace()));

    QCOMPARE(dev->exactBounds(), rc1 | rc2);

    QVector<QPoint> points =
        KisLazyFillTools::splitIntoConnectedComponents(dev, boundingRect);

    qDebug() << ppVar(points);

    QCOMPARE(points.size(), 2);
    QCOMPARE(points[0], QPoint(10,10));
    QCOMPARE(points[1], QPoint(30,10));
}

void KisLazyBrushTest::testEstimateTransparentPixels()
{
    KisPaintDeviceSP dev = new KisPaintDevice(KoColorSpaceRegistry::instance()->rgb8());

    const QRect totalRect(0,0,50,100);

    qreal value = 0.0;

    value = KritaUtils::estimatePortionOfTransparentPixels(dev, totalRect, 0.1);
    QCOMPARE(value, 1.0);

    dev->fill(QRect(0,0,25,50), KoColor(Qt::red, dev->colorSpace()));
    value = KritaUtils::estimatePortionOfTransparentPixels(dev, totalRect, 0.1);
    QVERIFY(qAbs(value - 0.75) < 0.05);

    dev->fill(QRect(25,0,25,50), KoColor(Qt::green, dev->colorSpace()));
    value = KritaUtils::estimatePortionOfTransparentPixels(dev, totalRect, 0.1);
    QVERIFY(qAbs(value - 0.5) < 0.05);

    dev->fill(QRect(25,50,25,50), KoColor(Qt::blue, dev->colorSpace()));
    value = KritaUtils::estimatePortionOfTransparentPixels(dev, totalRect, 0.1);
    QVERIFY(qAbs(value - 0.25) < 0.05);

    dev->fill(QRect(0,50,25,50), KoColor(Qt::blue, dev->colorSpace()));
    value = KritaUtils::estimatePortionOfTransparentPixels(dev, totalRect, 0.1);
    QCOMPARE(value, 0.0);
}

void KisLazyBrushTest::multiwayCutBenchmark()
{
    BOOST_CONCEPT_ASSERT(( ReadablePropertyMapConcept<KisLazyFillCapacityMap, KisLazyFillGraph::edge_descriptor> ));

    const KoColor fillColor(Qt::black, KoColorSpaceRegistry::instance()->rgb8());
    KisPaintDeviceSP mainDev = new KisPaintDevice(KoColorSpaceRegistry::instance()->rgb8());

    QRect mainRect(0,0,512,512);

    QPainterPath path;
    path.moveTo(100, 100);
    path.lineTo(400, 100);
    path.lineTo(400, 400);
    path.lineTo(100, 400);
    path.lineTo(100, 120);

    KisFillPainter gc(mainDev);
    gc.setPaintColor(fillColor);
    gc.drawPainterPath(path, QPen(Qt::white, 10));
    gc.fillRect(QRect(250, 100, 15, 120), fillColor);
    gc.fillRect(QRect(250, 280, 15, 120), fillColor);
    gc.fillRect(QRect(100, 250, 120, 15), fillColor);
    gc.fillRect(QRect(280, 250, 120, 15), fillColor);

    //KIS_DUMP_DEVICE_2(mainDev, mainRect, "1main", "dd");

    KisPaintDeviceSP aLabelDev = new KisPaintDevice(KoColorSpaceRegistry::instance()->alpha8());
    aLabelDev->fill(QRect(110, 110, 30,30), KoColor(Qt::black, KoColorSpaceRegistry::instance()->alpha8()));

    KisPaintDeviceSP bLabelDev = new KisPaintDevice(KoColorSpaceRegistry::instance()->alpha8());
    bLabelDev->fill(QRect(370, 110, 20,20), KoColor(Qt::black, KoColorSpaceRegistry::instance()->alpha8()));

    KisPaintDeviceSP cLabelDev = new KisPaintDevice(KoColorSpaceRegistry::instance()->alpha8());
    cLabelDev->fill(QRect(370, 370, 20,20), KoColor(Qt::black, KoColorSpaceRegistry::instance()->alpha8()));

    KisPaintDeviceSP dLabelDev = new KisPaintDevice(KoColorSpaceRegistry::instance()->alpha8());
    dLabelDev->fill(QRect(110, 370, 20,20), KoColor(Qt::black, KoColorSpaceRegistry::instance()->alpha8()));

    KisPaintDeviceSP eLabelDev = new KisPaintDevice(KoColorSpaceRegistry::instance()->alpha8());
    eLabelDev->fill(QRect(0, 0, 200,20), KoColor(Qt::black, KoColorSpaceRegistry::instance()->alpha8()));

    KisPaintDeviceSP filteredMainDev = KisPainter::convertToAlphaAsAlpha(mainDev);
    KisLazyFillTools::normalizeAndInvertAlpha8Device(filteredMainDev, mainRect);

    KisPaintDeviceSP resultColoring = new KisPaintDevice(mainDev->colorSpace());

    KisMultiwayCut cut(filteredMainDev, resultColoring, mainRect);

    cut.addKeyStroke(aLabelDev, KoColor(Qt::red, mainDev->colorSpace()));
    cut.addKeyStroke(bLabelDev, KoColor(Qt::green, mainDev->colorSpace()));
    cut.addKeyStroke(cLabelDev, KoColor(Qt::blue, mainDev->colorSpace()));
    cut.addKeyStroke(dLabelDev, KoColor(Qt::yellow, mainDev->colorSpace()));
    cut.addKeyStroke(eLabelDev, KoColor(Qt::transparent, mainDev->colorSpace()));


    QBENCHMARK_ONCE {
        cut.run();
    }


    // KIS_DUMP_DEVICE_2(resultColoring, mainRect, "00result", "dd");
    // KIS_DUMP_DEVICE_2(mainDev, mainRect, "1main", "dd");
    // KIS_DUMP_DEVICE_2(filteredMainDev, mainRect, "2filtered", "dd");
}



SIMPLE_TEST_MAIN(KisLazyBrushTest)
