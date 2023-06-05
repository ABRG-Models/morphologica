// An interface for neurons. Contains data and algorithm for each neuron.
namespace morph {
    namespace bnn {
        struct neuronmodel
        {
            // Attributes? position (for viz)? activation? Have some sort of sum of
            // inputs? interface? What to do on a timestep? Update activations. Will
            // need inputs. array of floats?
            somekindof input;

            void step() {}
        };
    }
}
