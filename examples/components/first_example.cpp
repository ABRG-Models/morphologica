/**
 * \file first_example.cpp
 * Process image from example movie data and use the graph visualization from the lib.
 * It runs at 30 FPS (original movie pace)
 *
 * \author Fabien Colonnier
 * \date November 2024
 */

#include <string>
#include <sstream>
#include <iostream>
#include <memory>
#include <chrono>
#include <unistd.h>

#include <morph/lib_components/components.h>
#include "load_example_img.hpp"

// Shorten the morph namespace
namespace m = morph;

constexpr int gl_version = m::gl::version_4_6;
const float pixel_size = 0.02f;  // size of the pixel in the Visual renderer
const unsigned int selected_window_size[2] = {5, 1};  // selected window size for the temporal window

// code to be moved to colormap.h
const float cm_lines[][3] = {
  {0.0, 0.447058823529412, 0.741176470588235},
  {0.850980392156863, 0.325490196078431, 0.0980392156862745},
  {0.466666666666667, 0.674509803921569, 0.188235294117647},
  {0.494117647058824, 0.184313725490196, 0.556862745098039},
  {0.929411764705882, 0.694117647058823, 0.125490196078431},
  {0.301960784313725, 0.745098039215686, 0.933333333333333},
  {0.635294117647059, 0.0784313725490196, 0.184313725490196}};

const std::size_t cm_lines_len = sizeof(cm_lines) / (sizeof(float) * 3);

template<typename T>
std::array<float, 3> GetLinesColor(T _datum)
{
  unsigned int index = static_cast<unsigned int>((std::abs(std::round(_datum % cm_lines_len))));
  std::array<float, 3> c = {cm_lines[index][0], cm_lines[index][1], cm_lines[index][2]};
  return c;
}
// end of code to be moved to colourmap.h

struct VisualizerTemporalSignal
{

  unsigned int input_img_width;  // initial image width in pixels
  unsigned int input_img_height;  // initial image height in pixels

  // object managing the scene in the openGL window
  m::Visual<gl_version> v;

  // pointers to the grids
  std::unique_ptr<m::SimpleGridVisual<float, int, float, gl_version>> input_sgv_ptr;
  std::unique_ptr<m::SimpleGridVisual<float, int, float, gl_version>> in_selected_sgv_ptr;

  // pointers to the graphs
  std::unique_ptr<m::ConstantAbscissaGraphVisual<float, int, float, gl_version>> pixel_graph;

  // internal data
  float sampling_time;  // saving sampling time as internal variable
  const size_t nb_sample;  // number of samples for the moving window

  std::vector<std::array<float, 3>> colors_for_line;  // list of colors used for the graph and grid
  std::array<unsigned int, 2> old_pix_position = {0, 0};  // store the location from previous call

  /**
   * @brief Constructor for VisualTemporalSignal object, which display temporal signal of the state variables
   *
   * @param img_w image width
   * @param img_h image height
   * @param window_name title of the window to be created
   * @param origin_top_left boolean to set the origin of the frame to top left, bottom right if false. The grid will be row major in both cases
   * @param time_window time horizon to keep the data
   * @param sampling_time_in sampling time, time between each input
   */
  VisualizerTemporalSignal(
    unsigned int img_w, unsigned int img_h,
    const std::string & window_name, bool origin_top_left,
        float time_window, float sampling_time_in)
    : input_img_width(img_w), input_img_height(img_h),
      v(1920, 1800, window_name),  // Visual v(width, height, title)
      sampling_time(sampling_time_in),
      nb_sample(std::ceil(time_window / sampling_time))
    {
      std::cout << "[VisualTemporalSignal::constructor] starts:" << std::endl;
      std::cout << "\t sampling time = " << sampling_time << "s\n";
      std::cout << "\t nb_sample to display in graph = " << nb_sample << "\n";

      // Position of the scene, used Ctrl-z when the window is active to get the current location
      this->v.setSceneTrans(m::vec<float, 3>({-1.38292f, 0.829382f, -25.5f}));

      // save sets of colors for the graph lines
      for (unsigned int i = 0; i < selected_window_size[0] * selected_window_size[1]; ++i) {
        colors_for_line.emplace_back(GetLinesColor<unsigned int>(i));
      }

      // vectors for scene offsets for each column
      m::vec<float> offset_left = {-1.0f * (pixel_size * img_w + 0.58f), 2.0f, 0.0f};
      m::vec<float> offset_right = {0.0f, 2.0f, 0.0f};

      // grid order setting
      m::GridOrder grid_order_setting;
      if (origin_top_left) {
        grid_order_setting = m::GridOrder::topleft_to_bottomright;
        offset_left += m::vec<float>{0.0f, pixel_size * img_h, 0.0f};
        //offset_right += m::vec<float>{0.0f, pixel_size * img_h, 0.0f};
      } else {
        grid_order_setting = m::GridOrder::bottomleft_to_topright;
      }

      /* initial view */
      // create grid for input
      input_sgv_ptr = std::make_unique<m::SimpleGridVisual<float, int, float, gl_version>>(
        this->v, img_w, img_h,
        "0: Input", offset_left, m::ColourMapType::RGB, grid_order_setting
      );

      /* selected pixels display */
      // create grid for input of selected pixels
      in_selected_sgv_ptr = std::make_unique<m::SimpleGridVisual<float, int, float, gl_version>>(
        this->v, selected_window_size[0], selected_window_size[1],
        "0b: selected pixels to display", offset_right, m::ColourMapType::RGB, grid_order_setting, 0.02f * img_h/2
      );

      /* create graph for original input data */
      // update offset
      if (origin_top_left) {
        offset_left += m::vec<float>({0.0f, -2 * pixel_size * img_h - 0.4f, 0.0f});
      } else {
        offset_left += m::vec<float>({0.0f, -pixel_size * img_h - 0.4f, 0.0f});
      }
      pixel_graph = std::make_unique<m::ConstantAbscissaGraphVisual<float, int, float, gl_version>>(
        this->v, offset_left, img_w * pixel_size, img_h * pixel_size, time_window,
        "grayscale pixel value [a.u.]", selected_window_size[0] * selected_window_size[1], colors_for_line
      );

      std::cout << "[VisualizerTemporalSignal::constructor] ends" << std::endl;
    }

  /**
   * @brief update the visualization with the computed data to visualize the new frame
   *
   * @param data input data converted to morphologica vectors
   * @param pix_x x position of the pixel at center of the ROI
   * @param pix_y y position of the pixel at selected pixel
   */
  void do_update(m::vvec<m::vec<float, 3>> & data, unsigned int pix_x, unsigned int pix_y)
  {
    this->v.setContext();

    /* catch if selected pixel changed */
    bool changed_pixel = false;
    if (this->old_pix_position[0] != pix_x) {
      changed_pixel = true;
      this->old_pix_position[0] = pix_x;
    }
    if (this->old_pix_position[1] != pix_y) {
      changed_pixel = true;
      this->old_pix_position[1] = pix_y;
    }

    // initialize variables related to window size
    const unsigned int nb_selected_pixels = selected_window_size[0] * selected_window_size[1];
    const unsigned int half_window_w = (selected_window_size[0] / 2);
    const unsigned int half_window_h = (selected_window_size[1] / 2);

    if (changed_pixel) {
      std::cout << "[VisualTemporalSignal::do_update] clear graphs" << std::endl;
      pixel_graph->clean_data_graph(nb_selected_pixels, 0.0f);
    }

    /* update first grid initial image */
    this->input_sgv_ptr->update_grid_data(data);
    if (changed_pixel == true) {
      this->input_sgv_ptr->SetGridSelectedPixels(
        1.0f, pix_x-half_window_w, pix_y-half_window_h,
        selected_window_size[0], selected_window_size[1],
        this->input_img_width, this->input_img_height,
        colors_for_line);
    }

    /* update second grid with selected pixels */
    //get specific pixels
    m::vvec<m::vec<float, 3>> morph_selected_pix;
    m::vec<float, 3> nullvec = {0.0f, 0.0f, 0.0f};
    morph_selected_pix.resize(nb_selected_pixels, nullvec);

    unsigned int i = 0;
    for (unsigned int y = pix_y - half_window_h; y <= pix_y + half_window_h; ++y) {
      for (unsigned int x = pix_x - half_window_w; x <= pix_x + half_window_w; ++x) {
        morph_selected_pix[i][0] = data[y * this->input_img_width + x][0];
        morph_selected_pix[i][1] = data[y * this->input_img_width + x][1];
        morph_selected_pix[i][2] = data[y * this->input_img_width + x][2];
        i++;
      }
    }
    // update the grid
    this->in_selected_sgv_ptr->update_grid_data(morph_selected_pix);

    // get the grid around the pixel to show which are selected
    // The selection is done from 
    if (changed_pixel == true) {
      this->in_selected_sgv_ptr->SetGridSelectedPixels(
        0.04f, 0, 0,
        selected_window_size[0], selected_window_size[1],
        selected_window_size[0], selected_window_size[1],
        colors_for_line);
    }

    /* get the vector values to update the graphs */
    std::vector<float> selected_pix_val;
    selected_pix_val.reserve(nb_selected_pixels);

    for (unsigned int y = pix_y - half_window_h; y <= pix_y + half_window_h; ++y) {
      for (unsigned int x = pix_x - half_window_w; x <= pix_x + half_window_w; ++x) {
        // signal for graph equals 
        float pix_mean = data[y * this->input_img_width + x].mean();
        selected_pix_val.push_back(pix_mean);
      }
    }

    /* update the graph */
    pixel_graph->update_graph(nb_sample, sampling_time, selected_pix_val);
    
    // render the image
    this->v.render();

    // Explicitly release context of the v Visual object, before calling setContext for any other object.
    this->v.releaseContext();
  }

  /**
   * @brief Destroy the VisualTemporalSignal object and check if any OpenGL error is triggered
   *
   */
  ~VisualizerTemporalSignal()
  {
    std::cout << "destructor VisualizerTemporalSignal \n";
    morph::gl::Util::checkError(__FILE__, __LINE__);
  }
};

/**
 * @brief  load images and uses them with the default visualizer
 *
 * @return int an integer 0 upon exit success
 */
int main()
{
  /*
   * Load Data
   */
  std::vector<m::vvec<m::vec<float, 3>>> img_input = morph_example::load_imgs();

  float sampling_time = 0.05;  // sampling time

  /*
   * Set up visualization objects
   */

  VisualizerTemporalSignal visualizer(
    morph_example::img_w,
    morph_example::img_h,
    "WindowTitle: temporal visualization", false,
    10, sampling_time);

  /*
   * Loop and refresh visualization
   */
  while (!visualizer.v.readyToFinish) {
    for (int idx = 0; idx < morph_example::num_pngs && !visualizer.v.readyToFinish; ++idx) {
      // Update camera view
      unsigned int pix_position[2] = {125, 50};
      visualizer.do_update(img_input[idx], pix_position[0], pix_position[1]);

      // trigger the window update
      visualizer.v.poll();

      // wait 30ms before computing and display next frame (optional)
      visualizer.v.wait(sampling_time);
    }
  }

  return 0;
}