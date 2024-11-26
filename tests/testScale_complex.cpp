/*
 * Test use of std::complex<> objects in a morph::scale class.
 */

#include <iostream>
#include <complex>
#include <array>
#include <morph/vvec.h>
#include <morph/scale.h>

int main()
{
    int rtn = 0;

    morph::scale<std::complex<float>> sc;
    sc.do_autoscale = true;

    morph::vvec<std::complex<float>> vc = { {1, 0}, {0, 1}, {-1, 0}, {0, -1}, {2, 0}, {0, 2}, {-2, 0}, {0, -2},  };
    morph::vvec<std::complex<float>> vcs (vc);
    sc.transform (vc, vcs);

    std::cout << "After autoscaling we have this morph::scale:\n" << sc << std::endl;
    std::cout << "vvec<complex> unscaled: " << vc << "\n";
    std::cout << "vvec<complex> scaled:   " << vcs << "\n";

    if (vcs[0] == std::complex<float>{0.5f, 0.0f} && vcs[5] == std::complex<float>{0.0f, 1.0f}) {
        // good
    } else { --rtn; }

    sc.reset();
    std::cout << "imaginary output range max...\n";
    sc.output_range = { {0, 0}, {0, -1} }; // for complex, range min should always be (0 + 0i). The magnitude of the top of the range is used.
    morph::vvec<std::complex<float>> vc1 = { {1, 0}, {0, 1}, {-1, 0}, {0, -1}, {2, 0}, {0, 2}, {-2, 0}, {0, -2},  };
    morph::vvec<std::complex<float>> vcs1 (vc1);
    sc.transform (vc1, vcs1);

    std::cout << "vvec<complex> unscaled: " << vc1 << "\n";
    std::cout << "vvec<complex> scaled:   " << vcs1 << "\n";

    if (vcs1[0] == std::complex<float>{0.5f, 0.0f} && vcs1[5] == std::complex<float>{0.0f, 1.0f}) {
        // good
    } else { --rtn; }

    std::cout << "0 to 10 output range...\n";
    sc.reset();
    sc.output_range = { {0, 0}, {10, 0} }; // for complex, range min should always be (0 + 0i). The magnitude of the top of the range is used.
    morph::vvec<std::complex<float>> vc2 = { {1, 0}, {0, 1}, {-1, 0}, {0, -1}, {2, 0}, {0, 2}, {-2, 0}, {0, -2},  };
    morph::vvec<std::complex<float>> vcs2 (vc2);
    sc.transform (vc2, vcs2);

    std::cout << "vvec<complex> unscaled: " << vc2 << "\n";
    std::cout << "vvec<complex> scaled:   " << vcs2 << "\n";

    if (vcs2[0] == std::complex<float>{5.0f, 0.0f} && vcs2[5] == std::complex<float>{0.0f, 10.0f}) {
        // good
    } else { --rtn; }

    try {
        std::cout << "Check that a non-zero output range min causes exception\n";
        sc.reset();
        sc.output_range = { {1, 0}, {2, 0} }; // for complex, range min should always be (0 + 0i)
        morph::vvec<std::complex<float>> vc1 = { {1, 0}, {0, 1}, {-1, 0}, {0, -1}, {2, 0}, {0, 2}, {-2, 0}, {0, -2},  };
        morph::vvec<std::complex<float>> vcs1 (vc1);
        sc.transform (vc1, vcs1);
        --rtn;
    } catch (const std::exception& e) {
        // expected catch
        std::cout << "Expected exception '" << e.what() << "' caught\n";
    }

    // Should not compile:
    // morph::scale<std::complex<std::array<float, 3>>> scale_cplx_of_vec;
    // morph::scale<std::pair<float, double>> scale_a_pair;

    std::cout << "Test " << (rtn == 0 ? "Passed" : "Failed") << std::endl;
    return rtn;
}
