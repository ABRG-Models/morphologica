/*!
 * \file
 *
 * Visualise a neural network
 *
 * \author Seb James
 * \date 2022
 */
#pragma once

#ifdef __OSX__
# include <OpenGL/gl3.h>
#else
# include <GL3/gl3.h>
#endif
#include <morph/VisualModel.h>
#include <morph/Vector.h>
#include <morph/Scale.h>
#include <vector>
#include <array>

#include <morph/nn/FeedForwardNet.h>

template <typename Flt>
class NetVisual : public morph::VisualModel
{
public:
    NetVisual(GLuint sp, GLuint tsp, const morph::Vector<float, 3> _offset, morph::nn::FeedForwardNet<Flt>* _thenet)
    {
        this->nn = _thenet;
        this->shaderprog = sp;
        this->tshaderprog = tsp;
        this->mv_offset = _offset;
        this->viewmatrix.translate (this->mv_offset);
    }

    void initializeVertices()
    {
        VBOint idx = 0;

        // For each neuron layer, draw discs, with colour indicating activation.
        // For each connection layer, draw lines, with colours indicating weight?

        // Draw neurons in a pattern; this position is updated as each is drawn.
        morph::Vector<float,3> nloc = {0,0,0}; // nloc: "neuron location"
        std::array<float,3> clr = {1,0,0};

        // Starting locations for each set of neurons, to help draw connection lines
        std::vector<morph::Vector<float,3>> startlocs;

        // Scaling for colour
        morph::Scale<float> s;
        float min_act = this->nn->min_neuron_activation();
        float max_act = this->nn->max_neuron_activation();
        s.compute_autoscale (min_act, max_act);

        // Our colour map
        morph::ColourMap<float> cmf(morph::ColourMapType::Plasma);

        for (auto nlayer : this->nn->neurons) {
            // Each nlayer is vVector<T>
            nloc[1] += this->radiusFixed * 5.0f;
            nloc[0] = 0.0f;
            nloc[0] -= this->radiusFixed * 2.0f * nlayer.size();
            startlocs.push_back (nloc);
            for (auto neuron : nlayer) {
                // Use colour from neuron value.
                clr = cmf.convert (s.transform_one(neuron));

                this->computeTube (idx,
                                   (nloc+this->puckthick)*zoom,
                                   (nloc-this->puckthick)*zoom,
                                   morph::Vector<float,3>({1,0,0}), morph::Vector<float,3>({0,1,0}),
                                   clr, clr,
                                   this->radiusFixed*zoom, 16);
                nloc[0] += this->radiusFixed * 4.0f;
            }

        }

        // Connections. There should be neurons.size()-1 connection layers:
        // std::list<morph::nn::FeedForwardConn<T>> connections;
        // Connection lines from "neuron location" to "neuron location 2" (nloc2)
        float min_weight = -1;
        float max_weight = 1;
        s.compute_autoscale (min_weight, max_weight);
        morph::Vector<float,3> nloc2 = {0,0,0};
        nloc = {0,0,0};
        auto sl = startlocs.begin();
        for (auto cl : this->nn->connections) {

            std::vector<morph::vVector<float>>& ws = cl.ws;
            nloc = *sl;
            nloc2 = *(++sl); --sl;
            for (auto population : ws) { // FeedForwardConn can accept connections from multiple neuron layers.
                // Set output position
                unsigned int counter = 0;
                for (auto w : population) {

                    // Draw connection line from w's input position
                    clr = cmf.convert (s.transform_one(w));

                    this->computeLine (idx, nloc, nloc2, this->uz, clr, clr,
                                       this->linewidth*zoom, this->linewidth/4*zoom);

                    // When reset is needed:
                    size_t M = population.size() / cl.N;
                    if (++counter >= M) {
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

    morph::Vector<float,3> puckthick = { 0, 0, 0.02 };
    //! A normal vector, fixed as pointing up
    morph::Vector<float, 3> uz = {0,0,1};
};
