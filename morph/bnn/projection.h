#pramga once

#include <vector>
#include <morph/bnn/projection.h>
#include <morph/bnn/postsynapse.h>

namespace morph {
    namespace bnn {
        template <typename T>
        struct projection
        {
            population* from;
            population* to;
            std::vector<T> w; // Simple, multiplicative weights.
            postsynapse* ps; // postsynapse component (contains maths)
        };
    }
}
