#pragma once

/*
 * This is just an enumerated class that defines the fonts we pack into a morph binary
 */

namespace morph {
    //! The fonts supported (i.e. compiled in) to morph::Visual
    enum class VisualFont {
        DVSans,             // fonts/dejavu/DejaVuSans.ttf
        DVSansItalic,       // fonts/dejavu/DejaVuSans-Oblique.ttf
        DVSansBold,         // fonts/dejavu/DejaVuSans-Bold.ttf
        DVSansBoldItalic,   // fonts/dejavu/DejaVuSans-BoldOblique.ttf
        Vera,               // fonts/ttf-bitstream-vera/Vera.ttf
        VeraItalic,         // fonts/ttf-bitstream-vera/VeraIt.ttf
        VeraBold,           // fonts/ttf-bitstream-vera/VeraBd.ttf
        VeraBoldItalic,     // fonts/ttf-bitstream-vera/VeraBI.ttf
        VeraMono,           // fonts/ttf-bitstream-vera/VeraMono.ttf
        VeraMonoItalic,     // fonts/ttf-bitstream-vera/VeraMoIt.ttf
        VeraMonoBold,       // fonts/ttf-bitstream-vera/VeraMoBD.ttf
        VeraMonoBoldItalic, // fonts/ttf-bitstream-vera/VeraMoBI.ttf
        VeraSerif,          // fonts/ttf-bitstream-vera/VeraSe.ttf
        VeraSerifBold       // fonts/ttf-bitstream-vera/VeraSeBd.ttf
    };
} // morph
