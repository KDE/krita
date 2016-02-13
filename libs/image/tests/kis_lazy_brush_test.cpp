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

#include <boost/config.hpp>
#include <iostream>
#include <string>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/boykov_kolmogorov_max_flow.hpp>
#include <boost/graph/read_dimacs.hpp>
#include <boost/graph/graph_utility.hpp>

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

template <class Iterator, class KeyType>
class my_iterator_map
{
public:
    typedef KeyType key_type;
    typedef typename std::iterator_traits<Iterator>::value_type value_type;
    typedef typename std::iterator_traits<Iterator>::reference reference;
    typedef boost::lvalue_property_map_tag category;

    my_iterator_map(Iterator i = Iterator())
        : m_iter(i) { }
    Iterator m_iter;
};

template <class Iter, class KeyType>
typename std::iterator_traits<Iter>::value_type
get(const my_iterator_map<Iter,KeyType>& i, KeyType key)
{
    return key[0] + key[1];
}

template <class Iter, class KeyType>
void
put(const my_iterator_map<Iter, KeyType>& i,
     KeyType key,
    const typename std::iterator_traits<Iter>::value_type& value)
{
    i.m_iter[0] = value;
}

template <class Iter, class KeyType>
typename std::iterator_traits<Iter>::reference
at(const my_iterator_map<Iter,KeyType>& i,
    KeyType key)
{
    return i.m_iter[0];
}

template <class Iter, class ID>
my_iterator_map<Iter, ID>
make_my_iterator_map(Iter iter, ID id) {
    return my_iterator_map<Iter, ID>(iter);
}

int doSomethingElse()
{

    const unsigned int D = 2;
    typedef boost::grid_graph<D> Graph;
    typedef boost::graph_traits<Graph>::vertex_descriptor VertexDescriptor;
    typedef boost::graph_traits<Graph>::edge_descriptor EdgeDescriptor;//ADDED
    typedef boost::graph_traits<Graph>::vertices_size_type VertexIndex;
    typedef boost::graph_traits<Graph>::edges_size_type EdgeIndex;


    boost::array<std::size_t, D> lengths = { { 3, 3 } };
    Graph graph(lengths, false);

    float pixel_intensity[]={10.0f,15.0f,25.0f,
                            5.0f,220.0f,240.0f,
                            12.0f,15.0,230.0f};
    std::vector<int> groups(num_vertices(graph));
    std::vector<float> residual_capacity(num_edges(graph)); //this needs to be initialized to 0
    std::vector<float> capacity(num_edges(graph)); //this is initialized below, I believe the capacities of an edge and its reverse should be equal, but I'm not sure
    std::vector<EdgeDescriptor> reverse_edges(num_edges(graph));//ADDED

    BGL_FORALL_EDGES(e,graph,Graph)
    {
        VertexDescriptor src = source(e,graph);
        VertexDescriptor tgt = target(e,graph);
        VertexIndex source_idx = get(boost::vertex_index,graph,src);
        VertexIndex target_idx = get(boost::vertex_index,graph,tgt);
        EdgeIndex edge_idx = get(boost::edge_index,graph,e);

        capacity[edge_idx] = 255.0f - fabs(pixel_intensity[source_idx]-pixel_intensity[target_idx]); //you should change this to your "gradiant or intensity or something"

        reverse_edges[edge_idx]=edge(tgt,src,graph).first;//ADDED
    }

    BGL_FORALL_VERTICES(v,graph,Graph)
    {
        qDebug() << ppVar(v[0]) << ppVar(v[1]);

/*      VertexDescriptor src = source(e,graph);
        VertexDescriptor tgt = target(e,graph);
        VertexIndex source_idx = get(boost::vertex_index,graph,src);
        VertexIndex target_idx = get(boost::vertex_index,graph,tgt);
        EdgeIndex edge_idx = get(boost::edge_index,graph,e);

        capacity[edge_idx] = 255.0f - fabs(pixel_intensity[source_idx]-pixel_intensity[target_idx]); //you should change this to your "gradiant or intensity or something"

        reverse_edges[edge_idx]=edge(tgt,src,graph).first;//ADDED
*/
    }

    VertexDescriptor s=vertex(0,graph), t=vertex(8,graph); 

    //auto mymap = make_my_iterator_map(&capacity[0], get(boost::edge_index, graph));
    auto mymap = make_my_iterator_map(&capacity[0], EdgeDescriptor());


//in the boykov_kolmogorov_max_flow header it says that you should use this overload with an explicit color property map parameter if you are interested in finding the minimum cut
    boykov_kolmogorov_max_flow(graph,
        mymap,
        make_iterator_property_map(&residual_capacity[0], get(boost::edge_index, graph)),
        make_iterator_property_map(&reverse_edges[0], get(boost::edge_index, graph)), //CHANGED
        make_iterator_property_map(&groups[0], get(boost::vertex_index, graph)),
        get(boost::vertex_index, graph),
        s,
        t
    );


   for(size_t index=0; index < groups.size(); ++index)
   {
        if((index%lengths[0]==0)&&index)
            std::cout << std::endl;
        std::cout << groups[index] << " ";
   }

   return 0;
}

void KisLazyBrushTest::test()
{
    doSomethingElse();
}

QTEST_MAIN(KisLazyBrushTest)
