/*
 * This is a slightly modified version of William Lenthe's code, used with thanks. I
 * made it into a single header by incorporating the minimal amount of code from the
 * colorspace.hpp header into this file.
 *
 * All the code is wrapped in a lenthe namespace so that it is clear when I use it in
 * morph::ColourMap.
 */

/*************************************************************************************/
/*                                                                                   */
/* Copyright (c) 2018, De Graef Group, Carnegie Mellon University                    */
/* Author: William Lenthe                                                            */
/* All rights reserved.                                                              */
/*                                                                                   */
/* Redistribution and use in source and binary forms, with or without                */
/* modification, are permitted provided that the following conditions are met:       */
/*                                                                                   */
/*     - Redistributions of source code must retain the above copyright notice, this */
/*       list of conditions and the following disclaimer.                            */
/*     - Redistributions in binary form must reproduce the above copyright notice,   */
/*       this list of conditions and the following disclaimer in the documentation   */
/*       and/or other materials provided with the distribution.                      */
/*     - Neither the copyright holder nor the names of its                           */
/*       contributors may be used to endorse or promote products derived from        */
/*       this software without specific prior written permission.                    */
/*                                                                                   */
/* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"       */
/* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE         */
/* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE    */
/* DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE      */
/* FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL        */
/* DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR        */
/* SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER        */
/* CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,     */
/* OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE         */
/* USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.          */
/*                                                                                   */
/*************************************************************************************/
#pragma once

#include <array>
#include <algorithm>
#include <type_traits>
#include <stdexcept>
#include <functional>
#include <numeric>
#include <cmath>
#include <cstdint>
#include <vector>

namespace lenthe {
    // This is the minimum code subset extracted (by Seb) from colourspace.hpp to make the maps work:
    namespace color {

	template <typename T> bool xyz2rgb(T const * const xyz, T * const rgb                      );
        template <typename T> bool luv2rgb(T const * const luv, T * const rgb, T const * ill = NULL);
	template <typename T> void luv2xyz(T const * const luv, T * const xyz, T const * ill = NULL);

        namespace detail {

            //@brief    : invert a 3x3 matrix analytically
            //@param mat: matrix to invert in row major order
            template <typename T> std::array<T, 9> inv3x3(const std::array<T, 9>& mat)
            {
                const T det = mat[3*0+0] * mat[3*1+1] * mat[3*2+2] + mat[3*0+1] * mat[3*1+2] * mat[3*2+0] + mat[3*0+2] * mat[3*1+0] * mat[3*2+1]
                -(mat[3*0+0] * mat[3*1+2] * mat[3*2+1] + mat[3*0+1] * mat[3*1+0] * mat[3*2+2] + mat[3*0+2] * mat[3*1+1] * mat[3*2+0]);
                return std::array<T, 9> {
                    (mat[3*1+1]*mat[3*2+2] - mat[3*1+2]*mat[3*2+1]) / det, (mat[3*0+2]*mat[3*2+1] - mat[3*0+1]*mat[3*2+2]) / det, (mat[3*0+1]*mat[3*1+2] - mat[3*0+2]*mat[3*1+1]) / det,
                    (mat[3*1+2]*mat[3*2+0] - mat[3*1+0]*mat[3*2+2]) / det, (mat[3*0+0]*mat[3*2+2] - mat[3*0+2]*mat[3*2+0]) / det, (mat[3*0+2]*mat[3*1+0] - mat[3*0+0]*mat[3*1+2]) / det,
                    (mat[3*1+0]*mat[3*2+1] - mat[3*1+1]*mat[3*2+0]) / det, (mat[3*0+1]*mat[3*2+0] - mat[3*0+0]*mat[3*2+1]) / det, (mat[3*0+0]*mat[3*1+1] - mat[3*0+1]*mat[3*1+0]) / det
                };
            }

            //@brief    : compute a matrix to convert from XYZ --> rgb
            //@param rgb: chromaticity of {red point, green point, blue point} as XYZ
            //@param w  : chromaticity of white point as XYZ
            template <typename T>
            std::array<T, 9> rgbMat(const T rgb[3][3], T const w[3])
            {
                //build and invert 3x3 matrices to solve for rows of conversion matrix
                //matrix * (r, g, b) = {x, 0, 0}, {0, x, 0}, {0, x, 0} and matrix^-1 * {1,1,1} = w
                const T W[3] = {w[0] / w[1], T(1), w[2] / w[1]};
                const std::array<T, 9> invR = inv3x3<T>({  W[0]   ,   W[1]   ,   W[2]   , rgb[1][0], rgb[1][1], rgb[1][2], rgb[2][0], rgb[2][1], rgb[2][2]});
                const std::array<T, 9> invG = inv3x3<T>({rgb[0][0], rgb[0][1], rgb[0][2],   W[0]   ,   W[1]   ,   W[2]   , rgb[2][0], rgb[2][1], rgb[2][2]});
                const std::array<T, 9> invB = inv3x3<T>({rgb[0][0], rgb[0][1], rgb[0][2], rgb[1][0], rgb[1][1], rgb[1][2],   W[0]   ,   W[1]   ,   W[2]   });
                return {invR[3*0+0], invR[3*1+0], invR[3*2+0], invG[3*0+1], invG[3*1+1], invG[3*2+1], invB[3*0+2], invB[3*1+2], invB[3*2+2]};//assemble matrix
            }

            //constants for standard illuminants and common rgb gamuts
            template <typename T>
            struct Standards {
                //standard illuminants as xyz (normalized XYZ)
                static constexpr T D65_2[3] = { T{0.31271}, T{0.32902}, T{0.35827} }; // D65 for 2 degree observer
                //RGB chromaticities as xyz (normalized XYZ)
                static constexpr T sRGB    [3][3] = { T{0.6400}, T{0.3300}, T{0.0300},
                                                      T{0.3000}, T{0.6000}, T{0.1000},
                                                      T{0.1500}, T{0.0600}, T{0.7900} }; //standard RGB (custom gamma)
                //sRGB custom gamma constants
                static constexpr T sA = T{0.055};   //deviation of gamma correction coefficient from 1
                static constexpr T sGamma = T{2.4};//gamma exponent
                static constexpr T sPhi = T{12.92};  //scale factor for linear gamma region
                static constexpr T sK0 = T{0.04045};   //cutoff for linear gamma correction in inverse direction
                //sRGB <--> XYZ conversion matrices
                static const std::array<T, 9> sRGBmat   ;//matrix to convert from XYZ  --> sRGB
                static const std::array<T, 9> sRGBmatInv;//matrix to convert from sRGB --> XYZ        };
            };

            //sRGB conversion matrices
            template <typename T> const std::array<T, 9> Standards<T>::sRGBmat    = detail::rgbMat(Standards<T>::sRGB, Standards<T>::D65_2);
            template <typename T> const std::array<T, 9> Standards<T>::sRGBmatInv = detail::inv3x3(Standards<T>::sRGBmat);
        } // namespace detail

        //@brief    : convert from XYZ to sRGB
        //@param xyz: XYZ (X, Y, Z) values to convert
        //@param rgb: location to write sRGB (red, green, blue) values
        //@return   : true/false if xyz falls outside/inside the sRGB color gamut
        template <typename T> bool xyz2rgb(T const * const xyz, T * const rgb) {
            using namespace detail;
            static const T gammaInv = T(1) / Standards<T>::sGamma;
            static const T k0Inv    = Standards<T>::sK0 / Standards<T>::sPhi;
            static const T a1       = T(1) + Standards<T>::sA;
            T work[3];
            for(size_t i = 0; i < 3; i++) work[i] = std::inner_product(xyz, xyz+3, Standards<T>::sRGBmat.begin() + 3*i, T(0));//XYZ -> linear rgb
            for(size_t i = 0; i < 3; i++) rgb[i] = work[i] <= k0Inv ? work[i] * Standards<T>::sPhi : a1 * std::pow(work[i], gammaInv) - Standards<T>::sA;//gamma correction

            //check if this value is outside the sRGB color gamut
            bool clamped = false;
            for(size_t i = 0; i < 3; i++) {
                if(std::signbit(rgb[i])) {
                    rgb[i] = T(0);
                    clamped = true;
                } else if(rgb[i] > T(1)) {
                    rgb[i] = T(1);
                    clamped = true;
                }
            }
            return clamped;
        }

        //@brief    : convert from Luv to xyz
        //@param luv: Luv (L*, u*, v*) values to convert
        //@param xyz: location to write XYZ (X, Y, Z) values
        //@param ill: Lab illuminant as XYZ (or NULL to use illuminant D65 for a 2 degree observer)
        template <typename T> void luv2xyz(T const * const luv, T * const xyz, T const * ill)
        {
            //handle L* = 0
            if(luv[0] == T(0)) {
                std::fill(xyz, xyz+3, T(0));
                return;
            }

            //compute u' and v'
            T const * const illum = (NULL != ill) ? ill : detail::Standards<T>::D65_2;
            const T denn = (illum[0] + illum[1] * 15 + illum[2] * 3) / illum[1];
            const T up = (luv[1] / 13 + luv[0] * (illum[0] / illum[1]) * 4 / denn) * 3;
            const T vp = (luv[2] / 13 + luv[0]                         * 9 / denn) * 4;
            const T Lp = (luv[0] + 16) / 116;

            //compute X, Y, and Z
            static const T d = T(27) / 24389;//(3/29)^3
            xyz[1] = luv[0] <= 8 ? luv[0] * d : Lp * Lp * Lp;
            xyz[2] = xyz[1] * (T(12) * luv[0] - up - vp * 5) / vp;
            xyz[0] = xyz[1] * (up * 3) / vp;
        }

        template <typename T> bool luv2rgb(T const * const luv, T * const rgb, T const * ill)
        {
            luv2xyz(luv, rgb, ill);
            return xyz2rgb(rgb, rgb);
        }//luv->xyz->rgb
    } // namespace color

    //@brief: perceptually uniform color maps for ramps, cycles, disks, spheres and balls
    //@reference: Kovesi, Peter. "Good colour maps: How to design them." arXiv preprint arXiv:1509.03700 (2015). [ramp and cycle color maps]
    //@reference: Lenthe [disk, sphere, and ball color maps]

    namespace colormap {

        ////////////////////////////////////////////////////////////////
        //                   Predefined Color Maps                    //
        ////////////////////////////////////////////////////////////////

        namespace ramp {
            //@brief    : predefined perceptually uniform linear color maps
            //@param t  : progress along ramp [0, 1]
            //@param rgb: location to write red, green, and blue [0,1]
            template <typename Real> void gray (const Real t, Real * const rgb);//black->white
            template <typename Real> void fire (const Real t, Real * const rgb);//black->blue   ->magenta->yellow->white
            template <typename Real> void ocean(const Real t, Real * const rgb);//black->blue   ->green  ->yellow->white
            template <typename Real> void ice  (const Real t, Real * const rgb);//black->magenta->blue   ->cyan  ->white
            template <typename Real> void div  (const Real t, Real * const rgb);//blue  ->gray  ->red    (traditional divergent)
            template <typename Real> using func = void (*)(const Real, Real * const);//typedef color function signature for convince
        };

        namespace cyclic {
            //@brief    : predefined perceptually uniform cyclic color maps
            //@param t  : progress around cycle [0, 1]
            //@param rgb: location to write red, green, and blue [0,1]
            template <typename Real> void gray (const Real t, Real * const rgb);//black->white ->black
            template <typename Real> void four (const Real t, Real * const rgb);//red  ->yellow->green->blue->red (for 4 fold symmetry)
            template <typename Real> void six  (const Real t, Real * const rgb);//red  ->yellow->green->teal->blue->magenta->red (for 6 fold symmetry)
            template <typename Real> void div  (const Real t, Real * const rgb);//blue ->gray  ->red  ->gray->blue (divergent)
            template <typename Real> using func = void (*)(const Real, Real * const);//typedef color function signature for convince
        };

        //disk, sphere, and ball color maps can handle inversion symmetry
        enum class Sym {
            None   ,//no inversion symmetry
            Azimuth,//make inversion symmetric by doubling the azimuthal angle (fewer degenerate colors but perceptually flat spot at equator)
            Polar  ,//make inversion symmetric by doubling the polar angle (entire equator is degenerate color but no perceptual flat spots)
        };

        namespace disk {
            //@brief    : predefined perceptually uniform color maps for the unit disk
            //@param r  : radius [0, 1]
            //@param t  : theta [0, 1]
            //@param rgb: location to write red, green, and blue [0,1]
            //@param w0 : true/false for white/black @ r == 0
            //@param sym: type of inversion symmetry
            template <typename Real> void four (const Real r, const Real t, Real * const rgb, const bool w0 = false, const Sym sym = Sym::None);//red->yellow->green      ->blue         ->red (for 4 fold symmetry)
            template <typename Real> void six  (const Real r, const Real t, Real * const rgb, const bool w0 = false, const Sym sym = Sym::None);//red->yellow->green->teal->blue->magenta->red (for 6 fold symmetry)
            template <typename Real> using func = void (*)(const Real, const Real, Real * const, const bool, const Sym);//typedef color function signature for convince
        }

        namespace sphere {
            //@brief    : predefined perceptually uniform color maps for the unit sphere
            //@param a  : theta [0, 1] (azimuthal angle)
            //@param p  : phi [0, 1] (polar angle 0->north pole, 0.5->equator, 1-> south pole)
            //@param rgb: location to write red, green, and blue [0,1]
            //@param w0 : true/false for white/black @ r == 0
            //@param sym: type of inversion symmetry
            template <typename Real> void four (const Real a, const Real p, Real * const rgb, const bool w0 = false, const Sym sym = Sym::None);//red->yellow->green      ->blue         ->red (for 4 fold symmetry)
            template <typename Real> void six  (const Real a, const Real p, Real * const rgb, const bool w0 = false, const Sym sym = Sym::None);//red->yellow->green->teal->blue->magenta->red (for 6 fold symmetry)
            template <typename Real> using func = void (*)(const Real, const Real, Real * const, const bool, const Sym);//typedef color function signature for convince
        }

        namespace ball {
            //@brief    : predefined perceptually uniform color maps for the unit ball
            //@param r  : radius [0, 1]
            //@param a  : theta [0, 1] (azimuthal angle)
            //@param p  : phi [0, 1] (polar angle 0->north pole, 0.5->equator, 1-> south pole)
            //@param rgb: location to write red, green, and blue [0,1]
            //@param w0 : true/false for white/black @ r == 0
            //@param sym: type of inversion symmetry
            template <typename Real> void four (const Real r, const Real a, const Real p, Real * const rgb, const bool w0 = false, const Sym sym = Sym::None);//red->yellow->green      ->blue         ->red (for 4 fold symmetry)
            template <typename Real> void six  (const Real r, const Real a, const Real p, Real * const rgb, const bool w0 = false, const Sym sym = Sym::None);//red->yellow->green->teal->blue->magenta->red (for 6 fold symmetry)
            template <typename Real> using func = void (*)(const Real, const Real, const Real, Real * const, const bool, const Sym);//typedef color function signature for convince
        }

        ////////////////////////////////////////////////////////////////
        //                     Legend Generation                      //
        ////////////////////////////////////////////////////////////////

        namespace ramp {
            //@brief       : create an rgb legend for a linear color map
            //@param ramp  : color map function to use
            //@param rgb   : location to write legend image data
            //@param ripple: true/false to apply ripple / create a flat map
            //@param alpha : true/false to include an alpha channel
            //@param W     : width of color bar in pixels
            //@param H     : height of color bar in pixels
            //@param N     : number of sine waves across legend (ignored for ripple = false)
            template <typename Real> void legend(func<Real> ramp, Real * const rgb, const bool ripple, bool alpha = false, const size_t W = 512, const size_t H = 128, const size_t N = 64);
        }

        namespace cyclic {
            //@brief       : create an rgb or rgba legend for a linear color map
            //@param cyclic: color map function to use
            //@param rgb   : location to write legend image data
            //@param ripple: true/false to apply ripple / create a flat map
            //@param alpha : true/false to include an alpha channel
            //@param WH    : width/height of color bar in pixels
            //@param vFill : value to use for background
            //@param rMin  : inner radius of legend [0,1]
            //@param N     : number of sine waves around half of legend (ignored for ripple = false)
            template <typename Real> void legend(func<Real> cyclic, Real * const rgb, const bool ripple, const bool alpha = false, const size_t WH = 512, const Real vFill = 0, const Real rMin = Real(0.35), const size_t N = 64);
        }

        //disk, sphere, and ball color maps have multiple degrees of freedom to ripple application
        enum class Ripple : uint8_t {
            None      = 0x00,//no ripple
            Azimuthal = 0x01,//ripple in azimuthal direction (applicable to disk, sphere, and ball)
            Polar     = 0x02,//ripple in polar direction (applicable to sphere and ball)
            Radial    = 0x04,//ripple in radial direction (applicable to disk and sphere)
        };
        inline Ripple operator|(const Ripple& lhs, const Ripple& rhs) {return (Ripple)(static_cast<uint8_t>(lhs) | static_cast<uint8_t>(rhs));}//define bit or for Ripple enum
        inline bool   operator&(const Ripple& lhs, const Ripple& rhs) {return 0x00 != (static_cast<uint8_t>(lhs) & static_cast<uint8_t>(rhs));}//define bit and for Ripple enum

        namespace disk {
            //@brief        : create an rgb or rgba legend for a disk color map
            //@param disk   : color map function to use
            //@param rgb    : location to write legend image data
            //@param w0     : true/false for white/black center
            //@param sym    : type of symmetry to apply
            //@param rRipple: magnitude of ripple in radial direction
            //@param tRipple: magnitude of ripple in theta direction
            //@param alpha  : true/false to include an alpha channel
            //@param WH     : width/height of color bar in pixels
            //@param vFill  : value to use for background
            //@param N      : number of sine waves from r = -1->1 and theta = 0->0.5 (ignored for ripple = false)
            template <typename Real> void legend(func<Real> disk, Real * const rgb, const bool w0, const Sym sym, const Real rRipple, const Real tRipple, const bool alpha = false, const size_t WH = 512, const Real vFill = 0, const size_t N = 64);
        }

        namespace sphere {
            //type of azimuthal projections for sphere legends
            enum class Projection {
                Ortho  ,//parallel
                Stereo ,//conformal
                Lambert,//equal area
                Dist   //equal distance
            };

            //@brief        : create an rgb or rgba legend for a single hemisphere of a sphere color map
            //@param sphere : color map function to use
            //@param rgb    : location to write legend image data
            //@param nh     : true/false north / south hemisphere
            //@param proj   : type of hemisphere -> disk projection
            //@param w0     : true/false for white/black center
            //@param sym    : type of symmetry to apply
            //@param pRipple: magnitude of ripple in polar direction
            //@param aRipple: magnitude of ripple in azimuthal direction
            //@param alpha  : true/false to include an alpha channel
            //@param WH     : width/height of color bar in pixels
            //@param vFill  : value to use for background
            //@param N      : number of sine waves from p = 0->1 and a = 0->0.5 (ignored for ripple = false)
            template <typename Real> void legend(func<Real> sphere, Real * const rgb, const bool nh, const Projection proj, const bool w0, const Sym sym, const Real pRipple, const Real aRipple, const bool alpha = false, const size_t WH = 512, const Real vFill = 0, const size_t N = 64);
        }

        namespace ball {
            //@brief        : create an rgb or rgba legend for a ball color map
            //@param ball   : color map function to use
            //@param rgb    : location to write legend image data
            //@param w0     : true/false for white/black center
            //@param sym    : type of symmetry to apply
            //@param rRipple: magnitude of ripple in radial direction
            //@param pRipple: magnitude of ripple in polar direction
            //@param aRipple: magnitude of ripple in azimuthal direction
            //@param alpha  : true/false to include an alpha channel
            //@param WH     : width/height/depth of color bar in pixels
            //@param vFill  : value to use for background
            //@param N      : number of sine waves from p = 0->1 and a = 0->0.5 (ignored for ripple = false)
            template <typename Real> void legend(func<Real> ball, Real * const rgb, const bool w0, const Sym sym, const Real rRipple, const Real pRipple, const Real aRipple, const bool alpha = false, const size_t WH = 512, const Real vFill = 0, const size_t N = 64);
        }

        ////////////////////////////////////////////////////////////////
        //                   Implementation Details                   //
        ////////////////////////////////////////////////////////////////

        namespace detail {
            template <size_t N, typename Real> struct UniformBicone;//helper struct to hold a uniform bicone color map
            template <size_t N, typename Real> struct UniformLut   ;//helper struct to hold a uniform ramp or cyclic color map

            //structs to hold data for predefined maps
            template <typename Real>
            struct Maps {
                //ramps
                static const UniformLut   < 4, Real> Gray  ;//black->white
                static const UniformLut   <10, Real> Fire  ;//black->blue   ->magenta->yellow->white
                static const UniformLut   <10, Real> Ocean ;//black->blue   ->green  ->yellow->white
                static const UniformLut   <11, Real> Ice   ;//black->magenta->blue   ->cyan  ->white
                static const UniformLut   < 9, Real> Div   ;//blue ->white  ->red

                //cyclic
                static const UniformLut   <15, Real> GrayCy;//black->white ->black
                static const UniformLut   <27, Real> FourCy;//red  ->yellow->green->blue->red (for 4 fold symmetry)
                static const UniformLut   <39, Real> SixCy ;//red  ->yellow->green->teal->blue->magenta->red (for 6 fold symmetry)
                static const UniformLut   <15, Real> DivCy ;//blue ->gray  ->red  ->gray->blue (divergent)

                //bicones (for disk, sphere, and ball maps)
                static const UniformBicone< 4, Real> FourBi;//magenta,yellow,green,cyan
                static const UniformBicone< 6, Real> SixBi ;//red,yellow,green,cyan,blue,magenta
            };

            template <typename Real, size_t N, size_t K, size_t D> struct UniformSpline;//helper struct to hold a Kth degree spline in D dimensions with N control points

            template <size_t N, typename Real>
            struct UniformLut : private UniformSpline<Real, N, 3, 3> //ramps and cycles are just cubic splines in LUV
            {
            public:
                //@brief        : construct a perceptually uniform linear color map from N colors
                //@param corners: color for N points in Luv space (should be uniformly spaced in L*)
                UniformLut(const UniformSpline<Real, N, 3, 3>& spline) : UniformSpline<Real, N, 3, 3>(spline) {}

                //@brief    : get the rgb value corresponding to a fractional position on the ramp
                //@param t  : fractional position on ramp [0,1]
                //@param rgb: location to write rgb color [0,1]
                void operator() (const Real t, Real * const rgb) const
                {
                    UniformSpline<Real, N, 3, 3>::interpolate(t, false, rgb);
                    color::luv2rgb(rgb, rgb);
                }
            };

            template <size_t N, typename Real>
            struct UniformBicone
            {
            public:
                //@brief        : perceptually uniform HSL like coloring with N colors specified around the equator
                //@param corners: color for N points around equator (hue control points for lightness = 0.5)
                //@param l0     : L* at tip of bottom cone pole (lightness = 0)
                //@param l1     : L* at tip of top    cone pole (lightness = 1)
                UniformBicone(Real const * const corners, const Real l0, const Real l1);

                //@brief       : convert from surface of perceptually uniform HSL like space (fully saturated) to rgb
                //@param h     : fractional hue [0,1]
                //@param l     : fractional lightness [0,1]
                //@param rgb   : location to write rgb color [0,1]
                //@param mirror: true/false if colors should be smooth with/without a mirror plane at l = 0.5
                void operator()(const Real h, const Real l, Real * const rgb, const bool mirror) const
                {
                    hl2luv(h, l, rgb, mirror);
                    color::luv2rgb(rgb, rgb);
                }//compute LUV coordinates and -> rgb

                //@brief       : convert from perceptually uniform HSL like space to rgb
                //@param h     : fractional hue [0,1]
                //@param r     : fractional radius [0,1] (0 -> center of bicone, 1 -> surface)
                //@param l     : fractional lightness [0,1]
                //@param rgb   : location to write rgb color [0,1]
                //@param mirror: true/false if colors should be smooth with/without a mirror plane at l = 0.5
                void operator()(const Real h, const Real r, const Real l, Real * const rgb, const bool mirror) const;

                //@brief      : map from a 2D direction to a color (perceptually uniform in r and theta)
                //@param r    : fractional radius [0,1]
                //@param theta: fractional angle [0,1]
                //@param rgb  : location to write rgb color
                //@param w0   : true/false for white/black @ r == 0
                //@param sym  : type of inversion symmetry
                void disk(const Real r, const Real theta, Real * const rgb, const bool w0, const Sym sym) const;

                //@brief    : map from a 3D unit direction to a color (perceptually uniform in polar and azimuthal angle)
                //@param a  : fractional azimuthal angle [0,1]
                //@param p  : fractional polar angle [0,1]
                //@param rgb: location to write rgb color
                //@param w0 : true/false for white/black @ phi = 0
                //@param sym: type of inversion symmetry
                void sphere(const Real a, const Real p, Real * const rgb, const bool w0, const Sym sym) const;

                //@brief    : map from a 3D direction to a color (perceptually uniform in polar and azimuthal angle)
                //@param r  : fractional radius [0,1]
                //@param a  : fractional azimuthal angle [0,1]
                //@param p  : fractional polar angle [0,1]
                //@param rgb: location to write rgb color
                //@param w0 : true/false for white/black @ phi = 0
                //@param sym: type of inversion symmetry
                void ball(const Real r, const Real a, const Real p, Real * const rgb, const bool w0, const Sym sym) const;

            private:
                //@brief       : actual implementation of perceptually uniform bicone in Luv space
                //@param h     : fractional hue [0,1]
                //@param l     : fractional lightness [0,1]
                //@param rgb   : location to write rgb color
                //@param mirror: true/false if colors should be smooth at equator with/without a mirror plane at l = 0.5
                //@param smooth: true/false if colors should be smooth at equator
                void hl2luv(const Real h, const Real l, Real * const luv, const bool mirror, const bool smooth = true) const;

                const Real minL, midL, maxL;//luminance of south pole, equator (average), and north pole
                const detail::UniformSpline<Real, (N+1)*3, 3, 3> eqSpline;//cubic spline to interpolate colors around equator
            };

            ////////////////////////////////////////////////////////////////
            //               Uniform Spline Implementation                //
            ////////////////////////////////////////////////////////////////

            template <typename Real, size_t N, size_t K, size_t D>
            struct UniformSpline
            {
            public:
                //@brief    : construct a K degree B-spline from N control points in D dimensions
                //@param pts: control points
                UniformSpline(Real const * const pts);

                //@brief        : interpolate coordinates using de Boor's algorithm for uniform knots
                //@param t      : parametric distance along spline [0, 1]
                //@param clamped: true/false to use clamped/unclamped uniform knots
                //@param pt     : location to write interpolated coordinates
                void interpolate(const Real t, const bool clamped, Real * const pt) const;

            private:
                std::array<Real, N*D> P;//control points
            };

            //@brief    : construct a cyclic b-spline from a control polygon (sides will be subdivided to accommodate degree)
            //@param pts: control points
            template<typename Real, size_t N, size_t K, size_t D>
            UniformSpline<Real, N, K, D>::UniformSpline(Real const * const pts) {
                static_assert(std::is_floating_point<Real>::value && K > 0 && K < N, "spline must be templated on floating point type with 1 <= degree < # pts");
                std::copy(pts, pts + N * D, P.begin());
            }

            //@brief        : interpolate coordinates using de Boor's algorithm for uniform knots
            //@param t      : parametric distance along spline [0, 1]
            //@param clamped: true/false to use clamped/unclamped uniform knots
            //@param pt     : location to write interpolated coordinates
            template<typename Real, size_t N, size_t K, size_t D>
            void UniformSpline<Real, N, K, D>::interpolate(const Real t, const bool clamped, Real * const pt) const {
                //remap t to knot domain and find segment t falls in
                if(t < Real{0} || t > Real{1}) throw std::out_of_range("spline parameter out of bounds [0,1]");
                static const size_t uMax = N - K;//maximum knot value for clamped knots
                const Real tt = clamped ? t * uMax : t * (N - K) + K;
                const size_t s = std::min((clamped ? K : 0) + (size_t)tt, N - 1);

                //copy control points to working array
                Real work[(K+1) * D];
                std::copy(P.begin() + (s-K) * D, P.begin() + (s+1) * D, work);//only points s-degree -> s (inclusive) are required

                //recursively compute coordinates (de Boor's algorithm)
                for(size_t k = 0; k <= K; k++) {
                    for(size_t i = s; i > s+k-K; i--) {
                        const size_t iKk = std::min<size_t>(i+K-k, N);
                        const size_t ui = i > K ? i - K : 0;//knots[i] for clamped knots
                        const size_t uiKk = iKk > K ? iKk - K : 0;//knots[iKk] for clamped knots
                        const Real w = clamped ? (tt - ui) / (uiKk - ui) : (tt - i) / (K - k) ;//compute weight
                        const Real x = Real{1} - w;//1 - weight
                        Real * const iter = work;
                        const size_t os = (i+K-s) * D; // determine offset
                                                       // once. Computing a separate
                                                       // offset in this line avoids
                                                       // array-offset warnings with
                                                       // some compilers
                        std::transform(iter + os, iter + os + D, iter + os - D, iter + os, [w, x](const Real& i, const Real& j) {
                            return i * w + j * x;
                        });//recursive calculation
                    }
                }
                std::copy(work + K * D, work + K * D + D, pt);
            }

            //@brief    : construct a cyclic b-spline from a control polygon by subdividing polygon sides to accommodate degree
            //@param pts: control polygon (d * n)
            template<typename Real, size_t N, size_t K, size_t D>
            UniformSpline<Real, (N+1)*K, K, D> splineLoop(Real const * const pts) {
                //compute linear interpolation weights once
                std::array<Real, K> weights;
                for(size_t i = 0; i < K; i++) weights[i] = Real(K - i) / K;

                //linearly interpolate extra control points
                std::array<Real, (N+1)*K * D> subPoints;
                for(size_t i = 0; i < N; i++)//loop over original points
                    for(size_t j = 0; j < K; j++)//loop over degrees (split each segment into K segments)
                        for(size_t k = 0; k < D; k++)//loop over dimensions
                            subPoints[i * K * D + j * D + k] = pts[i * D + k] * weights[j] + pts[((i+1) % N) * D + k] * (Real(1) - weights[j]);
                std::rotate(subPoints.begin(), subPoints.end() - D * ((K-1)/2) - K * D, subPoints.end() - K * D);//rotate control points so t = 0 is at (odd degree) or closely aligned with (even degree) first control point
                std::copy(subPoints.begin(), subPoints.begin() + K * D, subPoints.end() - K * D);//copy first degree control points to the end to make cyclic
                return UniformSpline<Real, (N+1)*K, K, D>(subPoints.data());
            }

            //@brief    : pad control points to create splace with f''(t)==0 at endpoints
            //@param pts: points to pad
            //@return   : spline constructed from padded points
            template <size_t N, typename Real, size_t K, size_t D>
            detail::UniformSpline<Real, N + 2, K, D> paddedSpline(Real const * const pts) {
                std::array<Real, (N+2) * D> padded;
                std::copy(pts, pts + N * D, padded.begin() + D);
                std::transform(pts, pts + D, pts + D, padded.begin(), [](const Real& v0, const Real& v1){return v0 * 2 - v1;});
                std::transform(pts + (N-1) * D, pts + N * D, pts + (N-2) * D, padded.end() - D, [](const Real& v0, const Real& v1){return v0 * 2 - v1;});
                return detail::UniformSpline<Real, N + 2, K, D>(padded.data());
            }

            ////////////////////////////////////////////////////////////////
            //              Uniform Bicone Member Functions               //
            ////////////////////////////////////////////////////////////////

            //@brief        : construct a UniformBicone object
            //@param corners: color for N equally spaced points around equator
            //@param l0     : luminance of south pole (chromaticity = 0)
            //@param l1     : luminance of north pole (chromaticity = 0)
            template <size_t N, typename Real>
            UniformBicone<N, Real>::UniformBicone(Real const * const corners, const Real l0, const Real l1) :
                minL(l0),
                midL([](Real const * const c)->Real{
                    Real mean(0);
                    for(size_t i = 0; i < N; i++) mean += c[3*i];
                    return mean / N;
                }(corners)),
                maxL(l1),
                eqSpline(detail::splineLoop<Real, N, 3, 3>(corners)) {}

            //@brief       : convert from perceptually uniform HSL like space to rgb
            //@param h     : fractional hue [0,1]
            //@param r     : fractional radius [0,1] (0 -> center of bicone, 1 -> surface)
            //@param l     : fractional lightness [0,1]
            //@param rgb   : location to write rgb color
            //@param mirror: true/false if colors should be smooth with/without a mirror plane at l = 0.5
            template <size_t N, typename Real>
            void UniformBicone<N, Real>::operator()(const Real h, const Real r, const Real l, Real * const rgb, const bool mirror) const {
                hl2luv(h, l, rgb, mirror);//compute LUV coordinates
                rgb[0] = (rgb[0] - midL) * r + midL;//rescale L from midpoint (midL, 0, 0)
                rgb[1] *= r; rgb[2] *= r;//rescale chromaticity from midpoint (midL, 0, 0)
                color::luv2rgb(rgb, rgb);
            }

            //@brief      : map from a 2D direction to a color (perceptually uniform in r and theta)
            //@param r    : fractional radius [0,1]
            //@param theta: fractional angle [0,1]
            //@param rgb  : location to write rgb color
            //@param w0   : true/false for white/black @ r == 0
            //@param sym  : type of inversion symmetry
            template <size_t N, typename Real>
            void UniformBicone<N, Real>::disk(const Real r, const Real theta, Real * const rgb, const bool w0, const Sym sym) const{// {hl2luv(inv ? (theta > Real(0.5) ? theta * 2 - 1 : theta * 2) : theta, w0 ? Real(1) - r / 2: r / 2, inv, inv);}
                switch(sym) {
                case Sym::None   : hl2luv(theta                                        , w0 ? Real(1) - r / 2 : r / 2, rgb, false, false); break;//select cone based on center color
                case Sym::Azimuth: hl2luv(theta > Real(0.5) ? theta * 2 - 1 : theta * 2, w0 ? Real(1) - r / 2 : r / 2, rgb, true , true ); break;//double azimuthal angle and select cone based on center color
                case Sym::Polar  : hl2luv(theta                                        , w0 ? Real(1) - r     : r    , rgb, false, true ); break;//double double polar angle
                }
                color::luv2rgb(rgb, rgb);//luv -> rgb
            }

            //@brief    : map from a 3D unit direction to a color (perceptually uniform in polar and azimuthal angle)
            //@param a  : fractional azimuthal angle [0,1]
            //@param p  : fractional polar angle [0,1]
            //@param rgb: location to write rgb color
            //@param w0 : true/false for white/black @ phi = 0
            template <size_t N, typename Real>
            void UniformBicone<N, Real>::sphere(const Real a, const Real p, Real * const rgb, const bool w0, const Sym sym) const {
                //first move to northern hemisphere if needed
                const bool sh = p > Real(0.5);
                const bool swap = sh && Sym::None != sym;
                const Real az = swap ? (a < Real(0.5) ? a + Real(0.5) : a - Real(0.5)) : a;
                const Real pl = swap ? Real(1) - p : p;

                //compute color in luv space
                switch(sym) {
                case Sym::None   : hl2luv(az                                  , w0 ? Real(1) - pl     : pl    , rgb, false, true); break;//select cone based on center color
                case Sym::Azimuth: hl2luv(az < Real(0.5) ? az * 2 : az * 2 - 1, w0 ? Real(1) - pl     : pl    , rgb, true , true); break;//double azimuthal angle and select cone based on center color
                case Sym::Polar  : hl2luv(az                                  , w0 ? Real(1) - pl * 2 : pl * 2, rgb, false, true); break;//double double polar angle
                }
                color::luv2rgb(rgb, rgb);//luv -> rgb
            }

            //@brief    : map from a 3D direction to a color (perceptually uniform in polar and azimuthal angle)
            //@param r  : fractional radius [0,1]
            //@param a  : fractional azimuthal angle [0,1]
            //@param p  : fractional polar angle [0,1]
            //@param rgb: location to write rgb color
            //@param w0 : true/false for white/black @ phi = 0
            //@param sym: type of inversion symmetry
            template <size_t N, typename Real>
            void UniformBicone<N, Real>::ball(const Real r, const Real a, const Real p, Real * const rgb, const bool w0, const Sym sym) const {
                //first move to northern hemisphere if needed
                const bool sh = p > Real(0.5);
                const bool swap = sh && Sym::None != sym;
                const Real az = swap ? (a < Real(0.5) ? a + Real(0.5) : a - Real(0.5)) : a;
                const Real pl = swap ? Real(1) - p : p;

                //compute color in luv space
                switch(sym) {
                case Sym::None   : hl2luv(az                                  , w0 ? Real(1) - pl     : pl    , rgb, false, false); break;//select cone based on center color
                case Sym::Azimuth: hl2luv(az < Real(0.5) ? az * 2 : az * 2 - 1, w0 ? Real(1) - pl     : pl    , rgb, true , true ); break;//double azimuthal angle and select cone based on center color
                case Sym::Polar  : hl2luv(az                                  , w0 ? Real(1) - pl * 2 : pl * 2, rgb, false, true ); break;//double double polar angle
                }

                //resacle color by radius
                rgb[0] = (rgb[0] - midL) * r + midL;
                rgb[1] *= r; rgb[2] *= r;
                color::luv2rgb(rgb, rgb);//luv -> rgb
            }

            //@brief       : compute perceptually uniform HSL like color in Luv space
            //@param h     : fractional hue [0,1]
            //@param l     : fractional lightness [0,1]
            //@param luv   : location to write luv color
            //@param mirror: should colors be smooth with/without a mirror plane at l = 0.5
            //@param smooth: true/false to make L* C1 continuous across equator (at expense of uniformity)
            template <size_t N, typename Real>
            void UniformBicone<N, Real>::hl2luv(const Real h, const Real l, Real * const luv, const bool mirror, const bool smooth) const {
                //spline interpolate color at equator (L = 0.5) for this hue
                eqSpline.interpolate(h, false, luv);

                //apply nonlinear rescaling to lightness interpolation to impose C2 continuity at equator
                //I'll use a piecewise polynomial with linear portions near the poles to maximize truly perceptually uniform region
                //        / f1 =                       c1 * x + d1 : 0       <= x <= 1/2 - t (linear from pole to first transition)
                // f(x) = | f2 = a2 * x^3 + b2 * x^2 + c2 * x + d2 : 1/2 - t <  x <= 1/2     (cubic from first transition to equator)
                //        | f3 = a3 * x^3 + b3 * x^2 + c3 * x + d3 : 1/2     <  x <  1/2 + t (cubic from equator to second transition)
                //        \ f4 =                       c4 * x + d4 : 1/2 + t <= x <= 1       (linear from second transition to pole)
                //12 unknowns solved for with the following 12 constraints
                //-C2 continuity between f1/f2, f2/f3, and f3, f4 (9 constraints)
                //-f(0) = minL, f(1/2) = luv[0], f(1) = maxL (3 constraints)
                static const Real tl = Real(0.1);//offset from equator to end of transition for C2 continiuity of L* [0,0.5] 0 -> true perceptually uniformity with visual discontinuity, 0.5 -> largest deviation from perceptually uniformity but spreads discontinuity over largest area
                const bool sh = l <= Real(0.5);//does this point fall below/above the equator (true/false)
                const Real deltaS = minL - luv[0];//delta luminance from equator to south pole
                const Real deltaN = maxL - luv[0];//delta luminance from equator to north pole
                const Real l0 = sh ? deltaS : (mirror ? deltaN : deltaS);//delta luminance at l == 0
                const Real l1 = sh ? (mirror ? deltaS : deltaN) : deltaN;//delta luminance at l == 1
                Real deltaL;
                if(smooth) {
                    const Real x = ( l0 + l1 ) / ( tl * 2 - 3 );//this value will be needed in all for segments
                    if(sh) {//in f1 or f2, don't bother calculating coefficients for f3 or f4
                        const Real hmt = Real(0.5) - tl;//transition from linear -> cubic in southern cone
                        const Real c1 = (x * tl - l0) * 2;//compute slope of linear region in southern cone
                        const Real d1 = l0;//compute intercept of linear region in southern cone
                        if(l <= hmt) {//in first linear region (f1)
                            deltaL = c1 * l + d1;//compute f1(l)
                        } else {//in first cubic region (f2)
                            const Real a2 = -x / (tl * tl);
                            const Real b2 = -a2 * hmt * 3;
                            const Real c2 =  c1 - b2 * hmt;
                            const Real d2 =  d1 - a2 * hmt * hmt * hmt;
                            const Real ll = l * l;//compute l^2 once
                            deltaL = a2 * ll * l + b2 * ll + c2 * l + d2;//compute f2(l)
                        }
                    } else {//in f3 or f4, don't bother calculating coefficients for f1 or f2
                        const Real hpt = Real(0.5) + tl;//transition from cubic -> linear in northern cone
                        const Real c4 = (l1 - x * tl) * 2;//compute slope of linear region in northern cone
                        const Real d4 = l1 - c4;//compute intercept of linear region in northern cone
                        if(l >= hpt) {//in second linear region (f4)
                            deltaL = c4 * l + d4;//compute f4(l)
                        } else {//in second cubic region (f3)
                            const Real a3 =  x / (tl * tl);
                            const Real b3 = -a3 * hpt * 3;
                            const Real c3 =  c4 - b3 * hpt;
                            const Real d3 =  d4 - a3 * hpt * hpt * hpt;
                            const Real ll = l * l;//compute l^2 once
                            deltaL = a3 * ll * l + b3 * ll + c3 * l + d3;//compute f3(l)
                        }
                    }
                } else {
                    deltaL = sh ? (l * -2 + 1) * l0 : (l *  2 - 1) * l1;//compute f1(l)
                }

                //interpolate luminance and compute scaling factor for chromaticity
                luv[0] += deltaL;
                Real fc = Real(1) - deltaL / (sh ? l0 : l1);//0->1 bicone scaling

                //adjust chromaticity scaling factor similarly to lightness to make chromaticity C1 continous at equator
                // f(x) = / f1 =                         x     : 0  < x <= tc (linear from pole to tc)
                //        \ f2 = a * x^3 + b * x^2 + c * x + d : tc < x <= 1  (cubic from tc to equator)
                //4 unknowns solved for with C2 continuity between f1/f2 (3 constraints) and f'(1) = 0 (C1 continuity at equator)
                static const Real tc = Real(0.8);//this parameter is less sensitive than tl since local uniformity is more strongly L* dependant
                static const Real ac = Real(-1) / ( (tc - 1) * (tc - 1) * 3 );
                static const Real bc = -tc * 3 * ac;
                static const Real cc = (tc * 6 - 3) * ac;
                static const Real dc = -tc * tc * tc * ac;
                if(fc > tc && smooth) {
                    const Real fcfc = fc * fc;//compute fc^2 once
                    fc = ac * fcfc * fc + bc * fcfc + cc * fc + dc;//compute f2(fc)
                }
                luv[1] *= fc; luv[2] *= fc;
            }

            ////////////////////////////////////////////////////////////////
            //         Predefined Uniform Struct Implementations          //
            ////////////////////////////////////////////////////////////////

            //ramps, comments are qualitatively closest web color
            template <typename Real> const UniformLut   < 4, Real> Maps<Real>::Gray   = UniformLut   < 4, Real>(detail::paddedSpline<2, Real, 3, 3>(std::array<Real, 2*3>({
                            Real(  0), Real(0), Real(0),//black
                            Real(100), Real(0), Real(0),//white
                        }).data()));

            template <typename Real> const UniformLut   <10, Real> Maps<Real>::Fire   = UniformLut   <10, Real>(detail::paddedSpline<8, Real, 3, 3>(std::array<Real, 8*3>({
                            Real( 1), Real(  0), Real(  0),//black
                            Real(15), Real( -2), Real(-53),//navy
                            Real(29), Real( 46), Real(-45),//dark magenta
                            Real(43), Real(101), Real(-21),//medium violet red
                            Real(57), Real(153), Real( 44),//orange red
                            Real(71), Real( 87), Real( 68),//dark orange
                            Real(85), Real( 12), Real( 93),//yellow
                            Real(99), Real(  0), Real(  0),//white
                        }).data()));

            template <typename Real> const UniformLut   <10, Real> Maps<Real>::Ocean  = UniformLut<10, Real>(detail::paddedSpline<8, Real, 3, 3>(std::array<Real, 8*3>({
                            Real( 1), Real(  0), Real(   0),//black
                            Real(15), Real( 17), Real( -27),//purple
                            Real(29), Real( -8), Real(-114),//blue
                            Real(43), Real(-30), Real( -30),//steel blue
                            Real(57), Real(-49), Real(  28),//sea green
                            Real(71), Real(-66), Real(  86),//lime green
                            Real(85), Real(  5), Real(  96),//yellow
                            Real(99), Real(  0), Real(   0),//white
                        }).data()));

            template <typename Real> const UniformLut   <11, Real> Maps<Real>::Ice    = UniformLut   <11, Real>(detail::paddedSpline<9, Real, 3, 3>(std::array<Real, 9*3>({
                            Real( 2), Real(  0), Real(   0),//black
                            Real(14), Real( 43), Real(   8),//dark red
                            Real(26), Real( 34), Real( -50),//purple
                            Real(38), Real( 22), Real(-101),//blue violet
                            Real(50), Real( -2), Real(-132),//medium slate blue
                            Real(62), Real(-34), Real( -91),//deep sky blue
                            Real(74), Real(-55), Real( -35),//sky blue
                            Real(86), Real(-71), Real(  33),//aquamarine
                            Real(98), Real(  0), Real(   0),//white
                        }).data()));

            template <typename Real> const UniformLut   < 9, Real> Maps<Real>::Div    = UniformLut   < 9, Real>(detail::paddedSpline<7, Real, 3, 3>(std::array<Real, 7*3>({
                            Real(35), Real( -9), Real(-132),//blue
                            Real(56), Real( -6), Real( -88),
                            Real(77), Real( -3), Real( -44),
                            Real(98), Real(  0), Real(   0),//white
                            Real(77), Real( 38), Real(   8),
                            Real(56), Real( 76), Real(  17),
                            Real(35), Real(114), Real(  25),//firebrick
                        }).data()));

            //cyclic, comments are qualitatively closest web color
            template <typename Real> const UniformLut   <15, Real> Maps<Real>::GrayCy = UniformLut   <15, Real>(detail::splineLoop<Real, 4, 3, 3>(std::array<Real, 4*3>({
                            Real( 50), Real(0), Real(0),//gray
                            Real(106), Real(0), Real(0),//white
                            Real( 50), Real(0), Real(0),//gray
                            Real( -6), Real(0), Real(0),//black
                        }).data()));

            template <typename Real> const UniformLut   <27, Real> Maps<Real>::FourCy = UniformLut   <27, Real>(detail::splineLoop<Real, 8, 3, 3>(std::array<Real, 8*3>({
                            Real(63), Real( 71), Real( -97),//violet
                            Real(79), Real( 39), Real(   2),
                            Real(95), Real(  8), Real( 102),//yellow
                            Real(79), Real(-25), Real(  89),
                            Real(63), Real(-59), Real(  76),//lime green
                            Real(47), Real(-34), Real( -22),
                            Real(31), Real( -9), Real(-121),//medium blue
                            Real(47), Real( 16), Real(-125),
                        }).data()));

            template <typename Real> const UniformLut   <39, Real> Maps<Real>::SixCy  = UniformLut   <39, Real>(detail::splineLoop<Real, 12, 3, 3>(std::array<Real, 12*3>({
                            Real(55), Real(109), Real( -55),//deep pink
                            Real(65), Real( 89), Real(  -2),
                            Real(75), Real( 70), Real(  50),//light salmon
                            Real(85), Real( 39), Real(  76),
                            Real(95), Real(  8), Real( 102),//yellow
                            Real(85), Real(-31), Real(  95),
                            Real(75), Real(-70), Real(  89),//lime grean
                            Real(65), Real(-55), Real(  34),
                            Real(55), Real(-40), Real( -21),//cadet blue
                            Real(45), Real(-25), Real( -76),
                            Real(35), Real(-10), Real(-132),//blue
                            Real(45), Real( 27), Real(-126),
                        }).data()));

            template <typename Real> const UniformLut   <15, Real> Maps<Real>::DivCy  = UniformLut   <15, Real>(detail::splineLoop<Real, 4, 3, 3>(std::array<Real, 6*3>({
                            Real(98), Real(  0), Real(   0),//white
                            Real(27), Real(128), Real(  28),//firebrick
                            Real(98), Real(  0), Real(   0),//white
                            Real(27), Real(-10), Real(-149),//blue
                        }).data()));

            //bicone
            template <typename Real> const UniformBicone< 4, Real> Maps<Real>::FourBi = UniformBicone< 4, Real>(std::array<Real,  4*3>({
                        Real(40), Real( 55.74317350486855), Real(-72.07542853960882),//m ~0xA900A9 as 24 bit rgb (L* = 40 on magenta -> black line)
                        Real(70), Real( 86.73783054671324), Real( 67.08308275883844),//y ~0xFA9200 as 24 bit rgb (L* = 70, most saturated color in sRGB bisecting m/g)
                        Real(40), Real(-37.87131352881698), Real( 48.96727222946806),//g ~0x006E00 as 24 bit rgb (L* = 40 on green   -> black line)
                        Real(70), Real(-50.70373411917466), Real(-39.21429404747341),//c ~0x00BBDB as 24 bit rgb (L* = 70, most saturated color in sRGB bisecting m/g)
                    }).data(), 12, 98);

            template <typename Real> const UniformBicone< 6, Real> Maps<Real>::SixBi  = UniformBicone< 6, Real>(std::array<Real,  6*3>({
                        Real(40), Real(131.49157), Real(  28.36797),//r ~0xC00000 as 24 bit rgb (L* = 40 on red     -> black line)
                        Real(70), Real( 27.13770), Real(  74.3228 ),//y ~0xC7A900 as 24 bit rgb (L* = 70 on surface of sRGB cube bisecting r/g)
                        Real(40), Real(-37.87359), Real(  48.97021),//g ~0x006E00 as 24 bit rgb (L* = 40 on green   -> black line)
                        Real(70), Real(-53.71706), Real( -15.01116),//c ~0x00BEC2 as 24 bit rgb (L* = 70 on surface of sRGB cube bisecting g/b)
                        Real(40), Real(-15.20070), Real(-132.90262),//b ~0x0043FF as 24 bit rgb (L* = 40 on blue    -> cyan  line)
                        Real(70), Real( 68.67266), Real( -62.23589),//m ~0xFF79E8 as 24 bit rgb (L* = 70 on surface of sRGB cube bisecting b/r)
                    }).data(), 12, 98);

            ////////////////////////////////////////////////////////////////
            //                   Test Signal Generation                   //
            ////////////////////////////////////////////////////////////////

            //@brief           : evaluate the test signal at a single value
            //@param x         : value to compute the test signal for [0,1]
            //@param periodic  : true/false if the signal will be used for periodic function
            //@param numPeriods: number of sine wave periods to use
            //@param amplitude : amplitude of sine wave (defaults to 0.05 or 10% of [0,1])
            template <typename Real> Real testSignal(const Real x, const bool periodic, const size_t numPeriods = 64, const Real amplitude = Real(0.05)) {
                if(periodic) {
                    const Real y = std::fmod(x + std::sin(x * M_PI * 2 * numPeriods) * amplitude, 1);//compute test signal value restricted to [-1,1]
                    return std::signbit(y) ? y + 1 : y;
                } else {
                    //compute minimum and maximum values of x + sin(x * pi * 2 * numPeriods) * amplitude
                    const Real kk = M_PI * 2 * amplitude * numPeriods;
                    const bool clip = kk <= Real(1);
                    const Real k = clip ? Real(1) / (2 * numPeriods) : std::acos(Real(-1) / kk) / (M_PI * 2 * numPeriods);//offset for extrema
                    const Real tMin = Real(           1) / numPeriods - k;//location of minima in first period
                    const Real tMax = Real(numPeriods-1) / numPeriods + k;//location of maxima in last period
                    const Real sk = clip ? 0 : std::sqrt(Real(1) - Real(1) / (kk * kk) );//sin(tMax * M_PI * 2 * N) && -sin(tMin * M_PI * 2 * N)
                    const Real vMin = std::min(tMin - sk * amplitude, Real(0));//value of signal minimum
                    const Real vMax = std::max(tMax + sk * amplitude, Real(1));

                    //compute linear modulation so that signal falls in [0,1] and compute
                    const Real m = Real(1) / (vMax - vMin);
                    const Real b = m * -vMin;
                    return (x + std::sin(x * M_PI * 2 * numPeriods) * amplitude) * m + b;//compute test signal value
                }
            }

            //@brief           : construct a test signal (linear ramp + sine wave in [0,1])
            //@param numSamples: number of equally spaced points along domain ([0,1]) to compute signal for
            //@param signal    : location to write test signal (must be able to hold numSamples)
            //@param periodic  : true/false if the signal will be used for periodic function
            //@param numPeriods: number of sine wave periods to use
            //@param amplitude : amplitude of sine wave (defaults to 0.05 or 10% of [0,1])
            template <typename Real> void testSignal(const size_t numSamples, Real * const signal, const bool periodic, const size_t numPeriods = 64, const Real amplitude = Real(0.05)) {
                if(periodic) {
                    //build signal
                    for(size_t i = 0; i < numSamples; i++) {
                        const Real x = Real(i) / (numSamples-1);
                        const Real y = std::fmod(x + std::sin(x * M_PI * 2 * numPeriods) * amplitude, 1);//compute test signal value restricted to [-1,1]
                        signal[i] = std::signbit(y) ? y + 1 : y;
                    }
                } else {
                    //compute minimum and maximum values of x + sin(x * pi * 2 * numPeriods) * amplitude
                    const Real kk = M_PI * 2 * amplitude * numPeriods;
                    const bool clip = kk <= Real(1);
                    const Real k = clip ? Real(1) / (2 * numPeriods) : std::acos(Real(-1) / kk) / (M_PI * 2 * numPeriods);//offset for extrema
                    const Real tMin = Real(           1) / numPeriods - k;//location of minima in first period
                    const Real tMax = Real(numPeriods-1) / numPeriods + k;//location of maxima in last period
                    const Real sk = clip ? 0 : std::sqrt(Real(1) - Real(1) / (kk * kk) );//sin(tMax * M_PI * 2 * N) && -sin(tMin * M_PI * 2 * N)
                    const Real vMin = std::min(tMin - sk * amplitude, Real(0));//value of signal minimum
                    const Real vMax = std::max(tMax + sk * amplitude, Real(1));

                    //compute linear modulation so that signal falls in [0,1]
                    const Real m = Real(1) / (vMax - vMin);
                    const Real b = m * -vMin;

                    //build signal
                    for(size_t i = 0; i < numSamples; i++) {
                        const Real x = Real(i) / (numSamples-1);
                        signal[i] = (x + std::sin(x * M_PI * 2 * numPeriods) * amplitude) * m + b;//compute test signal value
                    }
                }
            }
        }//namespace detail

        ////////////////////////////////////////////////////////////////
        //           Predefined Color Maps Implementations            //
        ////////////////////////////////////////////////////////////////

        namespace ramp {
            template <typename Real> void gray (const Real t, Real * const rgb) { detail::Maps<Real>::Gray  (t, rgb); }
            template <typename Real> void fire (const Real t, Real * const rgb) { detail::Maps<Real>::Fire  (t, rgb); }
            template <typename Real> void ocean(const Real t, Real * const rgb) { detail::Maps<Real>::Ocean (t, rgb); }
            template <typename Real> void ice  (const Real t, Real * const rgb) { detail::Maps<Real>::Ice   (t, rgb); }
            template <typename Real> void div  (const Real t, Real * const rgb) { detail::Maps<Real>::Div   (t, rgb); }
        }

        namespace cyclic {
            //@brief    : predefined perceptually uniform cyclic color maps
            //@param t  : progress around cycle [0, 1]
            //@param rgb: location to write red, green, and blue [0,1]
            template <typename Real> void gray (const Real t, Real * const rgb) { detail::Maps<Real>::GrayCy(t, rgb); }
            template <typename Real> void four (const Real t, Real * const rgb) { detail::Maps<Real>::FourCy(t, rgb); }
            template <typename Real> void six  (const Real t, Real * const rgb) { detail::Maps<Real>::SixCy (t, rgb); }
            template <typename Real> void div  (const Real t, Real * const rgb) { detail::Maps<Real>::DivCy (t, rgb); }
        };

        namespace disk {
            //@brief    : predefined perceptually uniform color maps for the unit disk
            //@param r  : radius [0, 1]
            //@param t  : theta [0, 1]
            //@param rgb: location to write red, green, and blue [0,1]
            //@param w0 : true/false for white/black @ r == 0
            //@param sym: type of inversion symmetry
            template <typename Real> void four (const Real r, const Real t, Real * const rgb, const bool w0, const Sym sym) {detail::Maps<Real>::FourBi.disk(r, t, rgb, w0, sym);}
            template <typename Real> void six  (const Real r, const Real t, Real * const rgb, const bool w0, const Sym sym) {detail::Maps<Real>::SixBi .disk(r, t, rgb, w0, sym);}
        }

        namespace sphere {
            //@brief    : predefined perceptually uniform color maps for the unit sphere
            //@param a  : theta [0, 1] (azimuthal angle)
            //@param p  : phi [0, 1] (polar angle 0->north pole, 0.5->equator, 1-> south pole)
            //@param rgb: location to write red, green, and blue [0,1]
            //@param w0 : true/false for white/black @ r == 0
            //@param sym: type of inversion symmetry
            template <typename Real> void four (const Real a, const Real p, Real * const rgb, const bool w0, const Sym sym) {detail::Maps<Real>::FourBi.sphere(p, a, rgb, w0, sym);}
            template <typename Real> void six  (const Real a, const Real p, Real * const rgb, const bool w0, const Sym sym) {detail::Maps<Real>::SixBi .sphere(p, a, rgb, w0, sym);}
        }

        namespace ball {
            //@brief    : predefined perceptually uniform color maps for the unit ball
            //@param r  : radius [0, 1]
            //@param a  : theta [0, 1] (azimuthal angle)
            //@param p  : phi [0, 1] (polar angle 0->north pole, 0.5->equator, 1-> south pole)
            //@param rgb: location to write red, green, and blue [0,1]
            //@param w0 : true/false for white/black @ r == 0
            //@param sym: type of inversion symmetry
            template <typename Real> void four (const Real r, const Real a, const Real p, Real * const rgb, const bool w0, const Sym sym) {detail::Maps<Real>::FourBi.ball(r, a, p, rgb, w0, sym);}
            template <typename Real> void six  (const Real r, const Real a, const Real p, Real * const rgb, const bool w0, const Sym sym) {detail::Maps<Real>::SixBi .ball(r, a, p, rgb, w0, sym);}
        }

        ////////////////////////////////////////////////////////////////
        //         Legend Generation Function Implementations         //
        ////////////////////////////////////////////////////////////////

        //@brief       : create an rgb legend for a linear color map
        //@param ramp  : color map function to use
        //@param rgb   : location to write legend image data
        //@param ripple: true/false to apply ripple / create a flat map
        //@param alpha : true/false to include an alpha channel
        //@param W     : W of color bar in pixels
        //@param H     : H of color bar in pixels
        //@param N     : number of sine waves across legend (ignored for ripple = false)
        template <typename Real> void ramp::legend(ramp::func<Real> ramp, Real * const rgb, const bool ripple, const bool alpha, const size_t W, const size_t H, const size_t N) {
            Real color[4] = {0, 0, 0, 1};//set alpha channel to 1
            const size_t stride = alpha ? 4 : 3;//is this an rgb or an rgba image
            if(ripple) {//for ripple every row is different
                std::vector<Real> signal(W);//allocate memory to hold test signal once
                for(size_t j = 0; j < H; j++) {//loop over rows
                    const size_t offset = W * stride * j;//compute offset to row start
                    const Real x = Real(j) / (H - 1);//compute fractional progress
                    detail::testSignal(W, signal.data(), false, N, Real(0.05) * x * x);//build test signal for this row
                    for(size_t i = 0; i < W; i++) {//loop over columns converting from signal -> color
                        ramp(signal[i] , color);//compute color
                        std::copy(color, color + stride, rgb + offset + stride * i);//copy to output
                    }
                }
            } else {//for flat every row is the same
                for(size_t i = 0; i < W; i++) {//loop first row computing color
                    ramp(Real(i) / (W - 1) , color);//compute color
                    std::copy(color, color + stride, rgb + stride * i);//copy to output
                }
                for(size_t j = 1; j < H; j++) std::copy(rgb, rgb + W * stride, rgb + j * W * stride);//copy first row to the rest
            }
        }

        //@brief       : create an rgb or rgba legend for a linear color map
        //@param cyclic: color map function to use
        //@param rgb   : location to write legend image data
        //@param ripple: true/false to apply ripple / create a flat map
        //@param alpha : true/false to include an alpha channel
        //@param WH    : width/height of color bar in pixels
        //@param vFill : value to use for background
        //@param rMin  : inner radius of legend [0,1]
        //@param N     : number of sine waves around half of legend (ignored for ripple = false)
        template <typename Real> void cyclic::legend(cyclic::func<Real> cyclic, Real * const rgb, const bool ripple, const bool alpha, const size_t WH, const Real vFill, const Real rMin, const size_t N) {
            Real color[4] = {0, 0, 0, 1};//set alpha channel to 1
            const size_t stride = alpha ? 4 : 3;//is this an rgb or an rgba image
            for(size_t j = 0; j < WH; j++) {//loop over rows
                const size_t offset = WH * stride * j;
                const Real y = Real(j) / (WH - 1) * 2 - 1;//[-1,1]
                const Real yy = y * y;
                for(size_t i = 0; i < WH; i++) {//loop over columns
                    const Real x = Real(i) / (WH - 1) * 2 - 1;//[-1,1]
                    const Real r = std::sqrt(x * x + yy);//compute radius
                    const size_t idx = offset + stride * i;//compute pixel index
                    if(rMin <= r && r <= Real(1)) {//compute color
                        Real t = std::atan2(y, x) / (M_PI * 2);//[-0.5,0.5]
                        if(std::signbit(t)) t += Real(1);//[0,1]
                        if(ripple) {
                            const Real x = (r - rMin) / (Real(1) - rMin);//compute fractional progress
                            t = detail::testSignal(t, true, N * 2, Real(0.05) * x * x);//apply ripple
                        }
                        cyclic(t, color);//compute color
                        std::copy(color, color + stride, rgb + idx);//copy to output (+ alpha = 1 if needed)
                    } else {//background
                        std::fill(rgb + idx, rgb + idx + stride, 4 == stride ? 0 : vFill);
                    }
                }
            }
        }

        //@brief        : create an rgb or rgba legend for a disk color map
        //@param disk   : color map function to use
        //@param rgb    : location to write legend image data
        //@param w0     : true/false for white/black center
        //@param sym    : type of symmetry to apply
        //@param rRipple: magnitude of ripple in radial direction
        //@param tRipple: magnitude of ripple in theta direction
        //@param alpha  : true/false to include an alpha channel
        //@param WH     : width/height of color bar in pixels
        //@param vFill  : value to use for background
        //@param N      : number of sine waves from r = -1->1 and theta = 0->0.5 (ignored for ripple = false)
        template <typename Real> void disk::legend(disk::func<Real> disk, Real * const rgb, const bool w0, const Sym sym, const Real rRipple, const Real tRipple, const bool alpha, const size_t WH, const Real vFill, const size_t N) {
            Real color[4] = {0, 0, 0, 1};//set alpha channel to 1
            const size_t stride = alpha ? 4 : 3;//is this an rgb or an rgba image
            const bool thetaRipple  = tRipple != Real(0);//does a theta  ripple need to be applied
            const bool radialRipple = rRipple != Real(0);//does a radial ripple need to be applied
            for(size_t j = 0; j < WH; j++) {//loop over rows
                const size_t offset = WH * stride * j;
                const Real y = Real(j) / (WH - 1) * 2 - 1;//[-1,1]
                const Real yy = y * y;
                for(size_t i = 0; i < WH; i++) {//loop over columns
                    const Real x = Real(i) / (WH - 1) * 2 - 1;//[-1,1]
                    Real r = std::sqrt(x * x + yy);//compute radius
                    const size_t idx = offset + stride * i;//compute pixel index
                    if(r <= Real(1)) {//compute color
                        Real t = std::atan2(y, x) / (M_PI * 2);//[-0.5,0.5]
                        if(std::signbit(t)) t += Real(1);//[0,1]
                        if(thetaRipple ) t = detail::testSignal(t, true , N * 2, tRipple);//apply r ripple
                        if(radialRipple) r = detail::testSignal(r, false, N / 2, rRipple);//apply t ripple
                        disk(r, t, color, w0, sym);//compute color
                        std::copy(color, color + stride, rgb + idx);//copy to output
                    } else {//background
                        std::fill(rgb + idx, rgb + idx + stride, 4 == stride ? 0 : vFill);
                    }
                }
            }
        }

        //@brief        : create an rgb or rgba legend for a disk color map
        //@param sphere : color map function to use
        //@param rgb    : location to write legend image data
        //@param nh     : true/false north / south hemisphere
        //@param proj   : type of hemisphere -> disk projection
        //@param w0     : true/false for white/black center
        //@param sym    : type of symmetry to apply
        //@param pRipple: magnitude of ripple in polar direction
        //@param aRipple: magnitude of ripple in azimuthal direction
        //@param alpha  : true/false to include an alpha channel
        //@param WH     : width/height of color bar in pixels
        //@param vFill  : value to use for background
        //@param N      : number of sine waves from p = 0->1 and a = 0->0.5 (ignored for ripple = false)
        template <typename Real> void sphere::legend(sphere::func<Real> sphere, Real * const rgb, const bool nh, const Projection proj, const bool w0, const Sym sym, const Real pRipple, const Real aRipple, const bool alpha, const size_t WH, const Real vFill, const size_t N) {
            //build unprojection function once
            Real(*unproject)(const Real&);
            switch(proj) {
            case Projection::Ortho  : unproject = [](const Real& r)->Real{return std::acos( std::sqrt( Real(1) - r * r ) ) / M_PI                ;}; break;//orthographic projection
            case Projection::Stereo : unproject = [](const Real& r)->Real{return Real(0) == r ? 0 : Real(1) - std::atan( Real(1) / r ) * 2 / M_PI;}; break;//stereographic projection
            case Projection::Lambert: unproject = [](const Real& r)->Real{return Real(1) - std::acos(r / std::sqrt(2)) * Real(2) / M_PI          ;}; break;//lambert equal area modified for pi()/2 projects -> 1
            case Projection::Dist   : unproject = [](const Real& r)->Real{return r / 2                                                           ;}; break;//equal distance
            }

            Real color[4] = {0, 0, 0, 1};//set alpha channel to 1
            const size_t stride = alpha ? 4 : 3;//is this an rgb or an rgba image
            const bool polarRipple   = pRipple != Real(0);//does a theta  ripple need to be applied
            const bool azimuthRipple = aRipple != Real(0);//does a radial ripple need to be applied
            for(size_t j = 0; j < WH; j++) {//loop over rows
                const size_t offset = WH * stride * j;
                const Real y = Real(j) / (WH - 1) * 2 - 1;//[-1,1]
                const Real yy = y * y;
                for(size_t i = 0; i < WH; i++) {//loop over columns
                    const Real x = Real(i) / (WH - 1) * 2 - 1;//[-1,1]
                    const Real r = std::sqrt(x * x + yy);//compute radius
                    const size_t idx = offset + stride * i;//compute pixel index
                    if(r <= Real(1)) {//compute color
                        Real p = unproject(r);
                        if(!nh) p = Real(1) - p;//move to southern hemisphere if needed
                        Real a = std::atan2(y, x) / (M_PI * 2);//azimuthal angle [-0.5,0.5]
                        if(std::signbit(a)) a += Real(1);//azimuthal angle [0,1]
                        if(polarRipple  ) p = detail::testSignal(p, false, N    , pRipple);//apply p ripple
                        if(azimuthRipple) a = detail::testSignal(a, true , N * 2, aRipple);//apply a ripple
                        sphere(p, a, color, w0, sym);//compute color
                        std::copy(color, color + stride, rgb + idx);//copy to output
                    } else {//background
                        std::fill(rgb + idx, rgb + idx + stride, 4 == stride ? 0 : vFill);
                    }
                }
            }
        }

        //@brief        : create an rgb or rgba legend for a ball color map
        //@param ball    : color map function to use
        //@param rgb    : location to write legend image data
        //@param w0     : true/false for white/black center
        //@param sym    : type of symmetry to apply
        //@param rRipple: magnitude of ripple in radial direction
        //@param pRipple: magnitude of ripple in polar direction
        //@param aRipple: magnitude of ripple in azimuthal direction
        //@param alpha  : true/false to include an alpha channel
        //@param WH     : width/height/depth of color bar in pixels
        //@param vFill  : value to use for background
        //@param N      : number of sine waves from p = 0->1 and a = 0->0.5 (ignored for ripple = false)
        template <typename Real> void ball::legend(ball::func<Real> ball, Real * const rgb, const bool w0, const Sym sym, const Real rRipple, const Real pRipple, const Real aRipple, const bool alpha, const size_t WH, const Real vFill, const size_t N) {
            Real color[4] = {0, 0, 0, 1};//set alpha channel to 1
            const size_t stride = alpha ? 4 : 3;//is this an rgb or an rgba image
            const bool radialRipple  = rRipple != Real(0);//does a radial ripple need to be applied
            const bool polarRipple   = pRipple != Real(0);//does a theta  ripple need to be applied
            const bool azimuthRipple = aRipple != Real(0);//does a radial ripple need to be applied
            for(size_t k = 0; k < WH; k++) {//loop over rows
                const Real z = Real(k) / (WH - 1) * 2 - 1;//[-1,1]
                const Real zz = z * z;
                const size_t offsetK = k * WH * WH * stride;//offset to slice start
                for(size_t j = 0; j < WH; j++) {//loop over rows
                    const size_t offset = offsetK + WH * stride * j;
                    const Real y = Real(j) / (WH - 1) * 2 - 1;//[-1,1]
                    const Real yy_zz = y * y + z * z;
                    for(size_t i = 0; i < WH; i++) {//loop over columns
                        const Real x = Real(i) / (WH - 1) * 2 - 1;//[-1,1]
                        const Real r2 = x * x + yy_zz;
                        const size_t idx = offset + stride * i;//compute pixel index
                        if(r2 <= Real(1)) {//compute color
                            //convert from cartesian to fractional sphereical
                            Real r = std::sqrt(r2);
                            Real p = std::acos(z / r) / M_PI;
                            Real a = std::atan2(y, x) / (M_PI * 2);//azimuthal angle [-0.5,0.5]
                            if(std::signbit(a)) a += Real(1);//azimuthal angle [0,1]
                            if(radialRipple ) r = detail::testSignal(r, false, N / 2, rRipple);//apply r ripple
                            if(polarRipple  ) p = detail::testSignal(p, false, N    , pRipple);//apply p ripple
                            if(azimuthRipple) a = detail::testSignal(a, true , N * 2, aRipple);//apply a ripple
                            ball(r, a, p, color, w0, sym);//compute color
                            std::copy(color, color + stride, rgb + idx);//copy to output
                        } else {//background
                            std::fill(rgb + idx, rgb + idx + stride, 4 == stride ? 0 : vFill);
                        }
                    }
                }
            }
        }
    }//namespace colormap

}//namespace lenthe
