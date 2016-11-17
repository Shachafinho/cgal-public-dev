#include <list>
#include <fstream>

#include <CGAL/Exact_predicates_exact_constructions_kernel.h>
#include <CGAL/Polygon_2.h>
#include <CGAL/single_mold_translational_casting_2.h>

typedef CGAL::Exact_predicates_exact_constructions_kernel Kernel;
typedef CGAL::Polygon_2<Kernel>                           Polygon_2;
typedef Kernel::Direction_2                               Direction_2;
typedef std::pair<Direction_2, Direction_2>               Direction_range;
typedef std::pair<size_t, Direction_range>                Top_edge;

namespace SMS = CGAL::Set_movable_separability_2;

// The main program:
int main(int  argc, char* argv[])
{
  Polygon_2 pgn;

  const char* filename = (argc > 1) ? argv[1] : "polygon.dat";
  std::ifstream input_file(filename);
  if (! input_file.is_open()) {
    std::cerr << "Failed to open the " << filename << std::endl;
    return -1;
  }
  input_file >> pgn;
  input_file.close();

  auto poly_orientation = pgn.orientation();
  std::list<Top_edge> top_edges;
  SMS::single_mold_translational_casting_2(pgn, std::back_inserter(top_edges));

  if (top_edges.empty())
    std::cout << "The polygon is not castable!" << std::endl;
  else {
    std::cout << "There are " << top_edges.size() << " top edges:" << std::endl;
    for (const auto& top_edge : top_edges) {
      std::cout << "Edge number: " << top_edge.first << std::endl
                << "\tEdge: "<< pgn.edge(top_edge.first) << std::endl
                << "\tPullout directions from: "<< top_edge.second.first
                << " to " << top_edge.second.second
                << std::endl<< std::endl;
    }
  }

  return 0;
}
