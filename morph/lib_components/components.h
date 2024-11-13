/*!
 * Components are defined as composition of visuals.
 * The idea is to make basic blocks easy to apprehend for users. 
 *
 * Author: Fabien Colonnier
 * Date: Nov 2024
 */

#pragma once

#include <morph/vec.h>
#include <morph/vvec.h>

#include <morph/loadpng.h>
#include <morph/Visual.h>
#include <morph/GridFeatures.h>
#include <morph/Grid.h>
#include <morph/GridVisual.h>
#include <morph/ColourMap.h>
#include <morph/VectorVisual.h>
#include <morph/ColourMap.h>
#include <morph/ColourBarVisual.h>

namespace morph // TODO decide if it should stay in the same namespace or define a morph_components
{
  // Shorten the morph namespace
  namespace m = morph;    

  template <typename T, typename I = unsigned int, typename C = float, int gl_version = morph::gl::version_4_1>
  class SimpleGridVisual
  {
  private:
    float pixel_size;  // pixel size for the GridVisual
    m::GridOrder grid_order;  // set the grid origin and if rowmajor following the enum GridOrder (currently row major only supported)

    // pointer to the grid data
    std::unique_ptr<m::Grid<int>> data_gd;
  
    // pointers to the grid owning the original image
    m::GridVisual<T, I, C, gl_version> * gv_ptr = nullptr;
  
    /*!
     * @brief create a default image to use at initialization of the grids
     *
     * @param img_w
     * @param img_h
     * @return m::vvec<m::vec<float, 3>>  image
     */
    m::vvec<m::vec<float, 3>> get_default_image(unsigned int img_w, unsigned int img_h)
    {
      constexpr m::vec<float, 3> nullvec = {0.0f, 0.0f, 0.0f};
      unsigned int img_size = img_w * img_h;

      m::vvec<m::vec<float, 3>> default_img;
      default_img.resize(img_size, nullvec);

      for (unsigned int c = 0; c < img_h; ++c) {
        for (unsigned int r = 0; r < img_w; ++r) {
          unsigned int idx = r + img_w * (img_h - c - 1);
          default_img[idx][0] = static_cast<float>(idx) / (img_size);
          default_img[idx][1] = static_cast<float>(idx) / (img_size);
          default_img[idx][2] = static_cast<float>(idx) / (img_size);
        }
      }

      return default_img;
    }
  
    /*!
     * @brief Create a grid to display the image by defining size of pixels and size of the image
     *
     * @param img_w image width in pixel
     * @param img_h image height in pixel
     * @param grid_pix_size pixel size of the grid in the display
     */
    void create_grid_img(
      unsigned int img_w, unsigned int img_h, float grid_pix_size)
    {
      // create a grid to visualize the image input
      m::vec<float, 2> grid_spacing = {grid_pix_size, grid_pix_size};
      m::vec<float, 2> grid_offset = grid_spacing * -0.5f;

      this->data_gd = std::make_unique<m::Grid<int>>(
        img_w, img_h, grid_spacing, grid_offset,
        m::GridDomainWrap::Horizontal,
        this->grid_order);

      if (this->data_gd->n > static_cast<int>(img_w * img_h)) {
        throw std::runtime_error("Grid wrong size");
      }
    }
  
    /*!
     * @brief Create a grid visualizer that can display vector
     *
     * @param v_ref reference to the Visual object managing the openGL window
     * @param grid_data_ptr pointer to the grid that will hold the data
     * @param offset location offset relative to the scene center
     * @param default_data an example of data to be plotted at initialization
     * @param colormap colourmap type used to display
     * @param grid_title grid title to be written below bottom left
     * @param title_location title location, within the model coordinates. Default value provided
     * @return m::GridVisual<float, int, float, gl_version>* a non owning pointer to the grid to update the data
     */
    m::GridVisual<T, I, C, gl_version> * create_grid_visualizer(
      m::Visual<gl_version> & v_ref,
      m::vec<float> & offset,
      m::vvec<m::vec<float, 3>> & default_data,
      m::ColourMapType colormap,
      std::string grid_title,
      m::vec<float, 3> title_location
    )
    {
  auto cgv =
    std::make_unique<m::GridVisual<float, int, float, gl_version>>(this->data_gd.get(), offset);
  v_ref.bindmodel(cgv);

  cgv->setVectorData(&default_data);
  cgv->cm.setType(colormap);
  cgv->zScale.setParams(0.0f, 0.0f);
  cgv->addLabel(std::string(grid_title), title_location);

  cgv->finalize();

  return v_ref.addVisualModel(cgv);
}
  
    /*!
     * @brief Set the colourbar object for a given grid
     *
     * @param v_ref reference to the Visual object managing the openGL window
     * @param colour_bar_location location of the colourbar in the window
     * @param colour_map_type type of colourmap used
     * @param cm reference to the grid colourmap
     * @param cscale colourmap scale
     */
    void set_colourbar(
      m::Visual<gl_version> & v_ref,
      m::vec<float> & colour_bar_location, m::ColourMapType colour_map_type,
      const m::ColourMap<float> & cm,
      const m::Scale<float, float> & cscale)
    {
      // Add the colour bar
      auto cbv = std::make_unique<m::ColourBarVisual<float, gl_version>>(colour_bar_location);
      v_ref.bindmodel(cbv);
      cbv->cm.setType(colour_map_type);    // ColourBarVisual also has a ColourMapType 'cm' member
      cbv->orientation = m::colourbar_orientation::vertical;
      cbv->tickside = m::colourbar_tickside::right_or_below;
      // Copy colourmap and scale to colourbar visual
      cbv->cm = cm;
      cbv->scale = cscale;

      // double the size of the colourbar
      cbv->width *= 2;
      cbv->length *= 2;

      cbv->twodimensional = false;   // allow to rotate with the grids

      // Now build it
      cbv->finalize();
      v_ref.addVisualModel(cbv);
    }
  
  public:
    /*!
     * @brief Construct a new Simple Grid Visual object
     *
     * @param v_ref reference to the Visual object managing the openGL window
     * @param img_w image width in pixel
     * @param img_h image height in pixel
     * @param grid_title grid title to be written below bottom left
     * @param grid_location location offset relative to the scene center
     * @param grid_colormap colourmap type used to display the grid
     * @param grid_order_in set the grid origin and if rowmajor following the enum GridOrder defined in GridFeatures.h in morphologica
     * @param pixel_size_in side length of the pixel to be rendered
     * @param legend_on add a colourbar as a reference for the magnitude of the grid
     * @param scale_min min of the scale of the colourbar
     * @param scale_max min of the scale of the colourbar
     */
    SimpleGridVisual(
      m::Visual<gl_version> & v_ref,
      unsigned int img_w, unsigned int img_h,
      std::string grid_title, m::vec<float> grid_location,
      m::ColourMapType grid_colormap,
      m::GridOrder grid_order_in = m::GridOrder::topleft_to_bottomright,
      float pixel_size_in = 0.02f,
      bool legend_on = false, float scale_min = 0.0f, float scale_max = 0.0f)
    {
      // create a default image to load initially
      m::vvec<m::vec<float, 3>> default_img = get_default_image(img_w, img_h);

      // get the pixel size of the grid
      this->pixel_size = pixel_size_in;
      // get the grid order
      this->grid_order = grid_order_in;

      // create grid for input
      create_grid_img(img_w, img_h, this->pixel_size);

      // Visualise the input with the provided grid_colormap
      m::vec<float, 4> gd_extents = data_gd->extents();
      m::vec<float, 3> title_location = {
        gd_extents[0] - this->pixel_size / 2 + 0.08f,
        gd_extents[2] - this->pixel_size / 2 - 0.12f,
        0.0f
      };

      this->gv_ptr = create_grid_visualizer(
        v_ref, grid_location, default_img,
        grid_colormap, grid_title, title_location);

      // set colourbar if required
      if (legend_on) {
        // set signal expected min-max for scale
        this->gv_ptr->colourScale.compute_scaling(scale_min, scale_max);
        // create the colourbar
        m::vec<float> bar_location = grid_location + m::vec<float>(
          {pixel_size * img_w + 0.08f, 0.0f, 0.0f}
        );
        if (this->grid_order == m::GridOrder::topleft_to_bottomright) {
          bar_location += m::vec<float>({0.0f, -this->pixel_size * img_h + 0.08f, 0.0f});
        } else {
          bar_location += m::vec<float>({0.0f, 0.05f, 0.0f});
        }
        set_colourbar(
          v_ref,
          bar_location, grid_colormap, this->gv_ptr->cm,
          this->gv_ptr->colourScale);
      }
    }
    /*!
     * @brief update grid function with new data
     *
     * @param new_data vector of vector of data to be displayed in the grid
     */
    void update_grid_data(m::vvec<m::vec<float>> & new_data)
    {
      this->gv_ptr->updateData(&new_data);
    }
  
    /*!
     * @brief Set the Grid around the Selected Pixels with the correct colors
     *
     * @param grid_thickness relative grid thickness to pixel size
     * @param pix_x pixel x location of the top left corner of the image
     * @param pix_y pixel y location of the top left corner of the image
     * @param selected_window_width width of the window in pixels (along the x axis)
     * @param selected_window_height height of the window in pixels (along the y axis)
     * @param image_width image width to know when the line is over
     * @param image_height image height, required if the order is from top
     * @param colors list of colors used for the graph and grid
     */
    void SetGridSelectedPixels(
      float grid_thickness,
      unsigned int pix_x, unsigned int pix_y,
      unsigned int selected_window_width, unsigned int selected_window_height,
      unsigned int image_width, unsigned int image_height,
      std::vector<std::array<float, 3>> & colors)
    {
      // clear previous vector if necessary
      this->gv_ptr->selected_pix_indexes.clear();
      this->gv_ptr->selected_pix_border_colour.clear();

      // add the selected pixels a border with colors
      this->gv_ptr->showselectedpixborder = true;
      this->gv_ptr->selected_pix_indexes.reserve(selected_window_width * selected_window_height);
      this->gv_ptr->grid_thickness = grid_thickness;

      unsigned int i = 0;
      for (unsigned int y = pix_y; y < pix_y + selected_window_height; ++y) {
        for (unsigned int x = pix_x; x < pix_x + selected_window_width; ++x) {
          // set the pixel indexes
          unsigned int pix_idx;
          if (this->grid_order == m::GridOrder::topleft_to_bottomright) {
            pix_idx = x + image_width * (image_height - y - 1);
          } else if (this->grid_order == m::GridOrder::bottomleft_to_topright) {
            pix_idx = x + image_width * y;
          } else {
            std::cerr << "[ERROR in SimpleGridVisual] bottomleft_to_topright_colmaj and topleft_to_bottomright_colmaj grid orders are not supported yet" << std::endl;
          }
          this->gv_ptr->selected_pix_indexes.push_back(pix_idx);
          // set the color for the corresponding pixels
          this->gv_ptr->selected_pix_border_colour.push_back(colors[i]);
          i++;
        }
      }
    }
  };

  template <typename T, typename I = unsigned int, typename C = float, int gl_version = morph::gl::version_4_1>
  class ConstantAbscissaGraphVisual
  {
  public:
    std::unique_ptr<m::GraphVisual<float, gl_version>> gvup;
    m::GraphVisual<float, gl_version> * gvp_graph;

    unsigned int n_curve_;  // number of data to plot

    // data to plot
    std::deque<float> absc_;  // abscissa data for graph to be plotted
    std::vector<std::deque<float>> data_;  // data for graph to be plotted

  public:
    /*!
     * @brief Constructor for a new Graph Visual object
     * with constant abscissa and temporal update
     *
     * @param v_ref reference to the Visual object managing the openGL window
     * @param graph_pos position of the graph in window
     * @param graph_width width of the graph
     * @param graph_height height of the graph
     * @param time_window time horizon to keep the data and plot them
     * @param ylabel_str label for the ordinate axis
     * @param n_curve number of data to plot
     */
    ConstantAbscissaGraphVisual(
      m::Visual<gl_version> & v_ref, m::vec<float> graph_pos, float graph_width, float graph_height,
      float time_window, std::string ylabel_str, unsigned int n_curve,
      std::vector<std::array<float, 3>> & line_colors)
    : n_curve_(n_curve)
    {
      this->gvup = std::make_unique<morph::GraphVisual<float, gl_version>>(graph_pos);
      v_ref.bindmodel(this->gvup);

      // Here, we change the size of the graph and range of the axes
      // set the same size as grid
      this->gvup->setsize(graph_width, graph_height);

      // set the axes ranges
      this->gvup->setlimits(-time_window, 0.0f, -0.1f, 1.1f);
      // y axis size chosen for pixel values which ranges from 0-1, should be updated in the do update

      // setup curve properties
      this->gvup->axiscolour = {0.5, 0.5, 0.5};
      this->gvup->axislinewidth = 0.01f;
      this->gvup->axisstyle = morph::axisstyle::boxfullticks;
      this->gvup->setthickness(0.001f);

      // setup axes properties
      this->gvup->fontsize = 0.1;
      this->gvup->tickstyle = morph::tickstyle::ticksin;
      this->gvup->ylabel = ylabel_str;
      this->gvup->xlabel = "time [s]";

      // set the line style and markers
      morph::DatasetStyle ds;
      ds.linewidth = 0.005;
      ds.markerstyle = morph::markerstyle::circle;
      ds.markersize = 0.01f;
      ds.markergap = 0.0f;

      for (unsigned int i = 0; i < n_curve_; ++i) {
        // update the line style for each curve
        ds.linecolour = line_colors[i];
        ds.markercolour = line_colors[i];

        // We 'prepare' two datasets, but won't fill them with data yet. However, we do give the data legend label here.
        // For each dataset added there should be a set of 'datastyles' - linestyle, markerstyle, etc
        this->gvup->prepdata(ds);
      }

      // Enable auto-rescaling of the x axis
      this->gvup->auto_rescale_x = true;
      // Enable auto-rescaling of the y axis
      this->gvup->auto_rescale_y = true;

      // finalize
      this->gvup->finalize();

      // Add the GraphVisual (as a VisualModel*)
      this->gvp_graph = v_ref.addVisualModel(this->gvup);

      // initialize first data
      this->clean_data_graph(n_curve_, 0.0f);
    }

    /*!
     * @brief clear the graph from any data to start again
     *
     * @param nb_sample_to_display number of curves to display
     * @param init_time time at t0 (for now should be 0.0f)
     */
    void clean_data_graph(const size_t nb_sample_to_display, float init_time)
    {
      // initialize abscissa
      absc_.clear();
      absc_.push_back(init_time);

      // initialize first data for each curve
      std::deque<float> first_data;
      first_data.push_back(0.0f);
      for (unsigned int i = 0; i < nb_sample_to_display; ++i) {
        if (data_.size() > i) {
          data_[i].clear();
          data_[i].push_back(0.0f);
        } else {
          data_.push_back(first_data);
        }
      }
    }

    /*!
     * @brief function to update the graph with new data
     *
     * @param nb_sample_to_display set the number of sample to display. The sample will be displayed from the last
     * @param dt sampling time
     * @param values vector of last values to add at each curve
     */
    void update_graph(const size_t nb_sample_to_display, float dt, std::vector<float> & values)
    {
      if (absc_.size() < (nb_sample_to_display)) {
        absc_.push_front(*(absc_.begin()) - dt);

        for (unsigned int i = 0; i < n_curve_; ++i) {
          data_.at(i).push_back(values[i]);
        }
      } else {
        for (unsigned int i = 0; i < n_curve_; ++i) {
          data_.at(i).pop_front();
          data_.at(i).push_back(values[i]);
        }
      }

      // update the curve in graph
      for (unsigned int i = 0; i < n_curve_; ++i) {
        this->gvp_graph->update(absc_, data_[i], i);
      }
    }
  };

}