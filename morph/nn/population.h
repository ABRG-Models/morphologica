#include <vector>
#include <morph/nn/neuronmodel.h>
namespace morph {
    namespace nn {
        struct population
        {
            //! A model specification in neuronmodel (which will be specialised). Number
            //! of members of neurons gives size of population.
            std::vector<morph::nn::neuronmodel*> neurons;
        };
    }
}
