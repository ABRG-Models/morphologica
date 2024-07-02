#include <morph/loadpng.h>

int main()
{
    int rtn = 0;

    // Load an image
    std::string fn = "../../examples/bike256_65.png";

    morph::vvec<float> image_data;
    try {
        morph::vec<unsigned int, 2> dims = morph::loadpng (fn, image_data);
        std::cout << "Image dims: " << dims << std::endl;
    } catch (const std::exception& e) {
        // Unexpected error
        std::cerr << "Failed to loadpng\n";
        --rtn;
    }

    fn = "examples/bad_name.png"; // known bad
    try {
        morph::vec<unsigned int, 2> dims = morph::loadpng (fn, image_data);
        std::cout << "Image dims: " << dims << std::endl;
        // This should have failed
        --rtn;
    } catch (const std::exception& e) {
        // Expected error
        std::cout << "Error: " << e.what() << std::endl;
    }

    std::cout << "return rtn = "  << rtn << std::endl;
    return rtn;
}
