#include <vector>
#include <morph/bnn/neuronmodel.h>
namespace morph {
    namespace bnn {
        struct population
        {
            //! A model specification in neuronmodel (which will be specialised). Number
            //! of members of neurons gives size of population.
            std::vector<morph::bnn::neuronmodel*> neurons;
        };
    }
}
