#include <string>
#include <list>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <utility>
#include <cctype>

#include <CGAL/Exact_predicates_exact_constructions_kernel.h>
#include <CGAL/Polygon_2.h>
#include <CGAL/single_mold_translational_casting_2.h>

typedef CGAL::Exact_predicates_exact_constructions_kernel Kernel;
typedef CGAL::Polygon_2<Kernel>                           Polygon_2;
typedef Kernel::Direction_2                               Direction_2;
typedef Kernel::Point_2                                   Point_2;

typedef std::pair<Direction_2, Direction_2>               Direction_range;
typedef std::pair<size_t, Direction_range>                Top_edge;

namespace SMS = CGAL::Set_movable_separability_2;

struct Top_edge_comparer {
  bool operator()(const Top_edge& a,  const Top_edge& b)
  {
    auto facet_a = a.first;
    const auto& d1_a = a.second.first;
    const auto& d2_a = a.second.second;
    auto facet_b = b.first;
    const auto& d1_b = b.second.first;
    const auto& d2_b = b.second.second;

    if (a.first < b.first) return true;
    if (a.first > b.first) return false;
    if (a.second.first < b.second.first) return true;
    if (a.second.first > b.second.first) return false;
    return a.second.second < b.second.second;
  }
};

bool test_one_file(std::ifstream& inp)
{
  Polygon_2 pgn;
  inp >> pgn;
  // std::cout << pgn << std::endl;

  std::vector<Top_edge> top_edges;
  SMS::single_mold_translational_casting_2(pgn, std::back_inserter(top_edges));

  size_t exp_num_top_edges;
  inp >> exp_num_top_edges;
  // std::cout << "Exp. no. of top facets: " << exp_num_top_edges << std::endl;
  std::vector<Top_edge> exp_top_edges(exp_num_top_edges);
  for (auto& top_edge : exp_top_edges) {
    size_t facet;
    Direction_2 d1, d2;
    inp >> facet >> d1 >> d2;
    // std::cout << facet << " " << d1 << " " << d2 << std::endl;
    top_edge = std::make_pair(facet, std::make_pair(d1, d2));
  }

  std::sort(top_edges.begin(), top_edges.end(), Top_edge_comparer());
  std::sort(exp_top_edges.begin(), exp_top_edges.end(), Top_edge_comparer());

  if (top_edges.size() != exp_top_edges.size()) {
    std::cerr << "Number of facets: "
              << "obtain: " << top_edges.size()
              << ", expected: " << exp_top_edges.size()
              << std::endl;
    return false;
  }
  auto exp_it = exp_top_edges.begin();
  size_t i(0);
  for (auto it = top_edges.begin(); it != top_edges.end(); ++it, ++exp_it) {
    auto facet = it->first;
    const auto& d1 = it->second.first;
    const auto& d2 = it->second.second;
    auto exp_facet = exp_it->first;
    const auto& exp_d1 = exp_it->second.first;
    const auto& exp_d2 = exp_it->second.second;
    if ((facet != exp_facet) || (d1 != exp_d1) || (d2 != exp_d2)) {
      std::cerr << "Top edge[" << i++ << "]: "
                << "obtained: " << facet << " " << d1 << " " << d2
                << ", expected: " << exp_facet << " " << exp_d1 << " " << exp_d2
                << std::endl;
      return false;
    }
  }
  return true;
}

int main(int argc, char* argv[])
{
  if (argc < 2) {
    std::cerr << "Missing input file" << std::endl;
    return -1;
  }

  int success = 0;
  for (size_t i = 1; i < argc; ++i) {
    std::string str(argv[i]);
    if (str.empty()) continue;

    auto itr = str.end();
    --itr;
    while (itr != str.begin()) {
      auto tmp = itr;
      --tmp;
      if (!isspace(*itr)) break;
      str.erase(itr);
      itr = tmp;
    }
    if (str.size() <= 1) continue;
    std::ifstream inp(str.c_str());
    if (!inp.is_open()) {
      std::cerr << "Failed to open " << str << std::endl;
      return -1;
    }
    if (! test_one_file(inp)) {
      std::cout << str << ": ERROR" << std::endl;
      ++success;
    }
    else std::cout << str << ": succeeded" << std::endl;
    inp.close();
  }

  return success;
}
