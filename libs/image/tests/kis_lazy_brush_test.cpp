/*
 *  Copyright (c) 2016 Dmitry Kazakov <dimula73@gmail.com>
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

#include "kis_lazy_brush_test.h"

#include <QTest>
#include "kis_debug.h"

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

bool verifyNormalVertex(const KisLazyFillGraph::vertex_descriptor &v, int x, int y)
{
    bool result = v.type == KisLazyFillGraph::vertex_descriptor::NORMAL &&
        v.x == x && v.y == y;

    if (!result) {
        qDebug() << ppVar(v) << ppVar(x) << ppVar(y);
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

    QCOMPARE(g.index_of(std::make_pair(v1, v2)), long(0));
    QCOMPARE(g.index_of(std::make_pair(v3, v4)), long(99));

    QCOMPARE(g.index_of(std::make_pair(v1, v3)), long(19800));
    QCOMPARE(g.index_of(std::make_pair(v2, v4)), long(19801));

    QCOMPARE(g.index_of(std::make_pair(v2, v1)), long(9900));
    QCOMPARE(g.index_of(std::make_pair(v4, v3)), long(9999));

    QCOMPARE(g.index_of(std::make_pair(v3, v1)), long(29700));
    QCOMPARE(g.index_of(std::make_pair(v4, v2)), long(29701));

    QCOMPARE(g.index_of(std::make_pair(vA, vA)), long(-1));
    QCOMPARE(g.index_of(std::make_pair(vB, vB)), long(-1));
    QCOMPARE(g.index_of(std::make_pair(v1, v8)), long(-1));
    QCOMPARE(g.index_of(std::make_pair(v1, v4)), long(-1));
    QCOMPARE(g.index_of(std::make_pair(v4, v1)), long(-1));
    QCOMPARE(g.index_of(std::make_pair(v1, v5)), long(-1));

    QCOMPARE(g.index_of(std::make_pair(v6, vA)), long(39621));
    QCOMPARE(g.index_of(std::make_pair(vA, v6)), long(40021));

    QCOMPARE(g.index_of(std::make_pair(v1, vA)), long(-1));
    QCOMPARE(g.index_of(std::make_pair(v7, vA)), long(-1));
    QCOMPARE(g.index_of(std::make_pair(vA, vB)), long(-1));
    QCOMPARE(g.index_of(std::make_pair(vB, vA)), long(-1));

    QCOMPARE(g.index_of(std::make_pair(v7, vB)), long(40421));
    QCOMPARE(g.index_of(std::make_pair(vB, v7)), long(40821));

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
    QImage bLabelImage(bLabelRect.size(), QImage::Format_ARGB32);;
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

QTEST_MAIN(KisLazyBrushTest)
