#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Periodic_2_triangulation_traits_2.h>
#include <CGAL/algorithm.h>
#include <CGAL/Periodic_2_Delaunay_triangulation_2.h>
#include <CGAL/Alpha_shape_2.h>

#include <iostream>
#include <fstream>
#include <vector>
#include <list>


typedef CGAL::Exact_predicates_inexact_constructions_kernel K;
typedef CGAL::Periodic_2_triangulation_traits_2<K> GT;

typedef GT::FT FT;
typedef GT::Point_2  Point;
typedef GT::Segment_2  Segment;

typedef CGAL::Periodic_2_triangulation_vertex_base_2<GT> P2Vb;
typedef CGAL::Periodic_2_triangulation_face_base_2<GT> P2Fb;
typedef CGAL::Alpha_shape_vertex_base_2<GT, P2Vb> Vb;
typedef CGAL::Alpha_shape_face_base_2<GT, P2Fb>  Fb;
typedef CGAL::Triangulation_data_structure_2<Vb,Fb> Tds;
typedef CGAL::Periodic_2_Delaunay_triangulation_2<GT,Tds> Triangulation_2;

typedef CGAL::Alpha_shape_2<Triangulation_2>  Alpha_shape_2;
typedef Alpha_shape_2::Alpha_shape_edges_iterator Alpha_shape_edges_iterator;

template <class OutputIterator>
void
alpha_edges( const Alpha_shape_2&  A,
	     OutputIterator out)
{
  for(Alpha_shape_edges_iterator it =  A.alpha_shape_edges_begin();
      it != A.alpha_shape_edges_end();
      ++it){
    *out++ = A.segment(*it);
  }
}

template <class OutputIterator>
bool
file_input(OutputIterator out)
{
  std::ifstream is("./data/fin", std::ios::in);

  if(is.fail()){
    std::cerr << "unable to open file for input" << std::endl;
    return false;
  }

  int n;
  is >> n;
  std::cout << "Reading " << n << " points from file" << std::endl;
  CGAL::cpp11::copy_n(std::istream_iterator<Point>(is), n, out);

  return true;
}


// Reads a list of points and returns a list of segments
// corresponding to the Alpha shape.
int main()
{
  std::list<Point> points;
  if(! file_input(std::back_inserter(points))){
    return -1;
  }

  Alpha_shape_2 A(points.begin(), points.end(),
         	  FT(0.001),
         	  Alpha_shape_2::GENERAL);

  std::vector<Segment> segments;
  alpha_edges( A, std::back_inserter(segments));

  std::cout << "Alpha Shape computed" << std::endl;
  std::cout << segments.size() << " alpha shape edges" << std::endl;
  std::cout << "Optimal alpha: " << *A.find_optimal_alpha(1)<<std::endl;

  return 0;
}
