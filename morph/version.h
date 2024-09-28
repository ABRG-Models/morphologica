#pragma once

/*!
 * This header defines a simple major/minor version for morphologica.
 *
 * \author Seb James
 * \date April 2024
 */

#include <string>

namespace morph {

    //! A version definition for the whole of morphologica
    static constexpr unsigned int version_major = 3;
    static constexpr unsigned int version_minor = 1;
    //! Returns a string for the version of the morphologica library
    std::string version_string()
    {
        std::string vers = std::to_string (morph::version_major) + std::string(".") + std::to_string (morph::version_minor);
        return vers;
    }

} // namespace morph
