#include <morph/Grid.h>
#include <iostream>

int main()
{
    int rtn = 0;

    morph::Grid<unsigned int, double> g(10,12, morph::vec<double, 2>({0.2, 0.2}));

    morph::vvec<double> abscissae = g.get_abscissae();
    morph::vvec<double> ordinates = g.get_ordinates();

    std::cout << "abscissae: " << abscissae << std::endl;
    std::cout << "ordinates: " << ordinates << std::endl;

    if (abscissae.size() != g.get_w() || ordinates.size() != g.get_h()) { --rtn; }

    morph::vec<double, 2> nul = { 0.0, 0.0 };
    morph::Grid<unsigned int, double> g2(10,12, morph::vec<double, 2>({0.2, 0.2}), nul,
                                         morph::GridDomainWrap::None,
                                         morph::GridOrder::topleft_to_bottomright);

    abscissae = g2.get_abscissae();
    ordinates = g2.get_ordinates();

    std::cout << "abscissae: " << abscissae << std::endl;
    std::cout << "ordinates: " << ordinates << std::endl;

    if (abscissae.size() != g2.get_w() || ordinates.size() != g2.get_h()) { --rtn; }

    morph::Grid<unsigned int, double> g3(10,12, morph::vec<double, 2>({0.2, 0.2}), nul,
                                         morph::GridDomainWrap::None,
                                         morph::GridOrder::topleft_to_bottomright_colmaj);

    abscissae = g3.get_abscissae();
    ordinates = g3.get_ordinates();

    std::cout << "abscissae: " << abscissae << std::endl;
    std::cout << "ordinates: " << ordinates << std::endl;

    if (abscissae.size() != g3.get_w() || ordinates.size() != g3.get_h()) { --rtn; }

    morph::Grid<unsigned int, double> g4(10,12, morph::vec<double, 2>({0.2, 0.2}), nul,
                                         morph::GridDomainWrap::None,
                                         morph::GridOrder::bottomleft_to_topright_colmaj);

    abscissae = g4.get_abscissae();
    ordinates = g4.get_ordinates();

    std::cout << "abscissae: " << abscissae << std::endl;
    std::cout << "ordinates: " << ordinates << std::endl;

    if (abscissae.size() != g4.get_w() || ordinates.size() != g4.get_h()) { --rtn; }

    return rtn;
}
