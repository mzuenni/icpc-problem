//=======================================================================
// Copyright 2007 Aaron Windsor
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
#ifndef __IS_STRAIGHT_LINE_DRAWING_HPP__
#define __IS_STRAIGHT_LINE_DRAWING_HPP__

#include <boost/config.hpp>
#include <boost/utility.hpp> //for next and prior
#include <boost/tuple/tuple.hpp>
#include <boost/tuple/tuple_comparison.hpp>
#include <boost/property_map/property_map.hpp>
#include <boost/graph/properties.hpp>
#include <boost/graph/planar_detail/bucket_sort.hpp>

#include <algorithm>
#include <vector>
#include <set>



namespace boost
{

  // Return true exactly when the line segments s1 = ((x1,y1), (x2,y2)) and
  // s2 = ((a1,b1), (a2,b2)) intersect in a point other than the endpoints of
  // the line segments. The one exception to this rule is when s1 = s2, in
  // which case false is returned - this is to accomodate multiple edges
  // between the same pair of vertices, which shouldn't invalidate the straight
  // line embedding. A tolerance variable epsilon can also be used, which
  // defines how far away from the endpoints of s1 and s2 we want to consider
  // an intersection.

  bool intersects(long double x1, long double y1,
                  long double x2, long double y2,
                  long double a1, long double b1,
                  long double a2, long double b2,
                  long double epsilon = 1e-9
                  )
  {

    if (x1 - x2 == 0)
      {
        std::swap(x1,a1);
        std::swap(y1,b1);
        std::swap(x2,a2);
        std::swap(y2,b2);
      }

    if (x1 - x2 == 0)
      {
        BOOST_USING_STD_MAX();
        BOOST_USING_STD_MIN();

        //two vertical line segments
        long double min_y = min BOOST_PREVENT_MACRO_SUBSTITUTION(y1,y2);
        long double max_y = max BOOST_PREVENT_MACRO_SUBSTITUTION(y1,y2);
        long double min_b = min BOOST_PREVENT_MACRO_SUBSTITUTION(b1,b2);
        long double max_b = max BOOST_PREVENT_MACRO_SUBSTITUTION(b1,b2);
        if ((max_y > max_b && max_b > min_y) ||
            (max_b > max_y && max_y > min_b)
            )
          return true;
        else
          return false;
      }

    long double x_diff = x1 - x2;
    long double y_diff = y1 - y2;
    long double a_diff = a2 - a1;
    long double b_diff = b2 - b1;

    long double beta_denominator = b_diff - (y_diff/((long double)x_diff)) * a_diff;

    if (beta_denominator == 0)
      {
        //parallel lines
        return false;
      }

    long double beta = (b2 - y2 - (y_diff/((long double)x_diff)) * (a2 - x2)) / 
      beta_denominator;
    long double alpha = (a2 - x2 - beta*(a_diff))/x_diff;

    long double upper_bound = 1 - epsilon;
    long double lower_bound = 0 + epsilon;

    return (beta < upper_bound && beta > lower_bound && 
            alpha < upper_bound && alpha > lower_bound);

  }


  template <typename Graph, 
            typename GridPositionMap, 
            typename VertexIndexMap
            >
  bool is_straight_line_drawing(const Graph& g, 
                                GridPositionMap drawing, 
                                VertexIndexMap vm
                                )
  {

    typedef typename graph_traits<Graph>::vertex_descriptor vertex_t;
    typedef typename graph_traits<Graph>::vertex_iterator vertex_iterator_t;
    typedef typename graph_traits<Graph>::edge_descriptor edge_t;
    typedef typename graph_traits<Graph>::edge_iterator edge_iterator_t;
    typedef typename graph_traits<Graph>::edges_size_type e_size_t;
    typedef typename graph_traits<Graph>::vertices_size_type v_size_t;

    typedef std::size_t x_coord_t;
    typedef std::size_t y_coord_t;
    typedef boost::tuple<edge_t, x_coord_t, y_coord_t> edge_event_t;
    typedef typename std::vector< edge_event_t > edge_event_queue_t;

    typedef tuple<y_coord_t, y_coord_t, x_coord_t, x_coord_t> active_map_key_t;
    typedef edge_t active_map_value_t;
    typedef std::map< active_map_key_t, active_map_value_t > active_map_t;
    typedef typename active_map_t::iterator active_map_iterator_t;


    edge_event_queue_t edge_event_queue;
    active_map_t active_edges;

    edge_iterator_t ei, ei_end;
    for(tie(ei,ei_end) = edges(g); ei != ei_end; ++ei)
      {
        edge_t e(*ei);
        vertex_t s(source(e,g));
        vertex_t t(target(e,g));
        edge_event_queue.push_back
          (make_tuple(e, 
                      static_cast<std::size_t>(drawing[s].x),
                      static_cast<std::size_t>(drawing[s].y)
                      )
           );
        edge_event_queue.push_back
          (make_tuple(e,
                      static_cast<std::size_t>(drawing[t].x),
                      static_cast<std::size_t>(drawing[t].y)
                      )
           );
      }

    // Order by edge_event_queue by first, then second coordinate 
    // (bucket_sort is a stable sort.)
    bucket_sort(edge_event_queue.begin(), edge_event_queue.end(),
                property_map_tuple_adaptor<edge_event_t, 2>()
                );
    
    bucket_sort(edge_event_queue.begin(), edge_event_queue.end(),
                property_map_tuple_adaptor<edge_event_t, 1>()
                );

    typedef typename edge_event_queue_t::iterator event_queue_iterator_t;
    event_queue_iterator_t itr_end = edge_event_queue.end();
    for(event_queue_iterator_t itr = edge_event_queue.begin(); 
        itr != itr_end; ++itr
        )
      {
        edge_t e(get<0>(*itr));
        vertex_t source_v(source(e,g));
        vertex_t target_v(target(e,g));
        if (drawing[source_v].y > drawing[target_v].y)
          std::swap(source_v, target_v);

        active_map_key_t key(get(drawing, source_v).y,
                             get(drawing, target_v).y,
                             get(drawing, source_v).x,
                             get(drawing, target_v).x
                             );

        active_map_iterator_t a_itr = active_edges.find(key);
        if (a_itr == active_edges.end())
          {
            active_edges[key] = e;
          }
        else
          {
            active_map_iterator_t before, after;
            if (a_itr == active_edges.begin())
              before = active_edges.end();
            else
              before = prior(a_itr);
            after = boost::next(a_itr);

            if (before != active_edges.end())
              {
                
                edge_t f = before->second;
                vertex_t e_source(source(e,g));
                vertex_t e_target(target(e,g));
                vertex_t f_source(source(f,g));
                vertex_t f_target(target(f,g));

                if (intersects(drawing[e_source].x, 
                               drawing[e_source].y,
                               drawing[e_target].x,
                               drawing[e_target].y,
                               drawing[f_source].x, 
                               drawing[f_source].y,
                               drawing[f_target].x,
                               drawing[f_target].y
                               )
                    )
                  return false;
              }

            if (after != active_edges.end())
              {
                
                edge_t f = after->second;
                vertex_t e_source(source(e,g));
                vertex_t e_target(target(e,g));
                vertex_t f_source(source(f,g));
                vertex_t f_target(target(f,g));

                if (intersects(drawing[e_source].x, 
                               drawing[e_source].y,
                               drawing[e_target].x,
                               drawing[e_target].y,
                               drawing[f_source].x, 
                               drawing[f_source].y,
                               drawing[f_target].x,
                               drawing[f_target].y
                               )
                    )
                  return false;
              }

            active_edges.erase(a_itr);

          }
      }

    return true;
    
  }


  template <typename Graph, typename GridPositionMap>
  bool is_straight_line_drawing(const Graph& g, GridPositionMap drawing)
  {
    return is_straight_line_drawing(g, drawing, get(vertex_index,g));
  }

}

#endif // __IS_STRAIGHT_LINE_DRAWING_HPP__