/*
 * An example morph::Visual scene, containing a HexGrid, onto which is sampled an image.
 */

#include <iostream>
#include <vector>
#include <cmath>

#include <morph/Scale.h>
#include <morph/vec.h>
#include <morph/vvec.h>
#include <morph/Visual.h>
#include <morph/VisualDataModel.h>
#include <morph/HexGridVisual.h>
#include <morph/HexGrid.h>

// Here, I used OpenCV to load a png image, convert it to monochrome and extract the data
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>

int main()
{
    morph::Visual v(1600, 1000, "Demo of HexGrid::resampleImage");

    morph::HexGrid hg(0.01f, 3.0f, 0.0f);
    hg.setCircularBoundary (1.2f);

    // Load an image with the help of OpenCV.
    std::string fn = "../examples/bike256.png";
    cv::Mat img = cv::imread (fn.c_str(), cv::IMREAD_GRAYSCALE);
    img.convertTo (img, CV_32F);
    morph::vvec<float> image_data;
    image_data.assign(reinterpret_cast<float*>(img.data),
                      reinterpret_cast<float*>(img.data) + img.total() * img.channels());
    image_data /= 255.0f;
    // This controls how large the photo will be on the HexGrid
    morph::vec<float,2> image_scale = {1.8f, 1.8f};
    // You can shift the photo with an offset if necessary
    morph::vec<float,2> image_offset = {0.0f, 0.0f};

    // Here's the HexGrid method that will resample the square pixel grid onto the hex grid
    morph::vvec<float> hex_image_data = hg.resampleImage (image_data, img.cols, image_scale, image_offset);

    // Now visualise with a HexGridVisual
    morph::HexGridVisual<float>* hgv = new morph::HexGridVisual<float>(v.shaderprog, v.tshaderprog, &hg, {0.0f,0.0f,0.0f });

    // Set the image data as the scalar data for the HexGridVisual
    hgv->setScalarData (&hex_image_data);
    // The inverse greyscale map is appropriate for a monochrome image
    hgv->cm.setType (morph::ColourMapType::GreyscaleInv);
    // As it's an image, we don't want relief, so set the zScale to have a zero gradient
    hgv->zScale.setParams (0, 1);

    hgv->finalize();
    v.addVisualModel (hgv);

    v.keepOpen();

    return 0;
}
