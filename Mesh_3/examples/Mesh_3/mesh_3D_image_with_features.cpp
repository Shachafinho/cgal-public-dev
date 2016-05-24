#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>

#include <CGAL/Mesh_triangulation_3.h>
#include <CGAL/Mesh_complex_3_in_triangulation_3.h>
#include <CGAL/Mesh_criteria_3.h>

#include <CGAL/Labeled_image_mesh_domain_3.h>
#include <CGAL/Mesh_domain_with_polyline_features_3.h>
#include <CGAL/make_mesh_3.h>
#include <CGAL/Image_3.h>

#include <CGAL/Mesh_3/polylines_to_protect.h> // undocumented header

#include <vector>
#include <iostream>

#include "read_polylines.h"

// Domain
typedef CGAL::Exact_predicates_inexact_constructions_kernel K;
typedef CGAL::Labeled_image_mesh_domain_3<CGAL::Image_3,K> Image_domain;
typedef CGAL::Mesh_domain_with_polyline_features_3<Image_domain> Mesh_domain;
#ifdef CGAL_CONCURRENT_MESH_3
typedef CGAL::Parallel_tag Concurrency_tag;
#else
typedef CGAL::Sequential_tag Concurrency_tag;
#endif

// Triangulation
typedef CGAL::Mesh_triangulation_3<Mesh_domain,CGAL::Default,Concurrency_tag>::type Tr;

typedef CGAL::Mesh_complex_3_in_triangulation_3<Tr> C3t3;

// Criteria
typedef CGAL::Mesh_criteria_3<Tr> Mesh_criteria;

typedef K::Point_3 Point_3;
typedef Mesh_domain::Image_word_type Word_type; // that is `unsigned char`

// To avoid verbose function and named parameters call
using namespace CGAL::parameters;

// Protect the intersection of the object with the box of the image,
// by declaring 1D-features. Note that `CGAL::polylines_to_protect` is
// not documented.
template <typename Polyline_iterator>
void add_1D_features(const CGAL::Image_3& image,
                     Mesh_domain& domain,
                     Polyline_iterator begin,
                     Polyline_iterator end)
{
    std::vector<std::vector<Point_3> > polylines_on_bbox;
    CGAL::polylines_to_protect<Point_3, Word_type>(image, polylines_on_bbox,
                                                   begin, end);
    domain.add_features(polylines_on_bbox.begin(), polylines_on_bbox.end());

    // It is very important that the polylines passed in the range
    // [begin, end[ contain only polylines in the inside of the box of the
    // image.
    domain.add_features(begin, end);
}

int main(int argc, char* argv[])
{
  const char* fname = (argc>1)?argv[1]:"data/420.inr";
  // Loads image
  CGAL::Image_3 image;
  if(!image.read(fname)){
    std::cerr << "Error: Cannot read file " <<  fname << std::endl;
    return EXIT_FAILURE;
  }

  const char* lines_fname = (argc>2)?argv[2]:"data/420.polylines.txt";
  std::vector<std::vector<Point_3> > features_inside;
  if(!read_polylines(lines_fname, features_inside)) // see file "read_polylines.h"
  {
    std::cerr << "Error: Cannot read file " <<  lines_fname << std::endl;
    return EXIT_FAILURE;
  }

  // Domain
  Mesh_domain domain(image);

  // Declare 1D-features, see above
  add_1D_features(image, domain,
                  features_inside.begin(), features_inside.end());

    // Mesh criteria
  Mesh_criteria criteria(edge_size=6, // `edge_size` is needed with 1D-features
                         facet_angle=30, facet_size=6, facet_distance=4,
                         cell_radius_edge_ratio=3, cell_size=8);

  // Meshing
  C3t3 c3t3 = CGAL::make_mesh_3<C3t3>(domain, criteria);

  // Output
  std::ofstream medit_file("out.mesh");
  c3t3.output_to_medit(medit_file);

  return 0;
}
