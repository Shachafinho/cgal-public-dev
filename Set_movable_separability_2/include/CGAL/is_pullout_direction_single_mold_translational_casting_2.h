// Copyright (c) 2016 Tel-Aviv University (Israel).
// All rights reserved.
//
// This file is part of CGAL (www.cgal.org).
// You can redistribute it and/or modify it under the terms of the GNU
// General Public License as published by the Free Software Foundation,
// either version 3 of the License, or (at your option) any later version.
//
// Licensees holding a valid commercial license may use this file in
// accordance with the commercial license agreement provided with the software.
//
// This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
// WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
//
// Author(s): Shahar    <shasha94@gmail.com>
//            Efi Fogel <efif@gmail.com>

#ifndef CGAL_IS_PULLOUT_DIRECTION_SINGLE_MOLD_TRANSLATIONAL_CASTING_2_H
#define CGAL_IS_PULLOUT_DIRECTION_SINGLE_MOLD_TRANSLATIONAL_CASTING_2_H
#include <CGAL/Polygon_2.h>
#include <CGAL/enum.h>
#include <limits>
#include "Set_movable_separability_2/Utils.h"
#include "Set_movable_separability_2/Circle_arrangment.h"

namespace CGAL {

  namespace Set_movable_separability_2 {

    /*! Given a simple polygon,an edge of the polygon and a pullout direction (not rotated)
     * this function determines whether a cavity (of a mold in the plane)
     * that has the shape of the polygon can be used so that the polygon could be
     * casted in the mold with the input edge and being the top edge and then pulled
     * out in the input direction (without rotation) of the mold without colliding
     * into the mold (but possibly sliding along the mold surface).
     *
     * The type that substitutes the template parameter `%CastingTraits_2` must be
     * a model of the concept `CastingTraits_2`.
     *
     * \param[in] pgn the input polygon.
     * \param[in] i the index of an edge in pgn.
     * \param[in] d the pullout direction
     * \return if the polygon can be pullout through edge i with direction d
     *
     * \pre `png` must be non-degenerate (has at least 3 vertices), simple, and
     * does not have three consecutive collinear vertices.
     */
    template <typename CastingTraits_2>
    bool is_pullout_direction_single_mold_translational_casting_2
    (const CGAL::Polygon_2<CastingTraits_2>& pgn,const typename CGAL::Polygon_2<CastingTraits_2>::Edge_const_iterator& i,
     typename CastingTraits_2::Direction_2& d, CastingTraits_2& traits)
    {
      //NOT CHECKED AT ALL
      CGAL_precondition(pgn.is_simple());
      CGAL_precondition(!internal::is_any_edge_colinear(pgn));

      auto e_it = pgn.edges_begin();
      CGAL::Orientation poly_orientation = pgn.orientation();
      auto cc_in_between = traits.counterclockwise_in_between_2_object();

      for (; e_it != pgn.edges_end(); ++e_it) {
	  auto segment_outer_circle =
	      internal::get_segment_outer_circle<CastingTraits_2>(*e_it, poly_orientation);
	  bool isordered = !cc_in_between(d,segment_outer_circle.second,
					  segment_outer_circle.first);
	  if (isordered == (e_it==i))
	    {
	      return false;
	    }
      }

      return true;
    }

    /*!
     */
    template <typename CastingTraits_2>
    bool is_pullout_direction_single_mold_translational_casting_2
    (const CGAL::Polygon_2<CastingTraits_2>& pgn, const typename CGAL::Polygon_2<CastingTraits_2>::Edge_const_iterator& i,
     typename CastingTraits_2::Direction_2& d)
    {
      CastingTraits_2 traits;
      return is_pullout_direction_single_mold_translational_casting_2(pgn, i, d,
								      traits);
    }

    /*! Given a simple polygon, and a pullout direction (not rotated)
     * this function determines whether a cavity (of a mold in the plane)
     * that has the shape of the polygon can be used so that the polygon could be
     * casted in the mold with the input edge and being the top edge and then pulled
     * out in the input direction (without rotation) of the mold without colliding
     * into the mold (but possibly sliding along the mold surface).
     *
     * The type that substitutes the template parameter `%CastingTraits_2` must be
     * a model of the concept `CastingTraits_2`.
     *
     * \param[in] pgn the input polygon.
     * \param[in] d the pullout direction
     * \return if the polygon can be pullout through some edge with direction d - top the edge, else pgn.edges_end()
     *
     * \pre `png` must be non-degenerate (has at least 3 vertices),simple, and
     * does not have three consecutive collinear vertices.
     */

    template <typename CastingTraits_2>
    typename CGAL::Polygon_2<CastingTraits_2>::Edge_const_iterator
    is_pullout_direction_single_mold_translational_casting_2
    (const CGAL::Polygon_2<CastingTraits_2>& pgn,
     typename CastingTraits_2::Direction_2& d, CastingTraits_2& traits)
     {
      //NOT CHECKED AT ALL
typedef typename CGAL::Polygon_2<CastingTraits_2>::Edge_const_iterator Edge_iter;
      CGAL_precondition(pgn.is_simple());
      CGAL_precondition(!internal::is_any_edge_colinear(pgn));

      Edge_iter e_it = pgn.edges_begin();
      CGAL::Orientation poly_orientation = pgn.orientation();
      auto segment_outer_circle =
	  internal::get_segment_outer_circle<CastingTraits_2>(*e_it++, poly_orientation);
      auto cc_in_between = traits.counterclockwise_in_between_2_object();
      Edge_iter top_edge= pgn.edges_end();
      for (; e_it != pgn.edges_end(); ++e_it) {
	  segment_outer_circle =
	      internal::get_segment_outer_circle<CastingTraits_2>(*e_it, poly_orientation);
	  bool isordered = !cc_in_between(d,
					  segment_outer_circle.second,
					  segment_outer_circle.first);
	  if (!isordered) //unlikely - this if must be true atleast once for any polygon - add ref to paper
	    {
	      if(top_edge== pgn.edges_end())
		{
		  top_edge=e_it;
		}
	      else
		return pgn.edges_end();
	    }
      }
      CGAL_postcondition(top_edge!=pgn.edges_end());
      return top_edge;
     }

    /*!
     */
    template <typename CastingTraits_2>
    typename CGAL::Polygon_2<CastingTraits_2>::Edge_const_iterator
    is_pullout_direction_single_mold_translational_casting_2
    (const CGAL::Polygon_2<CastingTraits_2>& pgn,
     typename CastingTraits_2::Direction_2& d)
     {
      CastingTraits_2 traits;
      return is_pullout_direction_single_mold_translational_casting_2(pgn, d, traits);
     }

  } /* end namesapce Set_movable_separability_2 */
} /* end namesapce CGAL */

#endif
