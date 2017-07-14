#ifndef HEXEXTR_H
#define HEXEXTR_H
#include"typedefs.h"
#include"handles.h"
#include"triangulation_to_LCC.h"
#include<unordered_map>
#include<map>
#include"func.h"
//#include"geometry_extraction.h"
#include"dart_extraction.h"
#include<vector>
#include"frame_field.h"


namespace std{
  int dart_count = 0;
  template<>
  struct hash<Face_handle>{
    std::size_t operator()(const Face_handle& fh) const{
      return (fh.enumeration)%1000;
    } 
  };

  /*template<>
  struct hash<Dart_handle>{
    std::size_t operator()(const Dart_handle& dh) const{
      return lcc.info(dh);
    } 
  };*/
}

class HexExtr{
  public:
    //HexExtr();
    HexExtr(std::string infilename): identity(1,0,0,0,1,0,0,0,1,1){
      load_off_to_LCC(infilename, input_tet_mesh); //tetmesh to lcc
      parametrize(input_tet_mesh);
      directions.push_back(Direction(1,0,0));
      directions.push_back(Direction(0,1,0));
      directions.push_back(Direction(0,0,1));
      directions.push_back(Direction(-1,0,0));
      directions.push_back(Direction(0,-1,0));
      directions.push_back(Direction(0,0,-1));
      for(int i = 0;i < 6;i++) 
        for(int j = 0;j < 6;j++) 
          for(int k = 0;k < 6;k++) 
            if(CGAL::cross_product(directions[i].vector(),directions[j].vector()) == directions[k].vector()) 
              G.push_back(Aff_transformation(directions[i].dx(), directions[j].dx(), directions[k].dx(),
directions[i].dy(), directions[j].dy(), directions[k].dy(), 
directions[i].dz(), directions[j].dz(), directions[k].dz(), 1)); //chiral cubical symmetry group
      int cell = 0;
      for(LCC_3::One_dart_per_cell_range<3>::iterator it = input_tet_mesh.one_dart_per_cell<3>().begin(), itend = input_tet_mesh.one_dart_per_cell<3>().end(); it != itend; it++){
        for(LCC_3::Dart_of_cell_range<3>::iterator it1 = input_tet_mesh.darts_of_cell<3>(it).begin(), it1end = input_tet_mesh.darts_of_cell<3>(it).end(); it1 != it1end; it1++){
          (input_tet_mesh.info(it1)).cell_no = cell;
        }
        cell++;
      }
      std::vector<std::vector<Aff_transformation>> g(cell);
      for(int j = 0; j<cell; j++){
        std::vector<Aff_transformation> temp(cell);
        g[j] = temp;
      }

      std::vector<Aff_transformation> parametrization_matrices(cell);      
      for(LCC_3::One_dart_per_cell_range<3>:: iterator it = input_tet_mesh.one_dart_per_cell<3>().begin(), itend = input_tet_mesh.one_dart_per_cell<3>().end(); it != itend; it++){
        std::vector<Point> points, parameters;
        for(LCC_3::One_dart_per_incident_cell_range<0,3>::iterator it2 = input_tet_mesh.one_dart_per_incident_cell<0,3>(it).begin(), it2end = input_tet_mesh.one_dart_per_incident_cell<0,3>(it).end(); it2 != it2end; it2++){
          points.push_back(input_tet_mesh.point(it2)); parameters.push_back((input_tet_mesh.info(it2)).parameters);
        }
        Aff_transformation at = get_parametrization_matrix(points[0], points[1], points[2], points[3], parameters[0], parameters[1], parameters[2], parameters[3]); //to find the parametrization function in every tet.
        //std::cout<<points[0]<<" "<<points[0]<<" "<<points[1]<<" "<<points[2]<<" "<<parameters[0]<<" "<<parameters[1]<<" "<<parameters[2]<<std::endl;
        //print_aff_transformation(at);
        parametrization_matrices[(input_tet_mesh.info(it)).cell_no] = at;
        Cell_handle ch(it, points, parameters, at);
        cells.push_back(ch);
        
      }


      for(LCC_3::One_dart_per_cell_range<2>::iterator it = input_tet_mesh.one_dart_per_cell<2>().begin(), 
itend = input_tet_mesh.one_dart_per_cell<2>().end(); it != itend; it++){
        // Face_handle fh(input_tet_mesh, it, i); i++;
         //dart_in_face.emplace(it, fh);        
        Aff_transformation at = extract_transition_function(it, input_tet_mesh, G);
        g[(input_tet_mesh.info(it)).cell_no][(input_tet_mesh.info(input_tet_mesh.alpha(it, 3))).cell_no] = at;
        g[(input_tet_mesh.info(input_tet_mesh.alpha(it, 3))).cell_no][(input_tet_mesh.info(it)).cell_no] = at.inverse();
         //std::cout<<i<<std::endl;
       // print_aff_transformation(at);
        // faces_with_transitions.emplace(fh, at);
         //faces.push_back(fh);
		
      }


     /* int v=0;
      for(LCC_3::Dart_range::iterator it = lcc.darts().begin(), itend = lcc.darts().end(); it != itend; it++){
        lcc.info(it) = v; v++;
      }*/
/*
      int nv = 0;
      for(LCC_3::One_dart_per_cell_range<0>::iterator it = input_tet_mesh.one_dart_per_cell<0>().begin(), itend = input_tet_mesh.one_dart_per_cell<0>().end(); it!=itend; it++){
        Vertex_handle vh(input_tet_mesh, it, nv, input_tet_mesh.is_free(it, 3));
        vertices.push_back(vh);
      }

      
      for(LCC_3::One_dart_per_cell_range<1>::iterator it = input_tet_mesh.one_dart_per_cell<1>().begin(), itend = input_tet_mesh.one_dart_per_cell<1>().end(); it != itend; it++){
        Vertex_handle from, to;
        for(std::vector<Vertex_handle>::iterator i = vertices.begin(), iend = vertices.end(); i != iend; i++){
          if(input_tet_mesh.point((*i).incident_dart) == input_tet_mesh.point(it)) from = (*i); //there must be a better way to implement this.
          if(input_tet_mesh.point((*i).incident_dart) == input_tet_mesh.point(input_tet_mesh.alpha(it, 0))) to = (*i);
        }
        Edge_handle eh(input_tet_mesh, it, from, to); 
        edges.push_back(eh); 
      }     

      optimise_frame_field(input_tet_mesh, vertices, edges, 1); 

//Sanitization 
   */
//Extract vertices
     for(LCC_3::Vertex_attribute_range::iterator v = input_tet_mesh.vertex_attributes().begin(), vend = input_tet_mesh.vertex_attributes().end(); v != vend; v++){
       Point p = input_tet_mesh.point_of_vertex_attribute(v);
       output_mesh.create_vertex_attribute(p);
     }
     //vertex_extraction(input_tet_mesh, output_mesh);
     extract_darts(input_tet_mesh, output_mesh, parametrization_matrices);
    
    }
    std::unordered_map<Face_handle, Aff_transformation> faces_with_transitions; //Take this as input and make dart_handle face_handle map using this
    std::map<Dart_handle, Face_handle> dart_in_face;
    std::vector<Face_handle> faces;
    std::vector<Edge_handle> edges;
    std::vector<Vertex_handle> vertices;
    std::vector<Cell_handle> cells;
    //std::vector<Point> hvertices;
    std::vector<Direction> directions;
    Aff_transformation identity;//(1,0,0,0,1,0,0,0,1,1);
    LCC_3 input_tet_mesh, output_mesh;//, parametrized_mesh, 
    std::vector<Aff_transformation> G; //chiral cubical symmetry group
   
};

#endif
