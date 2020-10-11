#include "morph/Visual.h"
#ifdef __OSX__
# include <OpenGL/gl3.h>
#else
# include "GL3/gl3.h"
#endif
#include <iostream>
using std::cout;
using std::cerr;
using std::endl;
#include <string>
using std::string;
#include <vector>
using std::vector;
#include <array>
using std::array;
#include <cstring>
using std::strlen;
#include "morph/Quaternion.h"
using morph::Quaternion;
#include "morph/tools.h"
using morph::ShaderInfo;
// Include the character constants containing the default shaders
#include "morph/VisualDefaultShaders.h"
#include "morph/VisualModel.h"
using morph::VisualModel;
// imwrite() from OpenCV is used in saveImage()
#include <opencv2/imgcodecs.hpp>
