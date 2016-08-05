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

#include "kis_lazy_fill_tools.h"

#include <KoColor.h>

#include <numeric>
#include <boost/limits.hpp>

#include <boost/graph/graph_traits.hpp>
#include <boost/graph/properties.hpp>
#include <boost/iterator/counting_iterator.hpp>
#include <boost/iterator/transform_iterator.hpp>

#include <boost/graph/properties.hpp>
#include <boost/graph/boykov_kolmogorov_max_flow.hpp>
#include <boost/graph/iteration_macros.hpp>

#include "lazybrush/kis_lazy_fill_graph.h"
#include "lazybrush/kis_lazy_fill_capacity_map.h"

#include "kis_sequential_iterator.h"
#include <floodfill/kis_scanline_fill.h>

namespace KisLazyFillTools {


void cutOneWay(const KoColor &color,
                   KisPaintDeviceSP src,
                   KisPaintDeviceSP colorScribble,
                   KisPaintDeviceSP backgroundScribble,
                   KisPaintDeviceSP resultDevice,
                   KisPaintDeviceSP maskDevice)
{
    using namespace boost;

    KIS_ASSERT_RECOVER_RETURN(src->pixelSize() == 1);
    KIS_ASSERT_RECOVER_RETURN(colorScribble->pixelSize() == 1);
    KIS_ASSERT_RECOVER_RETURN(backgroundScribble->pixelSize() == 1);
    KIS_ASSERT_RECOVER_RETURN(maskDevice->pixelSize() == 1);
    KIS_ASSERT_RECOVER_RETURN(*resultDevice->colorSpace() == *color.colorSpace());

    KisLazyFillCapacityMap capacityMap(src, colorScribble, backgroundScribble, maskDevice);
    KisLazyFillGraph &graph = capacityMap.graph();

    std::vector<default_color_type> groups(num_vertices(graph));
    std::vector<float> residual_capacity(num_edges(graph), 0);

    std::vector<typename graph_traits<KisLazyFillGraph>::vertices_size_type> distance_vec(num_vertices(graph), 0);
    std::vector<typename graph_traits<KisLazyFillGraph>::edge_descriptor> predecessor_vec(num_vertices(graph));

    auto vertexIndexMap = get(boost::vertex_index, graph);

    typedef KisLazyFillGraph::vertex_descriptor Vertex;

    Vertex s(Vertex::LABEL_A);
    Vertex t(Vertex::LABEL_B);

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
    Q_UNUSED(maxFlow);

    KisSequentialIterator dstIt(resultDevice, graph.rect());
    KisSequentialIterator mskIt(maskDevice, graph.rect());

    const int pixelSize = resultDevice->pixelSize();

    do {
        KisLazyFillGraph::vertex_descriptor v(dstIt.x(), dstIt.y());
        long vertex_idx = get(boost::vertex_index, graph, v);
        default_color_type label = groups[vertex_idx];

        if (label == black_color) {
            memcpy(dstIt.rawData(), color.data(), pixelSize);
            *mskIt.rawData() = 10 + (int(label) << 4);
        }
    } while (dstIt.nextPixel() && mskIt.nextPixel());
}

QVector<QPoint> splitIntoConnectedComponents(KisPaintDeviceSP dev)
{
    QVector<QPoint> points;
    const KoColorSpace *cs = dev->colorSpace();

    const QRect rect = dev->exactBounds();
    if (rect.isEmpty()) return points;

    /**
     * Please note that since we modify the device inside
     * clearNonZeroComponent() call, we must use a *writable*
     * iterator, for not ending up with a lazy copied old version of a
     * device.
     */
    KisSequentialIterator dstIt(dev, rect);

    do {
        if (cs->opacityU8(dstIt.rawData()) > 0) {
            const QPoint pt(dstIt.x(), dstIt.y());
            points << pt;

            KisScanlineFill fill(dev, pt, rect);
            fill.clearNonZeroComponent();
        }
    } while (dstIt.nextPixel());

    return points;
}









}
