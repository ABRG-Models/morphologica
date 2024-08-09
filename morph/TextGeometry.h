#pragma once

namespace morph {
    /*!
     * A class containing information about a text string, as it would be displayed on
     * the screen in some font, at a given size. The units should be the same as those
     * used to create the quads on which the text will be laid out.
     */
    struct TextGeometry
    {
        //! The sum of the advances of each glyph in the string
        float total_advance = 0.0f;
        //! The maximum extension above the baseline for any glyph in the string
        float max_bearingy = 0.0f;
        //! The maximum ymin - the extension of the lowest part of any glyph, like gpqy, etc.
        float max_drop = 0.0f;
        //! Return half the width of the string
        float half_width() { return this->total_advance * 0.5f; }
        //! The width is the total_advance.
        float width() { return this->total_advance; }
        //! The effective height is the maximum bearingy
        float height() { return this->max_bearingy; }
        //! Half the max_bearingy
        float half_height() { return this->max_bearingy * 0.5f; }
    };

} // namespace
