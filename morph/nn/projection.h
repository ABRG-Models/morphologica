#pramga once

#include <vector>
#include <morph/nn/projection.h>
#include <morph/nn/postsynapse.h>

namespace morph {
    namespace nn {
        template <typename T>
        struct projection
        {
            population* from;
            population* to;
            std::vector<T> w; // multiplicative weights
            postsynapse* ps; // postsynapse component (contains maths)
        };
    }
}
