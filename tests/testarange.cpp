#include <morph/vvec.h>

int main()
{
    int rtn = 0;

    morph::vvec<double> x;

    // x = np.arange(0, 6, 0.5)
    morph::vvec<double> x_ex1 = { 0.0 , 0.5, 1.0, 1.5, 2.0, 2.5, 3.0, 3.5, 4.0, 4.5, 5.0, 5.5 };
    x.arange (0.0, 6.0, 0.5);
    std::cout << "First example: " << x << std::endl;
    if (std::abs((x - x_ex1).longest()) >= std::numeric_limits<double>::epsilon()) { --rtn; }

    // x = np.arange(-2.5, 2.3, 2)
    morph::vvec<double> x_ex2 = { -2.5, -0.5,  1.5 };
    x.arange (-2.5, 2.3, 2.0);
    std::cout << "Second example: " << x << std::endl;
    if (std::abs((x - x_ex2).longest()) >= std::numeric_limits<double>::epsilon()) { --rtn; }

    // x = np.arange(-5, 5, 2)
    morph::vvec<double> x_ex3 = { -5.0, -3.0, -1.0,  1.0,  3.0 };
    x.arange (-5.0, 5.0, 2.0);
    std::cout << "Third example: " << x << std::endl;
    if (std::abs((x - x_ex3).longest()) >= std::numeric_limits<double>::epsilon()) { --rtn; }

    // x = np.arange(2, -2, 0.27)
    morph::vvec<double> x_ex4 = {}; // empty
    x.arange (2.0, -2.0, 0.27);
    std::cout << "Fourth example: " << x << std::endl;
    if (!x.empty()) { --rtn; }

    // x = np.arange(-2, 2, -0.27) should be empty
    morph::vvec<double> x_ex5 = {};
    x.arange (-2.0, 2.0, -0.27);
    std::cout << "Fifth example: " << x << std::endl;
    if (!x.empty()) { --rtn; }

    // x = np.arange(1, -1, -0.2) should be empty
    morph::vvec<double> x_ex6 = { 1.0,  0.8,  0.6, 0.4, 0.2,  0.0, -0.2, -0.4, -0.6, -0.8 };
    x.arange (1.0, -1.0, -0.2);
    std::cout << "Sixth example: " << x << std::endl;
    if (std::abs((x - x_ex6).longest()) >= std::numeric_limits<double>::epsilon()) { --rtn; }

    // x = np.arange(-2, 1.2, 0.3)
    morph::vvec<double> x_ex7 = { -2.0 , -1.7, -1.4, -1.1, -0.8, -0.5, -0.2,  0.1,  0.4,  0.7,  1.0 };
    x.arange (-2.0, 1.2, 0.3);
    std::cout << "Seventh example: " << x << std::endl;
    if (std::abs((x - x_ex7).longest()) >= std::numeric_limits<double>::epsilon()) { --rtn; }

    if (rtn == 0) { std::cout << "Success\n"; }
    return rtn;
}
