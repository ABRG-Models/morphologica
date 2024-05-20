/*!
 * \file Visualize the contents of a morph::Config
 */

#pragma once

#include <morph/VisualModel.h>
#include <morph/vec.h>
#include <morph/tools.h>
#include <morph/VisualTextModel.h>
#include <string>
#include <iostream>
#include <vector>
#include <array>
#include <stdexcept>

namespace morph {

    template<int glver = morph::gl::version_4_1>
    class ConfigVisual : public VisualModel<glver>
    {
    public:
        ConfigVisual (const morph::Config* _conf,
                      const std::vector<std::string>& _keys,
                      const morph::vec<float, 3>& _offset,
                      const morph::TextFeatures& _tfeatures)
        {
            // Set up...
            this->mv_offset = _offset;
            this->viewmatrix.translate (this->mv_offset);
            this->conf = _conf;
            this->keys = _keys;
            this->tfeatures = _tfeatures;
        }

        void initializeVertices()
        {
            if (conf == nullptr) { return; }
            if (!conf->ready) { return; }

            morph::vec<float> toffset = {0,0,0};

            // No op, but add text labels for parameters in the config
            for (auto key : this->keys) {
                // For now get value in float format
                float value = conf->get<float>(key, 0.0f);
                std::string lbl = key + std::string(": ") + std::to_string(value);
                morph::TextGeometry geom = this->addLabel (lbl, toffset, this->tfeatures);
                toffset[1] -= line_spacing * geom.height();
            }
        }

        // You must define the things you want to visualize from the Config. You
        // probably don't want *everything* right?
        std::vector<std::string> keys;

        // The Config thing that you'll get text from
        const morph::Config* conf = nullptr;

        // How to format the text
        morph::TextFeatures tfeatures;

        // Spacing between lines of output
        float line_spacing = 1.5f;
    };

} // namespace morph
