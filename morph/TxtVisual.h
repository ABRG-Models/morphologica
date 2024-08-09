#pragma once

// Add some text as a VisualModel

#include <morph/VisualModel.h>
#include <morph/vec.h>
#include <morph/tools.h>
#include <morph/TextFeatures.h>
#include <string>
#include <iostream>
#include <vector>
#include <array>
#include <stdexcept>

namespace morph {

    template<int glver = morph::gl::version_4_1>
    class TxtVisual : public VisualModel<glver>
    {
    public:
        TxtVisual (const std::string& _text,
                   const morph::vec<float, 3>& _offset,
                   const morph::TextFeatures& _tfeatures)
        {
            // Set up...
            this->mv_offset = _offset;
            this->viewmatrix.translate (this->mv_offset);

            this->text = _text;
            this->tfeatures = _tfeatures;
        }

        void initializeVertices()
        {
            // No op, but add text
            this->addLabel (this->text, morph::vec<float>({0,0,0}), this->tfeatures);
        }

        std::string text;
        morph::TextFeatures tfeatures;
    };

} // namespace morph
