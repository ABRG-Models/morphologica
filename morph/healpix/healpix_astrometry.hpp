#pragma once

// This extends the functions in healpix_bare.hpp with code copied from the Astrometry
// codebase.

#include <cmath>
#include <cstdint>
#include <cassert>
#include <algorithm>
#include <stdexcept>
#include <morph/vec.h>
#include <morph/healpix_bare.h> // the t_hpd type is used from healpix_bare.h, as it is essentially the same thing.

namespace hp {
    // The code in namespace hp::am (am for AstroMetry) originates from
    // astrometry.net. Again, slightly modified. I included this code for neighbour
    // relationships. Do I use these now?
    namespace am {

        // Neighbour stuff from astropy/astrometry.net, used with thanks.
        /*
          # This file is part of the Astrometry.net suite.
          # Licensed under a 3-clause BSD style license - see LICENSE_Astrometry_net
        */
        // Astrometry code documentation:
        /**
           In this documentation we talk about "base healpixes": these are the big,
           top-level healpixes.  There are 12 of these, with indices [0, 11].

           We say "fine healpixes" or "healpixes" or "pixels" when we mean the fine-
           scale healpixes; there are Nside^2 of these in each base healpix,
           for a total of 12*Nside^2, indexed from zero.
        */

        /**
           Some notes about the different indexing schemes:

           The healpix paper discusses two different ways to number healpixes, and
           there is a third way, which we prefer, which is (in my opinion) more
           sensible and easy.


           -RING indexing.  Healpixes are numbered first in order of decreasing DEC,
           then in order of increasing RA of the center of the pixel, ie:

           .       0       1       2       3
           .     4   5   6   7   8   9  10  11
           .  12  13  14  15  16  17  18  19
           .    20  21  22  23  24  25  26  27
           .  28  29  30  31  32  33  34  35
           .    36  37  38  39  40  41  42  43
           .      44      45      46      47

           Note that 12, 20 and 28 are part of base healpix 4, as is 27; it "wraps
           around".

           The RING index can be decomposed into the "ring number" and the index
           within the ring (called "longitude index").  Note that different rings
           contain different numbers of healpixes.  Also note that the ring number
           starts from 1, but the longitude index starts from zero.


           -NESTED indexing.  This only works for Nside parameters that are powers of
           two.  This scheme is hierarchical in the sense that each pair of bits of
           the index tells you where the pixel center is to finer and finer
           resolution.  It doesn't really show with Nside=2, but here it is anyway:

           .       3       7      11      15
           .     2   1   6   5  10   9  14  13
           .  19   0  23   4  27   8  31  12
           .    17  22  21  26  25  30  29  18
           .  16  35  20  39  24  43  28  47
           .    34  33  38  37  42  41  46  45
           .      32      36      40      44

           Note that all the base healpixes have the same pattern; they're just
           offset by factors of Nside^2.

           Here's a zoom-in of the first base healpix, turned 45 degrees to the
           right, for Nside=4:

           .   10  11  14  15
           .    8   9  12  13
           .    2   3   6   7
           .    0   1   4   5

           Note that the bottom-left block of 4 have the smallest values, and within
           that the bottom-left corner has the smallest value, followed by the
           bottom-right, top-left, then top-right.

           The NESTED index can't be decomposed into 'orthogonal' directions.


           -XY indexing.  This is arguably the most natural, at least for the
           internal usage of the healpix code.  Within each base healpix, the
           healpixes are numbered starting with 0 for the southmost pixel, then
           increasing first in the "y" (north-west), then in the "x" (north-east)
           direction.  In other words, within each base healpix there is a grid
           and we number the pixels "lexicographically" (mod a 135 degree turn).

           .       3       7      11      15
           .     1   2   5   6   9  10  13  14
           .  19   0  23   4  27   8  31  12
           .    18  21  22  25  26  29  30  17
           .  16  35  20  39  24  43  28  47
           .    33  34  37  38  41  42  45  46
           .      32      36      40      44

           Zooming in on the first base healpix, turning 45 degrees to the right,
           for Nside=4 we get:

           .    3   7  11  15
           .    2   6  10  14
           .    1   5   9  13
           .    0   4   8  12

           Notice that the numbers first increase from bottom to top (y), then left to
           right (x).

           The XY indexing can be decomposed into 'x' and 'y' coordinates
           (in case that wasn't obvious), where the above figure becomes (x,y):

           .    (0,3)  (1,3)  (2,3)  (3,3)
           .    (0,2)  (1,2)  (2,2)  (3,2)
           .    (0,1)  (1,1)  (2,1)  (3,1)
           .    (0,0)  (1,0)  (2,0)  (3,0)

           Note that "x" increases in the north-east direction, and "y" increases in
           the north-west direction.

           The major advantage to this indexing scheme is that it extends to
           fractional coordinates in a natural way: it is meaningful to talk about
           the position (x,y) = (0.25, 0.6) and you can compute its position.

           In this code, all healpix indexing uses the XY scheme.  If you want to
           use the other schemes you will have to use the conversion routines:
           .   xy_to_ring
           .   ring_to_xy
           .   xy_to_nested
           .   nested_to_xy
        */

        // This composes the 'xy index' from f (face/base patch), x and y.
        int64_t compose_xy (int32_t f, int64_t x, int64_t y, int64_t Nside)
        {
            assert(Nside > 0);
            assert(f >= 0);
            assert(f < 12);
            assert(x >= 0);
            assert(x < Nside);
            assert(y >= 0);
            assert(y < Nside);
            return (( (Nside * f) + x) * Nside) + y;
        }

        // Convert from discrete {face, x, y} struct to xy index
        static int64_t hpd_to_xy (hp::t_hpd hp, int64_t Nside)
        {
            return hp::am::compose_xy (hp.f, hp.x, hp.y, Nside);
        }

        // Decompose the xy index hpxy into pointers to face, x and y (as used in type t_hpd)
        void decompose_xy (int64_t hpxy, int32_t* pf, int64_t* px, int64_t* py, int64_t Nside)
        {
            assert (Nside > 0);
            int64_t hp = 0;
            int64_t ns2 = Nside * Nside;
            assert (hpxy < (ns2 * 12));
            assert (hpxy >= 0);
            if (pf) {
                int32_t f = (int32_t)(hpxy / ns2);
                assert(f >= 0);
                assert(f < 12);
                *pf = f;
            }
            hp = hpxy % ns2;
            if (px) {
                *px = hp / Nside;
                assert(*px >= 0);
                assert(*px < Nside);
            }
            if (py) {
                *py = hp % Nside;
                assert(*py >= 0);
                assert(*py < Nside);
            }
        }

        static uint32_t my_hweight32 (uint32_t w)
        {
            uint32_t res = (w & 0x55555555) + ((w >> 1) & 0x55555555);
            res = (res & 0x33333333) + ((res >> 2) & 0x33333333);
            res = (res & 0x0F0F0F0F) + ((res >> 4) & 0x0F0F0F0F);
            res = (res & 0x00FF00FF) + ((res >> 8) & 0x00FF00FF);
            return (res & 0x0000FFFF) + ((res >> 16) & 0x0000FFFF);
        }
        bool is_power_of_two(uint32_t x) { return (my_hweight32(x) == 1 ? true : false); }

        int64_t xy_to_nested (int64_t hpxy, int64_t Nside)
        {
            if (hpxy < 0 || Nside < 0) { return -1; }

            int64_t ns2 = Nside * Nside;
            int32_t f = 0;
            int64_t x = 0, y = 0;
            hp::am::decompose_xy (hpxy, &f, &x, &y, Nside);
            if (!hp::am::is_power_of_two (Nside)) {
                fprintf(stderr, "healpix_xy_to_nested: Nside must be a power of two.\n");
                return -1;
            }

            // We construct the index called p_n' in the healpix paper, whose bits
            // are taken from the bits of x and y:
            //    x = ... b4 b2 b0
            //    y = ... b5 b3 b1
            // We go through the bits of x,y, building up "index":
            int64_t index = 0;
            for (size_t i=0u; i<(8u*sizeof(int64_t)/2u); i++) {
                index |= (int64_t)(((y & 1) << 1) | (x & 1)) << (i*2);
                y >>= 1;
                x >>= 1;
                if (!x && !y) { break; }
            }

            return index + (int64_t)f * ns2;
        }

        // Convert nested index to the xy index.
        int64_t nested_to_xy (int64_t hpnest, int64_t Nside)
        {
            if (hpnest < 0 || Nside < 0) { return -1; }

            if (!hp::am::is_power_of_two (Nside)) {
                fprintf(stderr, "xy_to_nested: Nside must be a power of two.\n");
                return -1;
            }

            int64_t ns2 = Nside * Nside;
            int f = (int)(hpnest / ns2);
            // index is p' from https://arxiv.org/abs/astro-ph/0409513v1. This is the index within the patch.
            int64_t index = hpnest % ns2;
            int64_t x = 0;
            int64_t y = 0;
            for (size_t i = 0u; i < (8u * sizeof(int64_t) / 2u); i++) {
                x |= (index & 0x1) << i;
                index >>= 1;
                y |= (index & 0x1) << i;
                index >>= 1;
                if (!index) { break; }
            }
            return hp::am::compose_xy (f, x, y, Nside);
        }

        // Decompose the ring index hpring into p_ring and p_longind. Whatever they are.
        void decompose_ring (int64_t hpring, int64_t Nside, int* p_ring, int* p_longind)
        {
            int64_t longind;
            int64_t offset = 0;
            int64_t ns2;
            int ring;
            double x;
            ns2 = Nside * Nside;
            if (hpring < 2 * ns2) {
                ring = (int)(0.5 + sqrt(0.25 + 0.5 * hpring));
                offset = 2 * (int64_t)ring * ((int64_t)ring - 1);
                // The sqrt above can introduce precision issues that can cause ring to
                // be off by 1, so we check whether the offset is now larger than the HEALPix
                // value, and if so we need to adjust ring and offset accordingly
                if (offset > hpring) {
                    ring -= 1;
                    offset = 2 * (int64_t)ring * ((int64_t)ring - 1);
                }
                longind = hpring - offset;
            } else {
                offset = 2 * Nside * (Nside - 1);
                if (hpring < 10 * ns2) {
                    ring = (int)((hpring - offset) / ((int64_t)Nside * 4) + (int64_t)Nside);
                    offset += 4 * (ring - Nside) * Nside;
                    longind = hpring - offset;
                } else {
                    offset += 8 * ns2;
                    x = (2 * Nside + 1 - sqrt((2 * Nside + 1) * (2 * Nside + 1) - 2 * (hpring - offset)))*0.5;
                    ring = (int)x;
                    offset += 2 * (int64_t)ring * (2 * Nside + 1 - (int64_t)ring);
                    // The sqrt above can introduce precision issues that can cause ring to
                    // be off by 1, so we check whether the offset is now larger than the HEALPix
                    // value, and if so we need to adjust ring and offset accordingly
                    if (offset > hpring) {
                        ring -= 1;
                        offset -= 4 * Nside - 4 * (int64_t)ring;
                    }
                    longind = (int)(hpring - offset);
                    ring += 3 * Nside;
                }
            }
            if (p_ring != nullptr) { *p_ring = ring; }
            if (p_longind != nullptr) { *p_longind = (int)longind; }
        }

        int64_t ring_to_xy (int64_t hpring, int64_t Nside)
        {
            int f = 0;
            int64_t x = 0, y = 0;
            int ringind = 0, longind = 0;
            hp::am::decompose_ring (hpring, Nside, &ringind, &longind);
            if (hpring < 0 || Nside < 0) {
                return -1;
            } else if (ringind <= Nside) {
                f = longind / ringind;
                int64_t ind = (int64_t)longind - (int64_t)f * (int64_t)ringind;
                y = (Nside - 1) - ind;
                int frow = f / 4;
                int F1 = frow + 2;
                int v = F1*Nside - ringind - 1;
                x = v - y;
                return hp::am::compose_xy(f, x, y, Nside);
            } else if (ringind < 3*Nside) {
                int f = -1;
                int R = 0;
                int panel = longind / Nside;
                int ind = longind % Nside;
                int bottomleft = ind < (ringind - Nside + 1) / 2;
                int topleft = ind < ((int64_t)3*Nside - ringind + 1)/2;

                if (!bottomleft && topleft) {
                    // top row.
                    f = panel;
                } else if (bottomleft && !topleft) {
                    // bottom row.
                    f = 8 + panel;
                } else if (bottomleft && topleft) {
                    // left side.
                    f = 4 + panel;
                } else if (!bottomleft && !topleft) {
                    // right side.
                    f = 4 + (panel + 1) % 4;
                    if (f == 4) {
                        longind -= ((int64_t)4*Nside - 1);
                        // Gah!  Wacky hack - it seems that since
                        // "longind" is negative in this case, the
                        // rounding behaves differently, so we end up
                        // computing the wrong "h" and have to correct
                        // for it.
                        R = 1;
                    }
                }

                int frow = f / 4;
                int F1 = frow + 2;
                int F2 = 2*(f % 4) - (frow % 2) + 1;
                int s = (ringind - Nside) % 2;
                int v = F1*Nside - ringind - 1;
                int h = 2*longind - s - F2*Nside;
                if (R) { h--; }
                x = (v + h) / 2;
                y = (v - h) / 2;

                if ((v != (x+y)) || (h != (x-y))) {
                    h++;
                    x = (v + h) / 2;
                    y = (v - h) / 2;

                    if ((v != (x+y)) || (h != (x-y))) {
                        throw std::runtime_error ("Unexpected case");
                    }
                }
                return hp::am::compose_xy (f, x, y, Nside);
            } else {
                int ri = 4 * Nside - ringind;
                f = 8 + longind / ri;
                int ind = longind - (f%4) * ri;
                y = (ri-1) - ind;
                int frow = f / 4;
                int F1 = frow + 2;
                int v = F1 * Nside - ringind - 1;
                x = v - y;
                return hp::am::compose_xy (f, x, y, Nside);
            }
        }

        int64_t xy_to_ring (int64_t hpxy, int64_t Nside)
        {
            int32_t f = 0;
            int64_t x = 0, y = 0;

            hp::am::decompose_xy (hpxy, &f, &x, &y, Nside);
            int frow = f / 4;
            int F1 = frow + 2;
            int v = x + y;
            // "ring" starts from 1 at the north pole and goes to 4Nside-1 at
            // the south pole; the pixels in each ring have the same latitude.
            int ring = F1 * Nside - v - 1;
            /*
              ring:
              [1, Nside] : n pole
              (Nside, 2Nside] : n equatorial
              (2Nside+1, 3Nside) : s equat
              [3Nside, 4Nside-1] : s pole
            */
            // this probably can't happen (it's an invalid ring index)
            if ((ring < 1) || (ring >= 4*Nside)) { return -1; }

            int64_t index = 0;
            if (ring <= Nside) {
                // north polar.
                // left-to-right coordinate within this healpix
                index = (Nside - 1 - y);
                // offset from the other big healpixes
                index += ((f % 4) * ring);
                // offset from the other rings
                index += (int64_t)ring*(ring-1)*2;
            } else if (ring >= 3*Nside) {
                // south polar.
                // Here I first flip everything so that we label the pixels
                // at zero starting in the southeast corner, increasing to the
                // west and north, then subtract that from the total number of
                // healpixels.
                int ri = 4*Nside - ring;
                // index within this healpix
                index = (ri-1) - x;
                // big healpixes
                index += ((3-(f % 4)) * ri);
                // other rings
                index += (int64_t)ri*(ri-1)*2;
                // flip!
                index = 12*(int64_t)Nside*Nside - 1 - index;

            } else {
                // equatorial.
                int64_t s = (ring - Nside) % 2;
                int64_t F2 = 2*((int)f % 4) - (frow % 2) + 1;
                int64_t h = x - y;

                index = (F2 * Nside + h + s) / 2;
                // offset from the north polar region:
                index += Nside * (Nside - 1) * 2;
                // offset within the equatorial region:
                index += Nside * 4 * (ring - Nside);
                // handle healpix #4 wrap-around
                if ((f == 4) && (y > x)) { index += (4 * Nside - 1); }
            }
            return index;
        }

        static void xy_to_hpd (int64_t hpxy, hp::t_hpd* hp, int64_t Nside)
        {
            hp::am::decompose_xy (hpxy, &hp->f, &hp->x, &hp->y, Nside);
        }

        // the north polar healpixes are 0,1,2,3; the south polar healpixes are 8,9,10,11
        static bool ispolar (int f) { return (f <= 3) || (f >= 8); }
        // the north polar healpixes are 0,1,2,3; the south polar healpixes are 8,9,10,11
        static bool isequatorial (int f) { return (f >= 4) && (f <= 7); }
        static bool isnorthpolar (int f) { return (f <= 3); }
        static bool issouthpolar (int f) { return (f >= 8); }

        /**
           Given a large-scale healpix number, computes its neighbour in the
           direction (dx,dy).  Returns -1 if there is no such neighbour.
        */
        static int get_neighbour (int f, int dx, int dy)
        {
            if (hp::am::isnorthpolar(f)) {
                if ((dx ==  1) && (dy ==  0)) { return (f + 1) % 4; }
                if ((dx ==  0) && (dy ==  1)) { return (f + 3) % 4; }
                if ((dx ==  1) && (dy ==  1)) { return (f + 2) % 4; }
                if ((dx == -1) && (dy ==  0)) { return (f + 4); }
                if ((dx ==  0) && (dy == -1)) { return 4 + ((f + 1) % 4); }
                if ((dx == -1) && (dy == -1)) { return f + 8; }
                return -1;
            } else if (hp::am::issouthpolar(f)) {
                if ((dx ==  1) && (dy ==  0)) { return 4 + ((f + 1) % 4); }
                if ((dx ==  0) && (dy ==  1)) { return f - 4; }
                if ((dx == -1) && (dy ==  0)) { return 8 + ((f + 3) % 4); }
                if ((dx ==  0) && (dy == -1)) { return 8 + ((f + 1) % 4); }
                if ((dx == -1) && (dy == -1)) { return 8 + ((f + 2) % 4); }
                if ((dx ==  1) && (dy ==  1)) { return f - 8; }
                return -1;
            } else {
                if ((dx ==  1) && (dy ==  0)) { return f - 4; }
                if ((dx ==  0) && (dy ==  1)) { return (f + 3) % 4; }
                if ((dx == -1) && (dy ==  0)) { return 8 + ((f + 3) % 4); }
                if ((dx ==  0) && (dy == -1)) { return f + 4; }
                if ((dx ==  1) && (dy == -1)) { return 4 + ((f + 1) % 4); }
                if ((dx == -1) && (dy ==  1)) { return 4 + ((f - 1) % 4); }
                return -1;
            }
            return -1;
        }

        static void get_neighbours (t_hpd hp, t_hpd* neighbour, int64_t Nside)
        {
            int32_t base = hp.f;
            int64_t x = hp.x;
            int64_t y = hp.y;

            // ( + , 0 )
            int64_t nx = (x + 1) % Nside;
            int64_t ny = y;
            int32_t nbase = 0;
            if (x == (Nside - 1)) {
                nbase = hp::am::get_neighbour (base, 1, 0);
                if (hp::am::isnorthpolar (base)) {
                    nx = x;
                    std::swap (nx, ny);
                }
            } else {
                nbase = base;
            }

            neighbour[0].f = nbase;
            neighbour[0].x = nx;
            neighbour[0].y = ny;

            // ( + , + )
            nx = (x + 1) % Nside;
            ny = (y + 1) % Nside;
            if ((x == Nside - 1) && (y == Nside - 1)) {
                if (ispolar(base)) {
                    nbase = hp::am::get_neighbour (base, 1, 1);
                } else {
                    nbase = -1;
                }
            } else if (x == (Nside - 1)) {
                nbase = hp::am::get_neighbour (base, 1, 0);
            } else if (y == (Nside - 1)) {
                nbase = hp::am::get_neighbour (base, 0, 1);
            } else {
                nbase = base;
            }

            if (hp::am::isnorthpolar (base)) {
                if (x == (Nside - 1)) { nx = Nside - 1; }
                if (y == (Nside - 1)) { ny = Nside - 1; }
                if ((x == (Nside - 1)) || (y == (Nside - 1))) { std::swap (nx, ny); }
            }

            neighbour[1].f = nbase;
            neighbour[1].x = nx;
            neighbour[1].y = ny;

            // ( 0 , + )
            nx = x;
            ny = (y + 1) % Nside;
            if (y == (Nside - 1)) {
                nbase = hp::am::get_neighbour (base, 0, 1);
                if (hp::am::isnorthpolar (base)) {
                    ny = y;
                    std::swap (nx, ny);
                }
            } else {
                nbase = base;
            }

            neighbour[2].f = nbase;
            neighbour[2].x = nx;
            neighbour[2].y = ny;

            // ( - , + )
            nx = (x + Nside - 1) % Nside;
            ny = (y + 1) % Nside;
            if ((x == 0) && (y == (Nside - 1))) {
                if (hp::am::isequatorial (base)) {
                    nbase = hp::am::get_neighbour (base, -1, 1);
                } else {
                    nbase = -1;
                }
            } else if (x == 0) {
                nbase = hp::am::get_neighbour (base, -1, 0);
                if (hp::am::issouthpolar (base)) {
                    nx = 0;
                    std::swap (nx, ny);
                }
            } else if (y == (Nside - 1)) {
                nbase = hp::am::get_neighbour (base, 0, 1);
                if (hp::am::isnorthpolar (base)) {
                    ny = y;
                    std::swap (nx, ny);
                }
            } else {
                nbase = base;
            }

            neighbour[3].f = nbase;
            neighbour[3].x = nx;
            neighbour[3].y = ny;

            // ( - , 0 )
            nx = (x + Nside - 1) % Nside;
            ny = y;
            if (x == 0) {
                nbase = hp::am::get_neighbour (base, -1, 0);
                if (hp::am::issouthpolar (base)) {
                    nx = 0;
                    std::swap (nx, ny);
                }
            } else {
                nbase = base;
            }

            neighbour[4].f = nbase;
            neighbour[4].x = nx;
            neighbour[4].y = ny;

            // ( - , - )
            nx = (x + Nside - 1) % Nside;
            ny = (y + Nside - 1) % Nside;
            if ((x == 0) && (y == 0)) {
                if (hp::am::ispolar(base)) {
                    nbase = hp::am::get_neighbour (base, -1, -1);
                } else {
                    nbase = -1;
                }
            } else if (x == 0) {
                nbase = hp::am::get_neighbour (base, -1, 0);
            } else if (y == 0) {
                nbase = hp::am::get_neighbour (base, 0, -1);
            } else {
                nbase = base;
            }

            if (hp::am::issouthpolar(base)) {
                if (x == 0) { nx = 0; }
                if (y == 0) { ny = 0; }
                if ((x == 0) || (y == 0)) { std::swap (nx, ny); }
            }

            neighbour[5].f = nbase;
            neighbour[5].x = nx;
            neighbour[5].y = ny;

            // ( 0 , - )
            ny = (y + Nside - 1) % Nside;
            nx = x;
            if (y == 0) {
                nbase = hp::am::get_neighbour (base, 0, -1);
                if (hp::am::issouthpolar (base)) {
                    ny = y;
                    std::swap (nx, ny);
                }
            } else {
                nbase = base;
            }

            neighbour[6].f = nbase;
            neighbour[6].x = nx;
            neighbour[6].y = ny;

            // ( + , - )
            nx = (x + 1) % Nside;
            ny = (y + Nside - 1) % Nside;
            if ((x == (Nside - 1)) && (y == 0)) {
                if (hp::am::isequatorial (base)) {
                    nbase = hp::am::get_neighbour (base, 1, -1);
                } else {
                    nbase = -1;
                }
            } else if (x == (Nside - 1)) {
                nbase = hp::am::get_neighbour (base, 1, 0);
                if (hp::am::isnorthpolar (base)) {
                    nx = x;
                    std::swap (nx, ny);
                }
            } else if (y == 0) {
                nbase = hp::am::get_neighbour (base, 0, -1);
                if (hp::am::issouthpolar (base)) {
                    ny = y;
                    std::swap (nx, ny);
                }
            } else {
                nbase = base;
            }

            neighbour[7].f = nbase;
            neighbour[7].x = nx;
            neighbour[7].y = ny;
        }

        // For the given pixel in nested index format, return a morph::vec of
        // neighbours also in nested index format.
        void get_neighbours (int64_t hpnest, morph::vec<int64_t, 8>& neighbour, int64_t Nside)
        {
            t_hpd neigh[8];
            t_hpd hp = {};
            int64_t pix_xy = hp::am::nested_to_xy (hpnest, Nside);
            hp::am::xy_to_hpd (pix_xy, &hp, Nside);
            //std::cout << "nested pixel " << hpnest << " which is pix_xy=" << pix_xy
            //          << " becomes [x:" << hp.x << " y:" << hp.y << " f" << hp.f << "]" << std::endl;
            hp::am::get_neighbours (hp, neigh, Nside);
            neighbour.set_from (-1); // init to all unknown
            for (int i=0; i < 8; i++) {
                if (neigh[i].f >= 0) {
                    neighbour[i] = hp::am::xy_to_nested (hp::am::hpd_to_xy (neigh[i], Nside), Nside);
                }
            }
        }

        // Get neighbours using the ring indexing language
        void get_neighbours_ring (int64_t hpring, morph::vec<int64_t, 8>& neighbour, int64_t Nside)
        {
            t_hpd neigh[8];
            t_hpd hp = {};
            int64_t pix_xy = hp::am::ring_to_xy (hpring, Nside);
            hp::am::xy_to_hpd (pix_xy, &hp, Nside);
            hp::am::get_neighbours (hp, neigh, Nside);
            neighbour.set_from (-1);
            for (int i=0; i < 8; i++) {
                if (neigh[i].f >= 0) {
                    neighbour[i] = hp::am::xy_to_ring (hp::am::hpd_to_xy (neigh[i], Nside), Nside);
                }
            }
        }
    } // namespace am (for astrometry)
} // hp
