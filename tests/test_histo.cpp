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

    return rtn;
}
