#include <morph/histo.h>

int main()
{
    int rtn = 0;

    morph::vvec<int> numbers = { 1, 1, 2, 3, 4, 4, 4 };
    morph::histo<int, float> h(numbers, 3);

    std::cout << "For data: " << numbers << " arranged into three bins:\n\n";

    morph::range<int> _datarange = h.datarange; // Data range in terms of first histo template param type
    std::cout << "data range is: " << _datarange << std::endl;

    std::size_t _datacount = h.datacount; // Counts use the std::size_t type
    std::cout << "data count is: " << _datacount << std::endl;
    if (_datacount != 7u) { --rtn; }

    float _binwidth = h.binwidth;  // proportions, bin edges, bins, bin width are of type float
    std::cout << "bin width is: " << _binwidth << std::endl;

    morph::vvec<float> _bins = h.bins;
    std::cout << "bin centres are: " << _bins << std::endl;

    morph::vvec<float> _binedges = h.binedges;
    std::cout << "bin edges are: " << _binedges << std::endl;

    morph::vvec<std::size_t> _counts = h.counts;
    std::cout << "Counts are: " << _counts << std::endl;

    std::size_t csum = (_counts - morph::vvec<std::size_t>{ 2, 1, 4 }).sum();
    if (csum != 0u) { --rtn; }

    morph::vvec<float> _proportions = h.proportions;
    std::cout << "Proportions are: " << _proportions << std::endl;

    std::cout << "Below 1: " << h.proportion_below (1.0f) << std::endl;
    std::cout << "Below 2: " << h.proportion_below (2.0f) << std::endl;
    std::cout << "Below 2.5: " << h.proportion_below (2.5f) << std::endl;
    std::cout << "Below 3: " << h.proportion_below (3.0f) << std::endl;
    std::cout << "Below 4: " << h.proportion_below (4.0f) << std::endl;
    std::cout << "Below 5: " << h.proportion_below (5.0f) << std::endl;

    std::cout << "Above 5: " << h.proportion_above (5.0f) << std::endl;
    std::cout << "Above 4: " << h.proportion_above (4.0f) << std::endl;
    std::cout << "Above 3: " << h.proportion_above (3.0f) << std::endl;
    std::cout << "Above 2.5: " << h.proportion_above (2.5f) << std::endl;
    std::cout << "Above 2: " << h.proportion_above (2.0f) << std::endl;
    std::cout << "Above 0: " << h.proportion_above (0.0f) << std::endl;
    std::cout << "Above -1000: " << h.proportion_above (-1000.0f) << std::endl;

    if (std::abs(h.proportion_below(2.0f) - (2.0f/7.0f)) > std::numeric_limits<float>::epsilon()) { --rtn; }
    if (std::abs(h.proportion_below(3.0f) - (3.0f/7.0f)) > std::numeric_limits<float>::epsilon()) { --rtn; }
    if (std::abs(h.proportion_below(4.0f) - (7.0f/7.0f)) > std::numeric_limits<float>::epsilon()) { --rtn; }
    if (std::abs(h.proportion_below(3.5f) - (5.0f/7.0f)) > std::numeric_limits<float>::epsilon()) { --rtn; }

    if (std::abs(h.proportion_above(3.5f) - (2.0f/7.0f)) > std::numeric_limits<float>::epsilon()) { --rtn; }
    std::cout << "Above 3.5: " << h.proportion_above (3.5f) << " delta: "  << (h.proportion_above(3.5f) - (2.0f/7.0f)) << std::endl;

    if (rtn == 0) { std::cout << "Test SUCCESS\n"; } else { std::cout << "Test FAIL\n"; }

    return rtn;
}
