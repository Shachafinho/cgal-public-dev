#include "config.h"
#include "Scene_spheres_item.h"
#include "Scene_c3t3_item.h"

#include <QVector>
#include <QColor>
#include <QPixmap>
#include <QPainter>
#include <QtCore/qglobal.h>
#include <QGuiApplication>

#include <map>
#include <vector>
#include <CGAL/gl.h>
#include <CGAL/Mesh_3/dihedral_angle_3.h>
#include <CGAL/Three/Scene_interface.h>
#include <CGAL/Real_timer.h>

#include <QGLViewer/manipulatedFrame.h>
#include <QGLViewer/qglviewer.h>

#include <boost/function_output_iterator.hpp>
#include <boost/foreach.hpp>

#include <CGAL/AABB_tree.h>
#include <CGAL/AABB_traits.h>
#include <CGAL/AABB_C3T3_triangle_primitive.h>

typedef CGAL::AABB_C3T3_triangle_primitive<Kernel,C3t3> Primitive;
typedef CGAL::AABB_traits<Kernel, Primitive> Traits;
typedef CGAL::AABB_tree<Traits> Tree;
typedef Tree::Point_and_primitive_id Point_and_primitive_id;

// The special Scene_item only for triangles
class Scene_intersection_item : public CGAL::Three::Scene_item
{
  Q_OBJECT
public :
  Scene_intersection_item(Scene_c3t3_item* parent)
  :CGAL::Three::Scene_item(NumberOfBuffers,NumberOfVaos)
  {
    setParent(parent);
  }
  void init_vectors(
      std::vector<float> *p_vertices,
      std::vector<float> *p_normals,
      std::vector<float> *p_edges,
      std::vector<float> *p_colors)
  {
    vertices = p_vertices;
    normals = p_normals;
    edges = p_edges;
    colors = p_colors;
  }
  void setColor(QColor c)
  {
    qobject_cast<Scene_c3t3_item*>(this->parent())->setColor(c);
    Scene_item::setColor(c);
  }
  // Indicates if rendering mode is supported
  bool supportsRenderingMode(RenderingMode m) const {
    return (m != Gouraud && m != PointsPlusNormals && m != Splatting && m != Points);
  }
  void initialize_buffers(CGAL::Three::Viewer_interface *viewer)
  {
   //vao containing the data for the facets
    {
      program = getShaderProgram(PROGRAM_WITH_LIGHT, viewer);
      program->bind();

      vaos[Facets]->bind();
      buffers[Vertices].bind();
      buffers[Vertices].allocate(vertices->data(),
        static_cast<int>(vertices->size()*sizeof(float)));
      program->enableAttributeArray("vertex");
      program->setAttributeBuffer("vertex", GL_FLOAT, 0, 3);
      buffers[Vertices].release();

      buffers[Normals].bind();
      buffers[Normals].allocate(normals->data(),
        static_cast<int>(normals->size()*sizeof(float)));
      program->enableAttributeArray("normals");
      program->setAttributeBuffer("normals", GL_FLOAT, 0, 3);
      buffers[Normals].release();

      buffers[Colors].bind();
      buffers[Colors].allocate(colors->data(),
        static_cast<int>(colors->size()*sizeof(float)));
      program->enableAttributeArray("colors");
      program->setAttributeBuffer("colors", GL_FLOAT, 0, 3);
      buffers[Colors].release();

      vaos[Facets]->release();
      program->release();

    }
      //vao containing the data for the lines
      {
          program = getShaderProgram(PROGRAM_NO_SELECTION, viewer);
          program->bind();

          vaos[Lines]->bind();
          buffers[Edges].bind();
          buffers[Edges].allocate(edges->data(),
                                           static_cast<int>(edges->size()*sizeof(float)));
          program->enableAttributeArray("vertex");
          program->setAttributeBuffer("vertex", GL_FLOAT, 0, 3);
          buffers[Edges].release();

          vaos[Lines]->release();
          program->release();
      }
  }
  //Displays the item
  void draw(CGAL::Three::Viewer_interface* viewer) const
  {
    vaos[Facets]->bind();
    program = getShaderProgram(PROGRAM_WITH_LIGHT);
    attribBuffers(viewer, PROGRAM_WITH_LIGHT);
    program->bind();

    // positions_poly is also used for the faces in the cut plane
    // and changes when the cut plane is moved
    viewer->glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(vertices->size() / 3));
    program->release();
    vaos[Facets]->release();
  }
  void drawEdges(CGAL::Three::Viewer_interface* viewer) const
  {
    vaos[Lines]->bind();
    program = getShaderProgram(PROGRAM_NO_SELECTION);
    attribBuffers(viewer, PROGRAM_NO_SELECTION);
    program->bind();
    program->setAttributeValue("colors", QColor(Qt::black));
    viewer->glDrawArrays(GL_LINES, 0, static_cast<GLsizei>(edges->size() / 3));
    program->release();
    vaos[Lines]->release();
  }
  void addTriangle(Kernel::Point_3 pa, Kernel::Point_3 pb, Kernel::Point_3 pc, CGAL::Color color)
  {
    Kernel::Vector_3 n = cross_product(pb - pa, pc - pa);
    n = n / CGAL::sqrt(n*n);


    for (int i = 0; i<3; i++)
    {
      normals->push_back(n.x());
      normals->push_back(n.y());
      normals->push_back(n.z());
    }
    vertices->push_back(pa.x());
    vertices->push_back(pa.y());
    vertices->push_back(pa.z());

    vertices->push_back(pb.x());
    vertices->push_back(pb.y());
    vertices->push_back(pb.z());

    vertices->push_back(pc.x());
    vertices->push_back(pc.y());
    vertices->push_back(pc.z());

    edges->push_back(pa.x());
    edges->push_back(pa.y());
    edges->push_back(pa.z());

    edges->push_back(pb.x());
    edges->push_back(pb.y());
    edges->push_back(pb.z());

    edges->push_back(pb.x());
    edges->push_back(pb.y());
    edges->push_back(pb.z());

    edges->push_back(pc.x());
    edges->push_back(pc.y());
    edges->push_back(pc.z());

    edges->push_back(pc.x());
    edges->push_back(pc.y());
    edges->push_back(pc.z());

    edges->push_back(pa.x());
    edges->push_back(pa.y());
    edges->push_back(pa.z());

    for(int i=0; i<3; i++)
    {
      colors->push_back((float)color.red()/255);
      colors->push_back((float)color.green()/255);
      colors->push_back((float)color.blue()/255);
    }
  }

  Scene_item* clone() const {return 0;}
  QString toolTip() const {return QString();}
private:
  enum Buffer
  {
      Vertices =0,
      Normals,
      Colors,
      Edges,
      NumberOfBuffers
  };
  enum Vao
  {
      Facets=0,
      Lines,
      NumberOfVaos
  };
  //contains the data
  mutable std::vector<float> *vertices;
  mutable std::vector<float> *normals;
  mutable std::vector<float> *edges;
  mutable std::vector<float> *colors;
  mutable QOpenGLShaderProgram *program;
}; //end of class Scene_triangle_item


struct Scene_c3t3_item_priv {
  Scene_c3t3_item_priv(Scene_c3t3_item* item)
    : item(item), c3t3()
  {
    init_default_values();
  }
  Scene_c3t3_item_priv(const C3t3& c3t3_, Scene_c3t3_item* item)
    : item(item), c3t3(c3t3_)
  {
    init_default_values();
  }

  void init_default_values() {
    show_tetrahedra = false;
    is_aabb_tree_built = false;
  }

  void computeIntersection(const Primitive& facet);

  void fill_aabb_tree() {
    if(item->isEmpty()) return;
    QGuiApplication::setOverrideCursor(Qt::WaitCursor);
    CGAL::Real_timer timer;
    timer.start();
    tree.clear();
    for (Tr::Finite_facets_iterator
           fit = c3t3.triangulation().finite_facets_begin(),
           end = c3t3.triangulation().finite_facets_end();
         fit != end; ++fit)
    {
      Tr::Cell_handle ch = fit->first, nh =ch->neighbor(fit->second);

      if( (!c3t3.is_in_complex(ch)) &&  (!c3t3.is_in_complex(nh)) )
        continue;

      if(c3t3.is_in_complex(ch)){
        tree.insert(Primitive(fit));
      } else{
        int ni = nh->index(ch);
        tree.insert(Primitive(Tr::Facet(nh,ni)));
      }
    }
    tree.build();
    std::cerr << "C3t3 facets AABB tree built in " << timer.time()
              << " wall-clock seconds\n";

    is_aabb_tree_built = true;
    QGuiApplication::restoreOverrideCursor();
  }

  Scene_c3t3_item* item;
  C3t3 c3t3;
  Tree tree;
  QVector<QColor> colors;
  bool show_tetrahedra;
  bool is_aabb_tree_built;
};

struct Set_show_tetrahedra {
  Scene_c3t3_item_priv* priv;
  Set_show_tetrahedra(Scene_c3t3_item_priv* priv) : priv(priv) {}
  void operator()(bool b) {
    priv->show_tetrahedra = b;
    priv->item->show_intersection(b);
  }
};


double complex_diag(const Scene_item* item) {
  const Scene_item::Bbox& bbox = item->bbox();
  const double& xdelta = bbox.xmax()-bbox.xmin();
  const double& ydelta = bbox.ymax()-bbox.ymin();
  const double& zdelta = bbox.zmax()-bbox.zmin();
  const double diag = std::sqrt(xdelta*xdelta +
                                ydelta*ydelta +
                                zdelta*zdelta);
  return diag * 0.7;
}

Scene_c3t3_item::Scene_c3t3_item()
  : Scene_group_item("unnamed", NumberOfBuffers, NumberOfVaos)
  , d(new Scene_c3t3_item_priv(this))
  , frame(new ManipulatedFrame())
  , data_item_(NULL)
  , histogram_()
  , indices_()
{
  are_intersection_buffers_filled = false;
  positions_lines.resize(0);
  positions_poly.resize(0);
  normals.resize(0);
  s_vertex.resize(0);
  s_normals.resize(0);
  ws_vertex.resize(0);
  need_changed = false;
  spheres = NULL;
  intersection = NULL;
  compute_bbox();
  connect(frame, SIGNAL(modified()), this, SLOT(changed()));
  c3t3_changed();
  setRenderingMode(FlatPlusEdges);
  spheres_are_shown = false;
  cnc_are_shown = false;
    create_flat_and_wire_sphere(1.0f,s_vertex,s_normals, ws_vertex);

}

Scene_c3t3_item::Scene_c3t3_item(const C3t3& c3t3)
  : Scene_group_item("unnamed", NumberOfBuffers, NumberOfVaos)
  , d(new Scene_c3t3_item_priv(c3t3, this))
  , frame(new ManipulatedFrame())
  , data_item_(NULL)  
  , histogram_()
  , indices_()
{
  positions_lines.resize(0);
  positions_poly.resize(0);
  normals.resize(0);
  s_vertex.resize(0);
  s_normals.resize(0);
  ws_vertex.resize(0);
  need_changed = false;
  spheres = NULL;
  intersection = NULL;
  compute_bbox();
  connect(frame, SIGNAL(modified()), this, SLOT(changed()));
  reset_cut_plane();
  c3t3_changed();
  setRenderingMode(FlatPlusEdges);
  spheres_are_shown = false;
  cnc_are_shown = false;
  create_flat_and_wire_sphere(1.0f,s_vertex,s_normals, ws_vertex);
}

Scene_c3t3_item::~Scene_c3t3_item()
{
  frame = 0;
  delete frame;
  delete d;
}



const Scene_item*
Scene_c3t3_item::data_item() const
{
  return data_item_;
}

void
Scene_c3t3_item::set_data_item(const Scene_item* data_item)
{
  data_item_ = data_item;
  if (NULL != data_item)
  {
    connect(data_item, SIGNAL(aboutToBeDestroyed()),
      this, SLOT(data_item_destroyed()));
  }
}

void
Scene_c3t3_item::data_item_destroyed()
{
  set_data_item(NULL);
}

const C3t3&
Scene_c3t3_item::c3t3() const {
  return d->c3t3;
}

C3t3&
Scene_c3t3_item::c3t3()
{
  return d->c3t3;
}

void
Scene_c3t3_item::changed()
{
  need_changed = true;
  QTimer::singleShot(0,this, SLOT(updateCutPlane()));
}

void Scene_c3t3_item::updateCutPlane()
{ // just handle deformation - paint like selection is handled in eventFilter()
  if(need_changed) {
    are_intersection_buffers_filled = false;
    need_changed = false;
  }
}

void
Scene_c3t3_item::c3t3_changed()
{
  // Update colors
  // Fill indices map and get max subdomain value
  indices_.clear();

  int max = 0;
  for (C3t3::Cells_in_complex_iterator cit = this->c3t3().cells_in_complex_begin(),
    end = this->c3t3().cells_in_complex_end(); cit != end; ++cit)
  {
    max = (std::max)(max, cit->subdomain_index());
    indices_.insert(cit->subdomain_index());
  }
  for (C3t3::Facets_in_complex_iterator fit = this->c3t3().facets_in_complex_begin(),
    end = this->c3t3().facets_in_complex_end(); fit != end; ++fit)
  {
    max = (std::max)(max, fit->first->surface_patch_index(fit->second));
    indices_.insert(fit->first->surface_patch_index(fit->second));
  }

  d->colors.resize(max + 1);
  compute_color_map(color_);

  // Rebuild histogram
  build_histogram();

  d->tree.clear();
  d->is_aabb_tree_built = false;
}

QPixmap
Scene_c3t3_item::graphicalToolTip() const
{
  if (!histogram_.isNull())
  {
    return histogram_;
  }
  else
  {
    const_cast<Scene_c3t3_item&>(*this).build_histogram();
    return histogram_;
  }
}

template<typename C3t3>
std::vector<int>
create_histogram(const C3t3& c3t3, double& min_value, double& max_value)
{
  typedef typename C3t3::Triangulation::Point Point_3;

  std::vector<int> histo(181, 0);

  min_value = 180.;
  max_value = 0.;

  for (typename C3t3::Cells_in_complex_iterator cit = c3t3.cells_in_complex_begin();
    cit != c3t3.cells_in_complex_end();
    ++cit)
  {
    if (!c3t3.is_in_complex(cit))
      continue;

#ifdef CGAL_MESH_3_DEMO_DONT_COUNT_TETS_ADJACENT_TO_SHARP_FEATURES_FOR_HISTOGRAM
    if (c3t3.in_dimension(cit->vertex(0)) <= 1
      || c3t3.in_dimension(cit->vertex(1)) <= 1
      || c3t3.in_dimension(cit->vertex(2)) <= 1
      || c3t3.in_dimension(cit->vertex(3)) <= 1)
      continue;
#endif //CGAL_MESH_3_DEMO_DONT_COUNT_TETS_ADJACENT_TO_SHARP_FEATURES_FOR_HISTOGRAM

    const Point_3& p0 = cit->vertex(0)->point();
    const Point_3& p1 = cit->vertex(1)->point();
    const Point_3& p2 = cit->vertex(2)->point();
    const Point_3& p3 = cit->vertex(3)->point();

    double a = CGAL::to_double(CGAL::abs(CGAL::Mesh_3::dihedral_angle(p0, p1, p2, p3)));
    histo[static_cast<int>(std::floor(a))] += 1;
    min_value = (std::min)(min_value, a);
    max_value = (std::max)(max_value, a);

    a = CGAL::to_double(CGAL::abs(CGAL::Mesh_3::dihedral_angle(p0, p2, p1, p3)));
    histo[static_cast<int>(std::floor(a))] += 1;
    min_value = (std::min)(min_value, a);
    max_value = (std::max)(max_value, a);

    a = CGAL::to_double(CGAL::abs(CGAL::Mesh_3::dihedral_angle(p0, p3, p1, p2)));
    histo[static_cast<int>(std::floor(a))] += 1;
    min_value = (std::min)(min_value, a);
    max_value = (std::max)(max_value, a);

    a = CGAL::to_double(CGAL::abs(CGAL::Mesh_3::dihedral_angle(p1, p2, p0, p3)));
    histo[static_cast<int>(std::floor(a))] += 1;
    min_value = (std::min)(min_value, a);
    max_value = (std::max)(max_value, a);

    a = CGAL::to_double(CGAL::abs(CGAL::Mesh_3::dihedral_angle(p1, p3, p0, p2)));
    histo[static_cast<int>(std::floor(a))] += 1;
    min_value = (std::min)(min_value, a);
    max_value = (std::max)(max_value, a);

    a = CGAL::to_double(CGAL::abs(CGAL::Mesh_3::dihedral_angle(p2, p3, p0, p1)));
    histo[static_cast<int>(std::floor(a))] += 1;
    min_value = (std::min)(min_value, a);
    max_value = (std::max)(max_value, a);

  }

  return histo;
}

void
Scene_c3t3_item::build_histogram()
{
#ifdef CGAL_MESH_3_DEMO_BIGGER_HISTOGRAM_WITH_WHITE_BACKGROUNG
  // Create an histogram_ and display it
  const int height = 280;
  const int top_margin = 5;
  const int left_margin = 20;
  const int drawing_height = height - top_margin * 2;
  const int width = 804;
  const int cell_width = 4;
  const int text_margin = 3;
  const int text_height = 34;

  histogram_ = QPixmap(width, height + text_height);
  histogram_.fill(QColor(255, 255, 255));
#else
  // Create an histogram_ and display it
  const int height = 140;
  const int top_margin = 5;
  const int left_margin = 20;
  const int drawing_height = height - top_margin * 2;
  const int width = 402;
  const int cell_width = 2;
  const int text_margin = 3;
  const int text_height = 20;

  histogram_ = QPixmap(width, height + text_height);
  histogram_.fill(QColor(192, 192, 192));
#endif  

  QPainter painter(&histogram_);
  painter.setPen(Qt::black);
  painter.setBrush(QColor(128, 128, 128));
  //painter.setFont(QFont("Arial", 30));

  // Build histogram_ data
  double min_value, max_value;
  std::vector<int> histo_data = create_histogram(c3t3(), min_value, max_value);

  // Get maximum value (to normalize)
  int max_size = 0;
  for (std::vector<int>::iterator it = histo_data.begin(), end = histo_data.end();
    it != end; ++it)
  {
    max_size = (std::max)(max_size, *it);
  }

  // colored histogram
  int j = 0;

  // draw
  int i = left_margin;
  for (std::vector<int>::iterator it = histo_data.begin(), end = histo_data.end();
    it != end; ++it, i += cell_width)
  {
    int line_height = static_cast<int>(std::ceil(static_cast<double>(drawing_height)*
      static_cast<double>(*it) / static_cast<double>(max_size)) + .5);

    painter.fillRect(i,
      drawing_height + top_margin - line_height,
      cell_width,
      line_height,
      get_histogram_color(j++));
  }

  // draw bottom horizontal line
  painter.setPen(Qt::blue);

  painter.drawLine(QPoint(left_margin, drawing_height + top_margin),
    QPoint(left_margin + static_cast<int>(histo_data.size())*cell_width,
    drawing_height + top_margin));


  // draw min value and max value
  const int min_tr_width = static_cast<int>(2 * (std::floor(min_value)*cell_width + left_margin));
  const int max_tr_width = static_cast<int>(
    2 * ((histo_data.size() - std::floor(max_value))*cell_width + left_margin));
  const int tr_y = drawing_height + top_margin + text_margin;

  painter.setPen(get_histogram_color(min_value));
  QRect min_text_rect(0, tr_y, min_tr_width, text_height);
  painter.drawText(min_text_rect, Qt::AlignCenter, tr("%1").arg(min_value, 0, 'f', 1));

  painter.setPen(get_histogram_color(max_value));
  QRect max_text_rect(width - max_tr_width, tr_y, max_tr_width, text_height);
  painter.drawText(max_text_rect, Qt::AlignCenter, tr("%1").arg(max_value, 0, 'f', 1));
}

QColor
Scene_c3t3_item::get_histogram_color(const double v) const
{
  if (v < 5)            { return Qt::red; }
  else if (v < 10)      { return QColor(215, 108, 0); }
  else if (v < 15)      { return QColor(138, 139, 0); }
  else if (v < 165)     { return QColor(60, 136, 64); }
  else if (v < 170)     { return QColor(138, 139, 1); }
  else if (v < 175)     { return QColor(215, 108, 0); }
  else /* 175<v<=180 */   { return Qt::red; }
}

void
Scene_c3t3_item::update_histogram()
{
  build_histogram();
}

void
Scene_c3t3_item::compute_color_map(const QColor& c)
{
  typedef Indices::size_type size_type;

  size_type nb_domains = indices_.size();
  size_type i = 0;
  for (Indices::iterator it = indices_.begin(), end = indices_.end();
    it != end; ++it, ++i)
  {
    double hue = c.hueF() + 1. / nb_domains * i;
    if (hue > 1) { hue -= 1.; }
    d->colors[*it] = QColor::fromHsvF(hue, c.saturationF(), c.valueF());
  }
}

Kernel::Plane_3 Scene_c3t3_item::plane() const {
  const qglviewer::Vec& pos = frame->position();
  const qglviewer::Vec& n =
    frame->inverseTransformOf(qglviewer::Vec(0.f, 0.f, 1.f));
  return Kernel::Plane_3(n[0], n[1], n[2], -n * pos);
}

void Scene_c3t3_item::compute_bbox() const {
  if (isEmpty())
    _bbox = Bbox();
  else {
    CGAL::Bbox_3 result;
    for (Tr::Finite_vertices_iterator
           vit = ++c3t3().triangulation().finite_vertices_begin(),
           end = c3t3().triangulation().finite_vertices_end();
         vit != end; ++vit)
    {
      if(vit->in_dimension() == -1) continue;
      result = result + vit->point().bbox();
    }
    _bbox = Bbox(result.xmin(), result.ymin(), result.zmin(),
                 result.xmax(), result.ymax(), result.zmax());
  }
}

QString Scene_c3t3_item::toolTip() const {
  return tr("<p><b>3D complex in a 3D triangulation</b></p>"
    "<p>Number of vertices: %1<br />"
    "Number of surface facets: %2<br />"
    "Number of volume tetrahedra: %3</p>")
    .arg(c3t3().triangulation().number_of_vertices())
    .arg(c3t3().number_of_facets_in_complex())
    .arg(c3t3().number_of_cells_in_complex());
}

void Scene_c3t3_item::draw(CGAL::Three::Viewer_interface* viewer) const {
  Scene_c3t3_item* ncthis = const_cast<Scene_c3t3_item*>(this);

  if (!are_buffers_filled)
  {
    ncthis->computeElements();
    ncthis->initializeBuffers(viewer);
  }

  vaos[Grid]->bind();
  program = getShaderProgram(PROGRAM_WITHOUT_LIGHT);
  attribBuffers(viewer, PROGRAM_WITHOUT_LIGHT);
  program->bind();
  program->setAttributeValue("colors", QColor(Qt::black));
  QMatrix4x4 f_mat;
  for (int i = 0; i<16; i++)
    f_mat.data()[i] = frame->matrix()[i];
  program->setUniformValue("f_matrix", f_mat);
  viewer->glDrawArrays(GL_LINES, 0, static_cast<GLsizei>(positions_grid.size() / 3));
  program->release();
  vaos[Grid]->release();

  vaos[Facets]->bind();
  program = getShaderProgram(PROGRAM_C3T3);
  attribBuffers(viewer, PROGRAM_C3T3);
  program->bind();
  QVector4D cp(this->plane().a(),this->plane().b(),this->plane().c(),this->plane().d());
  program->setUniformValue("cutplane", cp);
  // positions_poly_size is the number of total facets in the C3T3
  // it is only computed once and positions_poly is emptied at the end
  viewer->glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(positions_poly_size / 3));
  program->release();
  vaos[Facets]->release();

  if(d->show_tetrahedra){
    if(!frame->isManipulated() && !are_intersection_buffers_filled)
    {
      if(!intersection->visible())
        intersection->setVisible(true);
      ncthis->computeIntersections();
      intersection->initialize_buffers(viewer);
      are_intersection_buffers_filled = true;
    }
    else if(frame->isManipulated() && intersection->visible())
      intersection->setVisible(false);
  }

  if(spheres_are_shown)
  {
    spheres->setPlane(this->plane());
  }
  Scene_group_item::draw(viewer);
}

void Scene_c3t3_item::drawEdges(CGAL::Three::Viewer_interface* viewer) const {
  if(renderingMode() == FlatPlusEdges)
  {
    GLint renderMode;
    glGetIntegerv(GL_RENDER_MODE, &renderMode);
    if(renderMode == GL_SELECT) return;
  }
  Scene_c3t3_item* ncthis = const_cast<Scene_c3t3_item*>(this);
  if (!are_buffers_filled)
  {
    ncthis->computeElements();
    ncthis->initializeBuffers(viewer);
  }

  if(renderingMode() == Wireframe)
  {
    vaos[Grid]->bind();

    program = getShaderProgram(PROGRAM_NO_SELECTION);
    attribBuffers(viewer, PROGRAM_NO_SELECTION);
    program->bind();
    program->setAttributeValue("colors", QColor(Qt::black));
    QMatrix4x4 f_mat;
    for (int i = 0; i<16; i++)
        f_mat.data()[i] = frame->matrix()[i];
    program->setUniformValue("f_matrix", f_mat);
    viewer->glDrawArrays(GL_LINES, 0, static_cast<GLsizei>(positions_grid.size() / 3));
    program->release();
    vaos[Grid]->release();
  }
  vaos[Edges]->bind();
  program = getShaderProgram(PROGRAM_C3T3_EDGES);
  attribBuffers(viewer, PROGRAM_C3T3_EDGES);
  program->bind();
  QVector4D cp(this->plane().a(),this->plane().b(),this->plane().c(),this->plane().d());
  program->setUniformValue("cutplane", cp);
  program->setAttributeValue("colors", QColor(Qt::black));
  viewer->glDrawArrays(GL_LINES, 0, static_cast<GLsizei>(positions_lines_size / 3));
  program->release();
  vaos[Edges]->release();

  if(d->show_tetrahedra){
    if(!frame->isManipulated() && !are_intersection_buffers_filled)
    {
      if(!intersection->visible())
        intersection->setVisible(true);
      ncthis->computeIntersections();
      intersection->initialize_buffers(viewer);
      are_intersection_buffers_filled = true;
    }
    else if(frame->isManipulated() && intersection->visible())
      intersection->setVisible(false);
  }
  if(spheres_are_shown)
  {
      spheres->setPlane(this->plane());
  }
  Scene_group_item::drawEdges(viewer);
  if(cnc_are_shown)
  {
    vaos[CNC]->bind();
    program = getShaderProgram(PROGRAM_NO_SELECTION);
    attribBuffers(viewer, PROGRAM_NO_SELECTION);
    program->bind();
    program->setAttributeValue("colors", QColor(Qt::black));
    viewer->glDrawArrays(GL_LINES, 0, static_cast<GLsizei>(positions_lines_not_in_complex_size / 3));
    program->release();
    vaos[Edges]->release();
  }
}

void Scene_c3t3_item::drawPoints(CGAL::Three::Viewer_interface * viewer) const
{
  Scene_c3t3_item* ncthis = const_cast<Scene_c3t3_item*>(this);
  if (!are_buffers_filled)
  {
    ncthis->computeElements();
    ncthis-> initializeBuffers(viewer);
  }
  vaos[Edges]->bind();
  program = getShaderProgram(PROGRAM_C3T3_EDGES);
  attribBuffers(viewer, PROGRAM_C3T3_EDGES);
  program->bind();
  QVector4D cp(this->plane().a(),this->plane().b(),this->plane().c(),this->plane().d());
  program->setUniformValue("cutplane", cp);
  program->setAttributeValue("colors", this->color());
  viewer->glDrawArrays(GL_POINTS, 0, static_cast<GLsizei>(positions_lines.size() / 3));
  vaos[Edges]->release();
  program->release();

  vaos[Grid]->bind();
  program = getShaderProgram(PROGRAM_NO_SELECTION);
  attribBuffers(viewer, PROGRAM_NO_SELECTION);
  program->bind();
  program->setAttributeValue("colors", this->color());
  QMatrix4x4 f_mat;
  for (int i = 0; i<16; i++)
    f_mat.data()[i] = frame->matrix()[i];
  program->setUniformValue("f_matrix", f_mat);
  viewer->glDrawArrays(GL_LINES, 0, static_cast<GLsizei>(positions_grid.size() / 3));
  program->release();
  vaos[Grid]->release();
  if(spheres_are_shown)
  {
    spheres->setPlane(this->plane());
  }
  Scene_group_item::drawEdges(viewer);

}

void Scene_c3t3_item::draw_triangle(const Kernel::Point_3& pa,
  const Kernel::Point_3& pb,
  const Kernel::Point_3& pc) const
{

  #undef darker
  Kernel::Vector_3 n = cross_product(pb - pa, pc - pa);
  n = n / CGAL::sqrt(n*n);


  for (int i = 0; i<3; i++)
  {
    normals.push_back(n.x());
    normals.push_back(n.y());
    normals.push_back(n.z());
  }
  positions_poly.push_back(pa.x());
  positions_poly.push_back(pa.y());
  positions_poly.push_back(pa.z());

  positions_poly.push_back(pb.x());
  positions_poly.push_back(pb.y());
  positions_poly.push_back(pb.z());

  positions_poly.push_back(pc.x());
  positions_poly.push_back(pc.y());
  positions_poly.push_back(pc.z());



}

void Scene_c3t3_item::draw_triangle_edges(const Kernel::Point_3& pa,
  const Kernel::Point_3& pb,
  const Kernel::Point_3& pc)const {

#undef darker
  positions_lines.push_back(pa.x());
  positions_lines.push_back(pa.y());
  positions_lines.push_back(pa.z());

  positions_lines.push_back(pb.x());
  positions_lines.push_back(pb.y());
  positions_lines.push_back(pb.z());

  positions_lines.push_back(pb.x());
  positions_lines.push_back(pb.y());
  positions_lines.push_back(pb.z());

  positions_lines.push_back(pc.x());
  positions_lines.push_back(pc.y());
  positions_lines.push_back(pc.z());

  positions_lines.push_back(pc.x());
  positions_lines.push_back(pc.y());
  positions_lines.push_back(pc.z());

  positions_lines.push_back(pa.x());
  positions_lines.push_back(pa.y());
  positions_lines.push_back(pa.z());

}
void Scene_c3t3_item::draw_triangle_edges_cnc(const Kernel::Point_3& pa,
                                          const Kernel::Point_3& pb,
                                          const Kernel::Point_3& pc)const {

#undef darker
  positions_lines_not_in_complex.push_back(pa.x());
  positions_lines_not_in_complex.push_back(pa.y());
  positions_lines_not_in_complex.push_back(pa.z());

  positions_lines_not_in_complex.push_back(pb.x());
  positions_lines_not_in_complex.push_back(pb.y());
  positions_lines_not_in_complex.push_back(pb.z());

  positions_lines_not_in_complex.push_back(pb.x());
  positions_lines_not_in_complex.push_back(pb.y());
  positions_lines_not_in_complex.push_back(pb.z());

  positions_lines_not_in_complex.push_back(pc.x());
  positions_lines_not_in_complex.push_back(pc.y());
  positions_lines_not_in_complex.push_back(pc.z());

  positions_lines_not_in_complex.push_back(pc.x());
  positions_lines_not_in_complex.push_back(pc.y());
  positions_lines_not_in_complex.push_back(pc.z());

  positions_lines_not_in_complex.push_back(pa.x());
  positions_lines_not_in_complex.push_back(pa.y());
  positions_lines_not_in_complex.push_back(pa.z());

}

double Scene_c3t3_item::complex_diag() const {
  const Bbox& bbox = this->bbox();
  const double& xdelta = bbox.xmax() - bbox.xmin();
  const double& ydelta = bbox.ymax() - bbox.ymin();
  const double& zdelta = bbox.zmax() - bbox.zmin();
  const double diag = std::sqrt(xdelta*xdelta +
    ydelta*ydelta +
    zdelta*zdelta);
  return diag * 0.7;
}

void Scene_c3t3_item::export_facets_in_complex()
{
  std::stringstream off_sstream;
  c3t3().output_facets_in_complex_to_off(off_sstream);
  std::string backup = off_sstream.str();
  // Try to read .off in a polyhedron
  Scene_polyhedron_item* item = new Scene_polyhedron_item;
  if (!item->load(off_sstream))
  {
    delete item;
    off_sstream.str(backup);

    // Try to read .off in a polygon soup
    Scene_polygon_soup_item* soup_item = new Scene_polygon_soup_item;

    if (!soup_item->load(off_sstream)) {
      delete soup_item;
      return;
    }

    soup_item->setName(QString("%1_%2").arg(this->name()).arg("facets"));
    scene->addItem(soup_item);
  }
  else{
    item->setName(QString("%1_%2").arg(this->name()).arg("facets"));
    scene->addItem(item);
  }
}

QMenu* Scene_c3t3_item::contextMenu()
{
  const char* prop_name = "Menu modified by Scene_c3t3_item.";

  QMenu* menu = Scene_item::contextMenu();

  // Use dynamic properties:
  // http://doc.qt.io/qt-5/qobject.html#property
  bool menuChanged = menu->property(prop_name).toBool();

  if (!menuChanged) {
    QAction* actionExportFacetsInComplex =
      menu->addAction(tr("Export facets in complex"));
    actionExportFacetsInComplex->setObjectName("actionExportFacetsInComplex");
    connect(actionExportFacetsInComplex,
      SIGNAL(triggered()), this,
      SLOT(export_facets_in_complex()));

    QAction* actionShowSpheres =
      menu->addAction(tr("Show protecting &spheres"));
    actionShowSpheres->setCheckable(true);
    actionShowSpheres->setObjectName("actionShowSpheres");
    connect(actionShowSpheres, SIGNAL(toggled(bool)),
            this, SLOT(show_spheres(bool)));

    QAction* actionShowCNC =
      menu->addAction(tr("Show cells not in complex"));
    actionShowCNC->setCheckable(true);
    actionShowCNC->setObjectName("actionShowCNC");
    connect(actionShowCNC, SIGNAL(toggled(bool)),
            this, SLOT(show_cnc(bool)));

    QAction* actionShowTets =
      menu->addAction(tr("Show &tetrahedra"));
    actionShowTets->setCheckable(true);
    actionShowTets->setObjectName("actionShowTets");
    connect(actionShowTets, &QAction::toggled, Set_show_tetrahedra(this->d));

    menu->setProperty(prop_name, true);
  }
  return menu;
}


void Scene_c3t3_item::initializeBuffers(CGAL::Three::Viewer_interface *viewer)
{
  //vao containing the data for the facets
  {
    program = getShaderProgram(PROGRAM_C3T3, viewer);
    program->bind();

    vaos[Facets]->bind();
    buffers[Facet_vertices].bind();
    buffers[Facet_vertices].allocate(positions_poly.data(),
      static_cast<int>(positions_poly.size()*sizeof(float)));
    program->enableAttributeArray("vertex");
    program->setAttributeBuffer("vertex", GL_FLOAT, 0, 3);
    buffers[Facet_vertices].release();

    buffers[Facet_normals].bind();
    buffers[Facet_normals].allocate(normals.data(),
      static_cast<int>(normals.size()*sizeof(float)));
    program->enableAttributeArray("normals");
    program->setAttributeBuffer("normals", GL_FLOAT, 0, 3);
    buffers[Facet_normals].release();

    buffers[Facet_colors].bind();
    buffers[Facet_colors].allocate(f_colors.data(),
      static_cast<int>(f_colors.size()*sizeof(float)));
    program->enableAttributeArray("colors");
    program->setAttributeBuffer("colors", GL_FLOAT, 0, 3);
    buffers[Facet_colors].release();

    vaos[Facets]->release();
    program->release();
    
    positions_poly_size = positions_poly.size();
    positions_poly.clear();
    positions_poly.swap(positions_poly);
    normals.clear();
    normals.swap(normals);
    f_colors.clear();
    f_colors.swap(f_colors);
  }

  //vao containing the data for the lines
  {
    program = getShaderProgram(PROGRAM_C3T3_EDGES, viewer);
    program->bind();

    vaos[Edges]->bind();
    buffers[Edges_vertices].bind();
    buffers[Edges_vertices].allocate(positions_lines.data(),
                                     static_cast<int>(positions_lines.size()*sizeof(float)));
    program->enableAttributeArray("vertex");
    program->setAttributeBuffer("vertex", GL_FLOAT, 0, 3);
    buffers[Edges_vertices].release();

    vaos[Edges]->release();
    program->release();

    positions_lines_size = positions_lines.size();
    positions_lines.clear();
    positions_lines.swap(positions_lines);
    
  }

  // vao containing the data for the cnc
  {
    program = getShaderProgram(PROGRAM_NO_SELECTION, viewer);
    program->bind();

    vaos[CNC]->bind();
    buffers[Edges_CNC].bind();
    buffers[Edges_CNC].allocate(positions_lines_not_in_complex.data(),
                                     static_cast<int>(positions_lines_not_in_complex.size()*sizeof(float)));
    program->enableAttributeArray("vertex");
    program->setAttributeBuffer("vertex", GL_FLOAT, 0, 3);
    buffers[Edges_CNC].release();

    vaos[CNC]->release();
    program->release();

    positions_lines_not_in_complex_size = positions_lines_not_in_complex.size();
    positions_lines_not_in_complex.clear();
    positions_lines_not_in_complex.swap(positions_lines_not_in_complex);

  }

  //vao containing the data for the grid
  {
    program = getShaderProgram(PROGRAM_NO_SELECTION, viewer);
    program->bind();

    vaos[Grid]->bind();
    buffers[Grid_vertices].bind();
    buffers[Grid_vertices].allocate(positions_grid.data(),
                                    static_cast<int>(positions_grid.size()*sizeof(float)));
    program->enableAttributeArray("vertex");
    program->setAttributeBuffer("vertex", GL_FLOAT, 0, 3);
    buffers[Grid_vertices].release();
    vaos[Grid]->release();
    program->release();
  }

    program->release();
    are_buffers_filled = true;
}



void Scene_c3t3_item_priv::computeIntersection(const Primitive& facet)
{
  const Kernel::Point_3& pa = facet.id().first->vertex(0)->point();
  const Kernel::Point_3& pb = facet.id().first->vertex(1)->point();
  const Kernel::Point_3& pc = facet.id().first->vertex(2)->point();
  const Kernel::Point_3& pd = facet.id().first->vertex(3)->point();

  QColor c = this->colors[facet.id().first->subdomain_index()].darker(150);

  CGAL::Color color(c.red(), c.green(), c.blue());

  item->intersection->addTriangle(pb, pa, pc, color);
  item->intersection->addTriangle(pa, pb, pd, color);
  item->intersection->addTriangle(pa, pd, pc, color);
  item->intersection->addTriangle(pb, pc, pd, color);

  {
    Tr::Cell_handle nh = facet.id().first->neighbor(facet.id().second);
    if(c3t3.is_in_complex(nh)){
      const Kernel::Point_3& pa = nh->vertex(0)->point();
      const Kernel::Point_3& pb = nh->vertex(1)->point();
      const Kernel::Point_3& pc = nh->vertex(2)->point();
      const Kernel::Point_3& pd = nh->vertex(3)->point();

      item->intersection->addTriangle(pb, pa, pc, color);
      item->intersection->addTriangle(pa, pb, pd, color);
      item->intersection->addTriangle(pa, pd, pc, color);
      item->intersection->addTriangle(pb, pc, pd, color);
    }
  }

}

struct ComputeIntersection {
  Scene_c3t3_item_priv& item_priv;

  ComputeIntersection(Scene_c3t3_item_priv& item_priv)
    : item_priv(item_priv)
  {}

  void operator()(const Primitive& facet) const
  {
    item_priv.computeIntersection(facet);
  }
};

void Scene_c3t3_item::computeIntersections()
{
  if(!d->is_aabb_tree_built) d->fill_aabb_tree();

  positions_poly.clear();
  normals.clear();
  f_colors.clear();
  positions_lines.clear();
  const Kernel::Plane_3& plane = this->plane();
  d->tree.all_intersected_primitives(plane,
        boost::make_function_output_iterator(ComputeIntersection(*this->d)));
}

void Scene_c3t3_item::computeSpheres()
{
  if(!spheres)
    return;
  for(Tr::Finite_vertices_iterator
      vit = d->c3t3.triangulation().finite_vertices_begin(),
      end =  d->c3t3.triangulation().finite_vertices_end();
      vit != end; ++vit)
  {
    if(vit->point().weight()==0) continue;

    typedef Tr::Vertex_handle Vertex_handle;
    std::vector<Vertex_handle> incident_vertices;
    d->c3t3.triangulation().incident_vertices(vit, std::back_inserter(incident_vertices));
    bool red = vit->is_special();
    for(std::vector<Vertex_handle>::const_iterator
        vvit = incident_vertices.begin(), end = incident_vertices.end();
        vvit != end; ++vvit)
    {
      if(Kernel::Sphere_3(vit->point().point(),
                          vit->point().weight()).bounded_side((*vvit)->point().point())
         == CGAL::ON_BOUNDED_SIDE)
        red = true;
    }
    QColor c;
    if(red)
      c = QColor(Qt::red);
    else
      c = spheres->color().darker(250);
    Kernel::Point_3 center(vit->point().point().x(),
    vit->point().point().y(),
    vit->point().point().z());
    float radius = CGAL::sqrt(vit->point().weight());
    Kernel::Sphere_3* sphere = new Kernel::Sphere_3(center, radius);
    spheres->add_sphere(sphere, CGAL::Color(c.red(), c.green(), c.blue()));
  }
  spheres->invalidateOpenGLBuffers();
}

void Scene_c3t3_item::computeElements()
{
  positions_poly.clear();
  normals.clear();
  f_colors.clear();
  positions_lines.clear();
  positions_lines_not_in_complex.clear();
  s_colors.resize(0);
  s_center.resize(0);
  s_radius.resize(0);


  //The grid
  {
    float x = (2 * (float)complex_diag()) / 10.0;
    float y = (2 * (float)complex_diag()) / 10.0;
    for (int u = 0; u < 11; u++)
    {

      positions_grid.push_back(-(float)complex_diag() + x* u);
      positions_grid.push_back(-(float)complex_diag());
      positions_grid.push_back(0.0);

      positions_grid.push_back(-(float)complex_diag() + x* u);
      positions_grid.push_back((float)complex_diag());
      positions_grid.push_back(0.0);
    }
    for (int v = 0; v<11; v++)
    {

      positions_grid.push_back(-(float)complex_diag());
      positions_grid.push_back(-(float)complex_diag() + v * y);
      positions_grid.push_back(0.0);

      positions_grid.push_back((float)complex_diag());
      positions_grid.push_back(-(float)complex_diag() + v * y);
      positions_grid.push_back(0.0);
    }
  }


  if (isEmpty()){
    return;
  }
  //The facets
  {  
    for (C3t3::Facet_iterator
      fit = c3t3().facets_begin(),
      end = c3t3().facets_end();
    fit != end; ++fit)
    {
      const Tr::Cell_handle& cell = fit->first;
      const int& index = fit->second;
      const Kernel::Point_3& pa = cell->vertex((index + 1) & 3)->point();
      const Kernel::Point_3& pb = cell->vertex((index + 2) & 3)->point();
      const Kernel::Point_3& pc = cell->vertex((index + 3) & 3)->point();

      QColor color = d->colors[cell->surface_patch_index(index)];
      f_colors.push_back(color.redF());f_colors.push_back(color.greenF());f_colors.push_back(color.blueF());
      f_colors.push_back(color.redF());f_colors.push_back(color.greenF());f_colors.push_back(color.blueF());
      f_colors.push_back(color.redF());f_colors.push_back(color.greenF());f_colors.push_back(color.blueF());
      if ((index % 2 == 1) == c3t3().is_in_complex(cell))
        draw_triangle(pb, pa, pc);
      else draw_triangle(pa, pb, pc);
      draw_triangle_edges(pa, pb, pc);
    }
    //Kernel::Point_3 p0(10, 10, 10);
    //c3t3().add_far_point(p0);
    //the cells not in the complex
    for(C3t3::Triangulation::Cell_iterator
        cit = c3t3().triangulation().finite_cells_begin(),
        end = c3t3().triangulation().finite_cells_end();
        cit != end; ++cit)
    {
      if(!c3t3().is_in_complex(cit))
      {

        bool has_far_point = false;
        for(int i=0; i<4; i++)
          if(c3t3().in_dimension(cit->vertex(i)) == -1)
          {
            has_far_point = true;
            break;
          }
        if(!has_far_point)
        {
          const Kernel::Point_3& p1 = cit->vertex(0)->point();
          const Kernel::Point_3& p2 = cit->vertex(1)->point();
          const Kernel::Point_3& p3 = cit->vertex(2)->point();
          const Kernel::Point_3& p4 = cit->vertex(3)->point();
          draw_triangle_edges_cnc(p1, p2, p4);
          draw_triangle_edges_cnc(p1, p3, p4);
          draw_triangle_edges_cnc(p2, p3, p4);
          draw_triangle_edges_cnc(p1, p2, p3);
        }
      }
    }

  }
}

bool Scene_c3t3_item::load_binary(std::istream& is)
{
  if(!CGAL::Mesh_3::load_binary_file(is, c3t3())) return false;
  if(is && frame == 0) {
    frame = new qglviewer::ManipulatedFrame();
  }
  reset_cut_plane();
  if(is.good()) {
    c3t3_changed();
    changed();
    return true;
  }
  else
    return false;
}

void
Scene_c3t3_item::reset_cut_plane() {
  const Bbox& bbox = this->bbox();
  const float xcenter = static_cast<float>((bbox.xmax()+bbox.xmin())/2.);
  const float ycenter = static_cast<float>((bbox.ymax()+bbox.ymin())/2.);
  const float zcenter = static_cast<float>((bbox.zmax()+bbox.zmin())/2.);

  frame->setPosition(qglviewer::Vec(xcenter, ycenter, zcenter));
}

void
Scene_c3t3_item::setColor(QColor c)
{
  color_ = c;
  compute_color_map(c);
  invalidateOpenGLBuffers();
  are_intersection_buffers_filled = false;
}
void Scene_c3t3_item::show_spheres(bool b)
{
  spheres_are_shown = b;
  if(b && !spheres)
  {
    spheres = new Scene_spheres_item(this, true);
    spheres->setName("Protecting spheres");
    spheres->setRenderingMode(Gouraud);
    connect(spheres, SIGNAL(destroyed()), this, SLOT(reset_spheres()));
    scene->addItem(spheres);
    scene->changeGroup(spheres, this);
    lockChild(spheres);
    computeSpheres();
  }
  else if (!b && spheres!=NULL)
  {
    unlockChild(spheres);
    scene->erase(scene->item_id(spheres));
  }
  Q_EMIT redraw();

}
void Scene_c3t3_item::show_intersection(bool b)
{
  if(b && !intersection)
  {
    intersection = new Scene_intersection_item(this);
    intersection->init_vectors(&positions_poly,
                               &normals,
                               &positions_lines,
                               &f_colors);
    intersection->setName("Intersection tetrahedra");
    intersection->setRenderingMode(renderingMode());
    connect(intersection, SIGNAL(destroyed()), this, SLOT(reset_intersection_item()));
    scene->addItem(intersection);
    scene->changeGroup(intersection, this);
    lockChild(intersection);
    are_intersection_buffers_filled = false;
  }
  else if (!b && intersection!=NULL)
  {
    unlockChild(intersection);
    scene->erase(scene->item_id(intersection));
  }
  Q_EMIT redraw();

}
#include "Scene_c3t3_item.moc"
