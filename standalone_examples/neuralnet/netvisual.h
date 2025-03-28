/*!
 * \file
 *
 * Visualise a neural network
 *
 * \author Seb James
 * \date 2022
 */
#pragma once

#include <morph/VisualModel.h>
#include <morph/vec.h>
#include <morph/scale.h>
#include <vector>
#include <array>
#include <sstream>
#include <iomanip>

#include <morph/nn/FeedForwardNet.h>

#include <morph/VisualTextModel.h>

// Set false to compile a version that draws spheres
static constexpr bool pucks_for_neurons = true;

template <typename Flt>
class NetVisual : public morph::VisualModel<>
{
public:
    NetVisual(const morph::vec<float, 3> _offset, morph::nn::FeedForwardNet<Flt>* _thenet)
    {
        this->nn = _thenet;
        this->mv_offset = _offset;
        this->viewmatrix.translate (this->mv_offset);
    }

    void initializeVertices()
    {
        // For each neuron layer, draw discs, with colour indicating activation.
        // For each connection layer, draw lines, with colours indicating weight?

        // Draw neurons in a pattern; this position is updated as each is drawn.
        morph::vec<float,3> nloc = {0,0,0}; // nloc: "neuron location"
        std::array<float,3> clr = {1,0,0};

        // Starting locations for each set of neurons, to help draw connection lines
        std::vector<morph::vec<float,3>> startlocs;

        // Scaling for colour
        morph::scale<float> s;
        float min_act = this->nn->min_neuron_activation();
        float max_act = this->nn->max_neuron_activation();
        s.compute_scaling (min_act, max_act);

        // Our colour map
        morph::ColourMap<float> cm(morph::ColourMapType::Plasma);

        float em = 0.04f; // text size
        morph::vec<float, 3> toffset = {this->radiusFixed + 0.2f*em, 0, 0};

        for (auto nlayer : this->nn->neurons) {
            // Each nlayer is vvec<T>
            nloc[1] += this->radiusFixed * 5.0f;
            nloc[0] = 0.0f;
            nloc[0] -= this->radiusFixed * 2.0f * nlayer.size();
            startlocs.push_back (nloc);
            for (auto neuron : nlayer) {
                // Use colour from neuron value.
                //float nval = s.transform_one(neuron);
                std::stringstream ss;
                ss << std::setprecision(3) << neuron;
                clr = cm.convert (s.transform_one(neuron));
                if constexpr (pucks_for_neurons) {
                    this->computeTube ((nloc+this->puckthick)*zoom,
                                       (nloc-this->puckthick)*zoom,
                                       morph::vec<float,3>({1,0,0}), morph::vec<float,3>({0,1,0}),
                                       clr, clr,
                                       this->radiusFixed*zoom, 16);
                } else {
                    this->computeSphere (nloc*zoom, clr, this->radiusFixed*zoom, 16, 20);
                }

                // Text label for activation
                {
                    auto vtm = std::make_unique<morph::VisualTextModel<>> (morph::TextFeatures(em));
                    this->bindmodel (vtm);
                    vtm->setupText (ss.str(), nloc+toffset);
                    this->texts.push_back (std::move(vtm));
                }
                nloc[0] += this->radiusFixed * 4.0f;
            }

        }

        // text offsets
        morph::vec<float, 3> toffset1 = {em, -em, 0};
        morph::vec<float, 3> toffset2 = {em, em, 0};
        morph::vec<float, 3> toffsetbias = {0.9f*this->radiusFixed, -0.77f*this->radiusFixed, 0};

        // Connections. There should be neurons.size()-1 connection layers.
        // Connection lines from "neuron location" to "neuron location 2" (nloc2)
        float min_weight = -1;
        float max_weight = 1;
        s.compute_scaling (min_weight, max_weight);
        morph::vec<float,3> nloc2 = {0,0,0};
        nloc = {0,0,0};
        auto sl = startlocs.begin();
        for (auto cl : this->nn->connections) {

            std::vector<morph::vvec<float>>& ws = cl.ws;
            nloc = *sl;
            nloc2 = *(++sl); --sl;

            for (auto population : ws) { // FeedForwardConn can accept connections from multiple neuron layers.

                // Index into the biases (a vvec of size N in each connection layer, cl)
                unsigned int bidx = 0;

                // Set output position
                unsigned int counter = 0;
                for (auto w : population) {

                    // Draw connection line from w's input position
                    clr = cm.convert (s.transform_one(w));

                    this->computeLine (nloc, nloc2, this->uz, clr, clr, this->linewidth*zoom, this->linewidth/4*zoom);

                    std::stringstream ss;
                    ss << std::setprecision(3) << w;
                    morph::vec<float,3> nloccross = nloc.cross (nloc2);
                    toffset = (nloccross[2] > 0) ? toffset1 : toffset2;
                    {
                        auto vtm = std::make_unique<morph::VisualTextModel<>> (morph::TextFeatures(em));
                        this->bindmodel (vtm);
                        vtm->setupText (ss.str(), ((nloc+nloc2)/2.0f) + toffset);
                        this->texts.push_back (std::move(vtm));
                    }
                    // When reset is needed:
                    size_t M = population.size() / cl.N;
                    if (++counter >= M) {
                        // Draw bias text
                        std::stringstream bb1;
                        bb1 << "bias " << std::setprecision(3) << cl.b[bidx++];
                        {
                            auto vtm = std::make_unique<morph::VisualTextModel<>> (morph::TextFeatures(em/2));
                            this->bindmodel (vtm);
                            vtm->setupText (bb1.str(), (nloc2+toffsetbias));
                            this->texts.push_back (std::move(vtm));
                        }
                        nloc = *sl; // reset nloc
                        nloc2[0] += this->radiusFixed * 4.0f; // increment nloc2
                        counter = 0;
                    } else {
                        nloc[0] += this->radiusFixed * 4.0f;
                        // nloc2 unchanged
                    }

                }
            }
            sl++;
        }
    }

    //! Pointer to a vector of locations to visualise
    morph::nn::FeedForwardNet<Flt>* nn = nullptr;
    Flt radiusFixed = 0.1;
    Flt linewidth = 0.02;
    //! Zoom the size of the netvisual
    float zoom = float{1};

    morph::vec<float,3> puckthick = { 0, 0, 0.02 };
    //! A normal vector, fixed as pointing up
    morph::vec<float, 3> uz = {0,0,1};
};
