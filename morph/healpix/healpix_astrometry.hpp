/*
 # This file is part of the Astrometry.net suite.
 # Licensed under a 3-clause BSD style license - see LICENSE
 *
 * Modified/namespaced for C++ compilation by Seb James
 */
#pragma once

#include <morph/mathconst.h>
#include <algorithm>
#include <cmath>
#include <cassert>
#include <stdio.h>
#include <cstdint>

/**
   The HEALPix paper is here:
   http://iopscience.iop.org/0004-637X/622/2/759/pdf/0004-637X_622_2_759.pdf
   See:
   http://adsabs.harvard.edu/cgi-bin/nph-bib_query?bibcode=2005ApJ...622..759G&db_key=AST&high=41069202cf02947
*/

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
   .   healpix_xy_to_ring
   .   healpix_ring_to_xy
   .   healpix_xy_to_nested
   .   healpix_nested_to_xy
*/

namespace hp { // Healpix
    namespace am { // Astrometry

        // The maximum healpix Nside that leads to int-sized healpix indices.
        // 12 * (13377+1)^2 > 2^31 (since we use signed ints)
        // This corresponds to about 16 arcsec side length.
        static constexpr int HP_MAX_INT_NSIDE = 13377;

        /**
           Converts a healpix index from the XY scheme to the RING scheme.
        */
        const int healpix_xy_to_ring(int hp, int Nside);

        /**
           Converts a healpix index from the RING scheme to the XY scheme.
        */
        const int healpix_ring_to_xy(int ring_index, int Nside);

        /**
           Converts a healpix index from the XY scheme to the NESTED scheme.
        */
        const int healpix_xy_to_nested(int hp, int Nside);

        /**
           Converts a healpix index from the NESTED scheme to the XY scheme.
        */
        const int healpix_nested_to_xy(int nested_index, int Nside);

        /**
           Decomposes a RING index into the "ring number" (each ring contain
           healpixels of equal latitude) and "longitude index".  Pixels within a
           ring have longitude index starting at zero for the first pixel with
           RA >= 0.  Different rings contain different numbers of healpixels.
        */
        void healpix_decompose_ring(int ring_index, int Nside,
                                    int* p_ring_number, int* p_longitude_index);

        /**
           Composes a RING index given the "ring number" and "longitude index".

           Does NOT check that the values are legal!  Garbage in, garbage out.
        */
        const int healpix_compose_ring(int ring, int longind, int Nside);

        /**
           Decomposes an XY index into the "base healpix" and "x" and "y" coordinates
           within that healpix.
        */
        void healpix_decompose_xy(int finehp, int* bighp, int* x, int* y, int Nside);

        void healpix_decompose_xyl(int64_t finehp, int* bighp, int* x, int* y, int Nside);

        /**
           Composes an XY index given the "base healpix" and "x" and "y" coordinates
           within that healpix.
        */
        const int healpix_compose_xy(int bighp, int x, int y, int Nside);

        const int64_t healpix_compose_xyl(int bighp, int x, int y, int Nside);

        /**
           Given (x,y) coordinates of resolution "nside" within a base-level
           healpixel, and an output resolution "outnside", returns the output
           (x,y) coordinates at the output resolution.
        */
        void healpix_convert_xy_nside(int x, int y, int nside, int outnside,
                                      int* outx, int* outy);

        /**
           Given a healpix index (in the XY scheme) of resolution "nside", and
           an output resolution "outnside", returns the healpix index at the
           output resolution.
        */
        void healpix_convert_nside(int hp, int nside, int outnside, int* outhp);
        void healpix_convert_nsidel(int64_t hp, int nside, int outnside, int64_t* outhp);

        /**
           Converts (RA, DEC) coordinates (in radians) to healpix index.
        */
        const int radectohealpix(double ra, double dec, int Nside);

        int radectohealpixf(double ra, double dec, int Nside, double* dx, double* dy);

        const int64_t radectohealpixl(double ra, double dec, int Nside);

        int64_t radectohealpixlf(double ra, double dec, int Nside, double* dx, double* dy);

        /**
           Converts (RA, DEC) coordinates (in degrees) to healpix index.
        */
        const int radecdegtohealpix(double ra, double dec, int Nside);

        int radecdegtohealpixf(double ra, double dec, int Nside, double* dx, double* dy);

        const int64_t radecdegtohealpixl(double ra, double dec, int Nside);

        int64_t radecdegtohealpixlf(double ra, double dec, int Nside, double* dx, double* dy);

        /**
           Converts (x,y,z) coordinates on the unit sphere into a healpix index.
        */
        const int xyztohealpix(double x, double y, double z, int Nside);

        const int64_t xyztohealpixl(double x, double y, double z, int Nside);

        int xyztohealpixf(double x, double y, double z, int Nside,
                          double* p_dx, double* p_dy);

        int64_t xyztohealpixlf(double x, double y, double z, int Nside,
                               double* p_dx, double* p_dy);

        /**
           Converts (x,y,z) coordinates (stored in an array) on the unit sphere into
           a healpix index.
        */
        int xyzarrtohealpix(const double* xyz, int Nside);

        int64_t xyzarrtohealpixl(const double* xyz, int Nside);

        int xyzarrtohealpixf(const double* xyz,int Nside, double* p_dx, double* p_dy);

        /**
           Converts a healpix index, plus fractional offsets (dx,dy), into (x,y,z)
           coordinates on the unit sphere.  (dx,dy) must be in [0, 1].  (0.5, 0.5)
           is the center of the healpix.  (0,0) is the southernmost corner, (1,1) is
           the northernmost corner, (1,0) is the easternmost, and (0,1) the
           westernmost.
        */
        void healpix_to_xyz(int hp, int Nside, double dx, double dy,
                            double* p_x, double *p_y, double *p_z);

        /**
           Same as healpix_to_xyz, but (x,y,z) are stored in an array.
        */
        void healpix_to_xyzarr(int hp, int Nside, double dx, double dy,
                               double* xyz);

        void healpixl_to_xyzarr(int64_t hp, int Nside, double dx, double dy,
                                double* xyz);


        /**
           Same as healpix_to_xyz, but returns (RA,DEC) in radians.
        */
        void healpix_to_radec(int hp, int Nside, double dx, double dy,
                              double* ra, double* dec);

        void healpix_to_radecdeg(int hp, int Nside, double dx, double dy,
                                 double* ra, double* dec);

        void healpixl_to_radecdeg(int64_t hp, int Nside, double dx, double dy,
                                  double* ra, double* dec);

        /**
           Same as healpix_to_radec, but (RA,DEC) are stored in an array.
        */
        void healpix_to_radecarr(int hp, int Nside, double dx, double dy,
                                 double* radec);

        void healpix_to_radecdegarr(int hp, int Nside, double dx, double dy,
                                    double* radec);

        /**
           Computes the approximate side length of a healpix, in arcminutes.
        */
        const double healpix_side_length_arcmin(int Nside);

        /**
           Computes the approximate Nside you need to get healpixes with side
           length about "arcmin" arcminutes.  (inverse of
           healpix_side_length_arcmin)
        */
        double healpix_nside_for_side_length_arcmin(double arcmin);

        /**
           Finds the healpixes neighbouring the given healpix, placing them in the
           array "neighbour".  Returns the number of neighbours.  You must ensure
           that "neighbour" has 8 elements.

           Healpixes in the interior of a large healpix will have eight neighbours;
           pixels near the edges can have fewer.
        */
        int healpix_get_neighbours(int hp, int* neighbours, int Nside);

        /**
           Same as above, but for Nsides big enough that it overflows 32-bit int.
        */
        int healpix_get_neighboursl(int64_t pix, int64_t* neighbour, int Nside);

        /**
           Finds the healpixes containing and neighbouring the given xyz
           position which are within distance 'range' (in units of distance of
           the unit sphere).  Places the results in 'healpixes', which must have
           at least 9 elements.  Returns the number of 'healpixes' set.

           Returns -1 if "Nside" < 0.
        */
        int healpix_get_neighbours_within_range(double* xyz, double range, int* healpixes,
                                                int Nside);

        /**
           Same as above, but RA,Dec,radius in degrees.
        */
        int healpix_get_neighbours_within_range_radec(double ra, double dec, double radius,
                                                      int* healpixes, int Nside);

        /**
           Returns the minimum distance (in degrees) between the given healpix
           and the given RA,Dec (in degrees).
        */
        double healpix_distance_to_radec(int hp, int Nside, double ra, double dec,
                                         double* closestradec);

#ifdef ENABLE_FUNCTIONS_REQUIRING_QSORT
        /**
           Returns the minimum distance (in degrees) between the given healpix
           and the given xyz (point on unit sphere).
        */
        double healpix_distance_to_xyz(int hp, int Nside, const double* xyz,
                                       double* closestxyz);

        /**
           Returns true if the closest distance between the given healpix and
           the given RA,Dec (in degrees) is less than then given radius (in degrees).
        */
        int healpix_within_range_of_radec(int hp, int Nside, double ra, double dec,
                                          double radius);
        int healpix_within_range_of_xyz(int hp, int Nside, const double* xyz,
                                        double radius);


        /**
           Computes the RA,Dec bounding-box of the given healpix.  Results are
           in degrees.  RA may be wacky for healpixes spanning RA=0.
        */
        void healpix_radec_bounds(int hp, int nside,
                                  double* ralo, double* rahi,
                                  double* declo, double* dechi);
#endif
    }
}

namespace hp {
    namespace am {

        // "borrowed" from <linux/bitops.h> from linux-2.4
        static unsigned int my_hweight32(unsigned int w) {
            unsigned int res = (w & 0x55555555) + ((w >> 1) & 0x55555555);
            res = (res & 0x33333333) + ((res >> 2) & 0x33333333);
            res = (res & 0x0F0F0F0F) + ((res >> 4) & 0x0F0F0F0F);
            res = (res & 0x00FF00FF) + ((res >> 8) & 0x00FF00FF);
            return (res & 0x0000FFFF) + ((res >> 16) & 0x0000FFFF);
        }

        int is_power_of_two(unsigned int x) {
            return (my_hweight32(x) == 1);
        }

        // Internal type
        struct hp_s {
            int bighp;
            int x;
            int y;
        };
        typedef struct hp_s hp_t;

        static int64_t hptointl(hp_t hp, int Nside) {
            return healpix_compose_xyl(hp.bighp, hp.x, hp.y, Nside);
        }

        static int hptoint(hp_t hp, int Nside) {
            return healpix_compose_xy(hp.bighp, hp.x, hp.y, Nside);
        }

        static void intltohp(int64_t pix, hp_t* hp, int Nside) {
            healpix_decompose_xyl(pix, &hp->bighp, &hp->x, &hp->y, Nside);
        }

        static void inttohp(int pix, hp_t* hp, int Nside) {
            healpix_decompose_xy(pix, &hp->bighp, &hp->x, &hp->y, Nside);
        }
        static void longtohp(int64_t pix, hp_t* hp, int Nside) {
            healpix_decompose_xyl(pix, &hp->bighp, &hp->x, &hp->y, Nside);
        }

        static void hp_decompose(hp_t* hp, int* php, int* px, int* py) {
            if (php) { *php = hp->bighp; }
            if (px) { *px = hp->x; }
            if (py) { *py = hp->y; }
        }

        const static double mysquare(double d) { return d*d; }

        const int healpix_xy_to_nested(int hp, int Nside) {
            int bighp,x,y;
            int index;
            int i;

            healpix_decompose_xy(hp, &bighp, &x, &y, Nside);
            if (!is_power_of_two(Nside)) {
                fprintf(stderr, "healpix_xy_to_nested: Nside must be a power of two.\n");
                return -1;
            }

            // We construct the index called p_n' in the healpix paper, whose bits
            // are taken from the bits of x and y:
            //    x = ... b4 b2 b0
            //    y = ... b5 b3 b1
            // We go through the bits of x,y, building up "index":
            index = 0;
            for (i=0; i<(8*static_cast<int>(sizeof(int))/2); i++) {
                index |= (((y & 1) << 1) | (x & 1)) << (i*2);
                y >>= 1;
                x >>= 1;
                if (!x && !y) break;
            }

            return index + bighp * Nside * Nside;
        }

        const int healpix_nested_to_xy(int hp, int Nside) {
            int bighp, x, y;
            int index;
            int i;
            if (!is_power_of_two(Nside)) {
                fprintf(stderr, "healpix_xy_to_nested: Nside must be a power of two.\n");
                return -1;
            }
            bighp = hp / (Nside*Nside);
            index = hp % (Nside*Nside);
            x = y = 0;
            for (i=0; i<(8*static_cast<int>(sizeof(int))/2); i++) {
                x |= (index & 0x1) << i;
                index >>= 1;
                y |= (index & 0x1) << i;
                index >>= 1;
                if (!index) break;
            }
            return healpix_compose_xy(bighp, x, y, Nside);
        }

        const int healpix_compose_ring(int ring, int longind, int Nside) {
            if (ring <= Nside)
                // north polar
                return ring * (ring-1) * 2 + longind;
            if (ring < 3*Nside)
                // equatorial
                return Nside*(Nside-1)*2 + Nside*4*(ring-Nside) + longind;
            {
                int ri;
                ri = 4*Nside - ring;
                return 12*Nside*Nside-1 - ( ri*(ri-1)*2 + (ri*4 - 1 - longind) );
            }
        }

        void healpix_decompose_ring(int hp, int Nside, int* p_ring, int* p_longind) {
            // FIXME: this could be written in closed form...
            int longind;
            int ring;
            int offset = 0;
            for (ring=1; ring<=Nside; ring++) {
                if (offset + ring*4 > hp) {
                    longind = hp - offset;
                    goto gotit;
                }
                offset += ring*4;
            }
            for (; ring<(3*Nside); ring++) {
                if (offset + Nside*4 > hp) {
                    longind = hp - offset;
                    goto gotit;
                }
                offset += Nside*4;
            }
            for (; ring<(4*Nside); ring++) {
                if (offset + (Nside*4 - ring)*4 > hp) {
                    longind = hp - offset;
                    goto gotit;
                }
                offset += (Nside*4 - ring)*4;
            }
            fprintf(stderr, "healpix_decompose_ring: shouldn't get here!\n");
            if (p_ring) *p_ring = -1;
            if (p_longind) *p_longind = -1;
            return;
        gotit:
            if (p_ring)
                *p_ring = ring;
            if (p_longind)
                *p_longind = longind;
        }

        const int healpix_ring_to_xy(int ring, int Nside) {
            int bighp, x, y;
            int ringind, longind;
            healpix_decompose_ring(ring, Nside, &ringind, &longind);
            if (ringind <= Nside) {
                int ind;
                int v;
                int F1;
                int frow;
                bighp = longind / ringind;
                ind = longind - bighp * ringind;
                y = (Nside - 1 - ind);
                frow = bighp / 4;
                F1 = frow + 2;
                v = F1*Nside - ringind - 1;
                x = v - y;
                return healpix_compose_xy(bighp, x, y, Nside);
            } else if (ringind < 3*Nside) {
                int panel;
                int ind;
                int bottomleft;
                int topleft;
                int frow, F1, F2, s, v, h;
                int bighp = -1;
                int x, y;
                int hp;
                int R = 0;

                panel = longind / Nside;
                ind = longind % Nside;
                bottomleft = ind < (ringind - Nside + 1) / 2;
                topleft = ind < (3*Nside - ringind + 1)/2;

                if (!bottomleft && topleft) {
                    // top row.
                    bighp = panel;
                } else if (bottomleft && !topleft) {
                    // bottom row.
                    bighp = 8 + panel;
                } else if (bottomleft && topleft) {
                    // left side.
                    bighp = 4 + panel;
                } else if (!bottomleft && !topleft) {
                    // right side.
                    bighp = 4 + (panel + 1) % 4;
                    if (bighp == 4) {
                        longind -= (4*Nside - 1);
                        // Gah!  Wacky hack - it seems that since
                        // "longind" is negative in this case, the
                        // rounding behaves differently, so we end up
                        // computing the wrong "h" and have to correct
                        // for it.
                        R = 1;
                    }
                }

                frow = bighp / 4;
                F1 = frow + 2;
                F2 = 2*(bighp % 4) - (frow % 2) + 1;
                s = (ringind - Nside) % 2;
                v = F1*Nside - ringind - 1;
                h = 2*longind - s - F2*Nside;
                if (R)
                    h--;
                x = (v + h) / 2;
                y = (v - h) / 2;
                //fprintf(stderr, "bighp=%i, frow=%i, F1=%i, F2=%i, s=%i, v=%i, h=%i, x=%i, y=%i.\n", bighp, frow, F1, F2, s, v, h, x, y);

                if ((v != (x+y)) || (h != (x-y))) {
                    h++;
                    x = (v + h) / 2;
                    y = (v - h) / 2;
                    //fprintf(stderr, "tweak h=%i, x=%i, y=%i\n", h, x, y);

                    if ((v != (x+y)) || (h != (x-y))) {
                        //fprintf(stderr, "still not right.\n");
                    }
                }
                hp = healpix_compose_xy(bighp, x, y, Nside);
                //fprintf(stderr, "hp %i\n", hp);
                return hp;
            } else {
                int ind;
                int v;
                int F1;
                int frow;
                int ri;
                ri = 4*Nside - ringind;
                bighp = 8 + longind / ri;
                ind = longind - (bighp%4) * ri;
                y = (ri-1) - ind;
                frow = bighp / 4;
                F1 = frow + 2;
                v = F1*Nside - ringind - 1;
                x = v - y;
                return healpix_compose_xy(bighp, x, y, Nside);
            }
        }

        const int healpix_xy_to_ring(int hp, int Nside) {
            int bighp,x,y;
            int frow;
            int F1;
            int v;
            int ring;
            int index;

            healpix_decompose_xy(hp, &bighp, &x, &y, Nside);
            frow = bighp / 4;
            F1 = frow + 2;
            v = x + y;
            // "ring" starts from 1 at the north pole and goes to 4Nside-1 at
            // the south pole; the pixels in each ring have the same latitude.
            ring = F1*Nside - v - 1;
            /*
              ring:
              [1, Nside] : n pole
              (Nside, 2Nside] : n equatorial
              (2Nside+1, 3Nside) : s equat
              [3Nside, 4Nside-1] : s pole
            */
            // this probably can't happen...
            if ((ring < 1) || (ring >= 4*Nside)) {
                fprintf(stderr, "Invalid ring index: %i\n", ring);
                return -1;
            }
            if (ring <= Nside) {
                // north polar.
                // left-to-right coordinate within this healpix
                index = (Nside - 1 - y);
                // offset from the other big healpixes
                index += ((bighp % 4) * ring);
                // offset from the other rings
                index += ring*(ring-1)*2;
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
                index += ((3-(bighp % 4)) * ri);
                // other rings
                index += ri*(ri-1)*2;
                // flip!
                index = 12*Nside*Nside - 1 - index;
            } else {
                // equatorial.
                int s, F2, h;
                s = (ring - Nside) % 2;
                F2 = 2*((int)bighp % 4) - (frow % 2) + 1;
                h = x - y;
                index = (F2 * (int)Nside + h + s) / 2;
                // offset from the north polar region:
                index += Nside*(Nside-1)*2;
                // offset within the equatorial region:
                index += Nside * 4 * (ring - Nside);
                // handle healpix #4 wrap-around
                if ((bighp == 4) && (y > x))
                    index += (4 * Nside - 1);
                //fprintf(stderr, "frow=%i, F1=%i, v=%i, ringind=%i, s=%i, F2=%i, h=%i, longind=%i.\n", frow, F1, v, ring, s, F2, h, (F2*(int)Nside+h+s)/2);
            }
            return index;
        }

        const double healpix_side_length_arcmin(int Nside) {
            return std::sqrt((morph::mathconst<double>::four_pi * mysquare(180.0 * 60.0 / morph::mathconst<double>::pi)) /
                             (12.0 * Nside * Nside));
        }

        double healpix_nside_for_side_length_arcmin(double arcmin) {
            // arcmin2rad: a*pi/10800.0
            return std::sqrt(morph::mathconst<double>::four_pi / (mysquare(arcmin*morph::mathconst<double>::pi/10800.0) * 12.0));
        }

        static bool ispolar(int healpix)
        {
            // the north polar healpixes are 0,1,2,3
            // the south polar healpixes are 8,9,10,11
            return (healpix <= 3) || (healpix >= 8);
        }

        static bool isequatorial(int healpix)
        {
            // the north polar healpixes are 0,1,2,3
            // the south polar healpixes are 8,9,10,11
            return (healpix >= 4) && (healpix <= 7);
        }

        static bool isnorthpolar(int healpix)
        {
            return (healpix <= 3);
        }

        static bool issouthpolar(int healpix)
        {
            return (healpix >= 8);
        }

        static int compose_xy(int x, int y, int Nside) {
            assert(Nside > 0);
            assert(x >= 0);
            assert(x < Nside);
            assert(y >= 0);
            assert(y < Nside);
            return (x * Nside) + y;
        }

        const int healpix_compose_xy(int bighp, int x, int y, int Nside) {
            assert(bighp >= 0);
            assert(bighp < 12);
            return (bighp * Nside * Nside) + compose_xy(x, y, Nside);
        }

        const int64_t healpix_compose_xyl(int bighp, int x, int y, int Nside) {
            int64_t ns = Nside;
            assert(Nside > 0);
            assert(bighp >= 0);
            assert(bighp < 12);
            assert(x >= 0);
            assert(x < Nside);
            assert(y >= 0);
            assert(y < Nside);
            return ((((int64_t)bighp * ns) + x) * ns) + y;
        }

        void healpix_convert_nside(int hp, int nside, int outnside, int* outhp) {
            int basehp, x, y;
            int ox, oy;
            healpix_decompose_xy(hp, &basehp, &x, &y, nside);
            healpix_convert_xy_nside(x, y, nside, outnside, &ox, &oy);
            *outhp = healpix_compose_xy(basehp, ox, oy, outnside);
        }

        void healpix_convert_nsidel(int64_t hp, int nside, int outnside, int64_t* outhp) {
            int basehp, x, y;
            int ox, oy;
            healpix_decompose_xyl(hp, &basehp, &x, &y, nside);
            healpix_convert_xy_nside(x, y, nside, outnside, &ox, &oy);
            *outhp = healpix_compose_xyl(basehp, ox, oy, outnside);
        }

        void healpix_convert_xy_nside(int x, int y, int nside, int outnside,
                                      int* outx, int* outy) {
            double fx, fy;
            int ox, oy;
            assert(x >= 0);
            assert(x < nside);
            assert(y >= 0);
            assert(y < nside);

            // MAGIC 0.5: assume center of pixel...
            fx = (x + 0.5) / (double)nside;
            fy = (y + 0.5) / (double)nside;

            ox = floor(fx * outnside);
            oy = floor(fy * outnside);

            if (outx)
                *outx = ox;
            if (outy)
                *outy = oy;
        }

        void healpix_decompose_xy(int finehp, int* pbighp, int* px, int* py, int Nside) {
            int hp;
            assert(Nside > 0);
            assert(finehp < (12 * Nside * Nside));
            assert(finehp >= 0);
            if (pbighp) {
                int bighp   = finehp / (Nside * Nside);
                assert(bighp >= 0);
                assert(bighp < 12);
                *pbighp = bighp;
            }
            hp = finehp % (Nside * Nside);
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

        void healpix_decompose_xyl(int64_t finehp,
                                   int* pbighp, int* px, int* py,
                                   int Nside) {
            int64_t hp;
            int64_t ns2 = (int64_t)Nside * (int64_t)Nside;
            assert(Nside > 0);
            assert(finehp < (12L * ns2));
            assert(finehp >= 0);
            if (pbighp) {
                int bighp   = finehp / ns2;
                assert(bighp >= 0);
                assert(bighp < 12);
                *pbighp = bighp;
            }
            hp = finehp % ns2;
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

        /**
           Given a large-scale healpix number, computes its neighbour in the
           direction (dx,dy).  Returns -1 if there is no such neighbour.
        */
        static int healpix_get_neighbour(int hp, int dx, int dy)
        {
            if (isnorthpolar(hp)) {
                if ((dx ==  1) && (dy ==  0)) return (hp + 1) % 4;
                if ((dx ==  0) && (dy ==  1)) return (hp + 3) % 4;
                if ((dx ==  1) && (dy ==  1)) return (hp + 2) % 4;
                if ((dx == -1) && (dy ==  0)) return (hp + 4);
                if ((dx ==  0) && (dy == -1)) return 4 + ((hp + 1) % 4);
                if ((dx == -1) && (dy == -1)) return hp + 8;
                return -1;
            } else if (issouthpolar(hp)) {
                if ((dx ==  1) && (dy ==  0)) return 4 + ((hp + 1) % 4);
                if ((dx ==  0) && (dy ==  1)) return hp - 4;
                if ((dx == -1) && (dy ==  0)) return 8 + ((hp + 3) % 4);
                if ((dx ==  0) && (dy == -1)) return 8 + ((hp + 1) % 4);
                if ((dx == -1) && (dy == -1)) return 8 + ((hp + 2) % 4);
                if ((dx ==  1) && (dy ==  1)) return hp - 8;
                return -1;
            } else {
                if ((dx ==  1) && (dy ==  0)) return hp - 4;
                if ((dx ==  0) && (dy ==  1)) return (hp + 3) % 4;
                if ((dx == -1) && (dy ==  0)) return 8 + ((hp + 3) % 4);
                if ((dx ==  0) && (dy == -1)) return hp + 4;
                if ((dx ==  1) && (dy == -1)) return 4 + ((hp + 1) % 4);
                if ((dx == -1) && (dy ==  1)) return 4 + ((hp - 1) % 4);
                return -1;
            }
            return -1;
        }

        static int get_neighbours(hp_t hp, hp_t* neighbour, int Nside) {
            int base;
            int x, y;
            int nn = 0;
            int nbase;
            int nx, ny;

            base = hp.bighp;
            x = hp.x;
            y = hp.y;

            // ( + , 0 )
            nx = (x + 1) % Nside;
            ny = y;
            if (x == (Nside - 1)) {
                nbase = healpix_get_neighbour(base, 1, 0);
                if (isnorthpolar(base)) {
                    nx = x;
                    std::swap(nx, ny);
                }
            } else
                nbase = base;

            neighbour[nn].bighp = nbase;
            neighbour[nn].x = nx;
            neighbour[nn].y = ny;
            nn++;

            // ( + , + )
            nx = (x + 1) % Nside;
            ny = (y + 1) % Nside;
            if ((x == Nside - 1) && (y == Nside - 1)) {
                if (ispolar(base))
                    nbase = healpix_get_neighbour(base, 1, 1);
                else
                    nbase = -1;
            } else if (x == (Nside - 1))
                nbase = healpix_get_neighbour(base, 1, 0);
            else if (y == (Nside - 1))
                nbase = healpix_get_neighbour(base, 0, 1);
            else
                nbase = base;

            if (isnorthpolar(base)) {
                if (x == (Nside - 1))
                    nx = Nside - 1;
                if (y == (Nside - 1))
                    ny = Nside - 1;
                if ((x == (Nside - 1)) || (y == (Nside - 1)))
                    std::swap(nx, ny);
            }

            //printf("(+ +): nbase=%i, nx=%i, ny=%i, pix=%i\n", nbase, nx, ny, nbase*Ns2+xy_to_pnprime(nx,ny,Nside));

            if (nbase != -1) {
                neighbour[nn].bighp = nbase;
                neighbour[nn].x = nx;
                neighbour[nn].y = ny;
                nn++;
            }

            // ( 0 , + )
            nx = x;
            ny = (y + 1) % Nside;
            if (y == (Nside - 1)) {
                nbase = healpix_get_neighbour(base, 0, 1);
                if (isnorthpolar(base)) {
                    ny = y;
                    std::swap(nx, ny);
                }
            } else
                nbase = base;

            //printf("(0 +): nbase=%i, nx=%i, ny=%i, pix=%i\n", nbase, nx, ny, nbase*Ns2+xy_to_pnprime(nx,ny,Nside));

            neighbour[nn].bighp = nbase;
            neighbour[nn].x = nx;
            neighbour[nn].y = ny;
            nn++;

            // ( - , + )
            nx = (x + Nside - 1) % Nside;
            ny = (y + 1) % Nside;
            if ((x == 0) && (y == (Nside - 1))) {
                if (isequatorial(base))
                    nbase = healpix_get_neighbour(base, -1, 1);
                else
                    nbase = -1;
            } else if (x == 0) {
                nbase = healpix_get_neighbour(base, -1, 0);
                if (issouthpolar(base)) {
                    nx = 0;
                    std::swap(nx, ny);
                }
            } else if (y == (Nside - 1)) {
                nbase = healpix_get_neighbour(base, 0, 1);
                if (isnorthpolar(base)) {
                    ny = y;
                    std::swap(nx, ny);
                }
            } else
                nbase = base;

            //printf("(- +): nbase=%i, nx=%i, ny=%i, pix=%i\n", nbase, nx, ny, nbase*Ns2+xy_to_pnprime(nx,ny,Nside));

            if (nbase != -1) {
                neighbour[nn].bighp = nbase;
                neighbour[nn].x = nx;
                neighbour[nn].y = ny;
                nn++;
            }

            // ( - , 0 )
            nx = (x + Nside - 1) % Nside;
            ny = y;
            if (x == 0) {
                nbase = healpix_get_neighbour(base, -1, 0);
                if (issouthpolar(base)) {
                    nx = 0;
                    std::swap(nx, ny);
                }
            } else
                nbase = base;

            //printf("(- 0): nbase=%i, nx=%i, ny=%i, pix=%i\n", nbase, nx, ny, nbase*Ns2+xy_to_pnprime(nx,ny,Nside));

            neighbour[nn].bighp = nbase;
            neighbour[nn].x = nx;
            neighbour[nn].y = ny;
            nn++;

            // ( - , - )
            nx = (x + Nside - 1) % Nside;
            ny = (y + Nside - 1) % Nside;
            if ((x == 0) && (y == 0)) {
                if (ispolar(base))
                    nbase = healpix_get_neighbour(base, -1, -1);
                else
                    nbase = -1;
            } else if (x == 0)
                nbase = healpix_get_neighbour(base, -1, 0);
            else if (y == 0)
                nbase = healpix_get_neighbour(base, 0, -1);
            else
                nbase = base;

            if (issouthpolar(base)) {
                if (x == 0)
                    nx = 0;
                if (y == 0)
                    ny = 0;
                if ((x == 0) || (y == 0))
                    std::swap(nx, ny);
            }

            //printf("(- -): nbase=%i, nx=%i, ny=%i, pix=%i\n", nbase, nx, ny, nbase*Ns2+xy_to_pnprime(nx,ny,Nside));

            if (nbase != -1) {
                neighbour[nn].bighp = nbase;
                neighbour[nn].x = nx;
                neighbour[nn].y = ny;
                nn++;
            }

            // ( 0 , - )
            ny = (y + Nside - 1) % Nside;
            nx = x;
            if (y == 0) {
                nbase = healpix_get_neighbour(base, 0, -1);
                if (issouthpolar(base)) {
                    ny = y;
                    std::swap(nx, ny);
                }
            } else
                nbase = base;

            //printf("(0 -): nbase=%i, nx=%i, ny=%i, pix=%i\n", nbase, nx, ny, nbase*Ns2+xy_to_pnprime(nx,ny,Nside));

            neighbour[nn].bighp = nbase;
            neighbour[nn].x = nx;
            neighbour[nn].y = ny;
            nn++;

            // ( + , - )
            nx = (x + 1) % Nside;
            ny = (y + Nside - 1) % Nside;
            if ((x == (Nside - 1)) && (y == 0)) {
                if (isequatorial(base)) {
                    nbase = healpix_get_neighbour(base, 1, -1);
                } else
                    nbase = -1;

            } else if (x == (Nside - 1)) {
                nbase = healpix_get_neighbour(base, 1, 0);
                if (isnorthpolar(base)) {
                    nx = x;
                    std::swap(nx, ny);
                }
            } else if (y == 0) {
                nbase = healpix_get_neighbour(base, 0, -1);
                if (issouthpolar(base)) {
                    ny = y;
                    std::swap(nx, ny);
                }
            } else
                nbase = base;

            //printf("(+ -): nbase=%i, nx=%i, ny=%i, pix=%i\n", nbase, nx, ny, nbase*Ns2+xy_to_pnprime(nx,ny,Nside));

            if (nbase != -1) {
                neighbour[nn].bighp = nbase;
                neighbour[nn].x = nx;
                neighbour[nn].y = ny;
                nn++;
            }

            return nn;
        }

        int healpix_get_neighbours(int pix, int* neighbour, int Nside) {
            hp_t neigh[8];
            hp_t hp;
            int nn;
            int i;
            inttohp(pix, &hp, Nside);
            nn = get_neighbours(hp, neigh, Nside);
            for (i=0; i<nn; i++)
                neighbour[i] = hptoint(neigh[i], Nside);
            return nn;
        }

        int healpix_get_neighboursl(int64_t pix, int64_t* neighbour, int Nside) {
            hp_t neigh[8];
            hp_t hp;
            int nn;
            int i;
            intltohp(pix, &hp, Nside);
            nn = get_neighbours(hp, neigh, Nside);
            for (i=0; i<nn; i++)
                neighbour[i] = hptointl(neigh[i], Nside);
            return nn;
        }

        static hp_t xyztohp(double vx, double vy, double vz, int Nside,
                            double* p_dx, double* p_dy) {
            double phi;
            constexpr double twothirds = 2.0 / 3.0;
            double dx, dy;
            int basehp;
            int x, y;
            double sector;
            int offset;
            double phi_t;
            hp_t hp;

            // only used in asserts()
            __attribute__ ((unused)) double EPS = 1e-8;

            assert(Nside > 0);

            /* Convert our point into cylindrical coordinates for middle ring */
            phi = atan2(vy, vx);
            if (phi < 0.0)
                phi += morph::mathconst<double>::two_pi;
            phi_t = fmod(phi, morph::mathconst<double>::pi_over_2);
            assert (phi_t >= 0.0);

            // North or south polar cap.
            if ((vz >= twothirds) || (vz <= -twothirds)) {
                double zfactor;
                bool north;
                int column;
                double root;
                double xx, yy, kx, ky;

                // Which pole?
                if (vz >= twothirds) {
                    north = true;
                    zfactor = 1.0;
                } else {
                    north = false;
                    zfactor = -1.0;
                }

                // solve eqn 20: k = Ns - xx (in the northern hemi)
                root = (1.0 - vz*zfactor) * 3.0 * mysquare(Nside * (2.0 * phi_t - morph::mathconst<double>::pi) / morph::mathconst<double>::pi);
                kx = (root <= 0.0) ? 0.0 : std::sqrt(root);

                // solve eqn 19 for k = Ns - yy
                root = (1.0 - vz*zfactor) * 3.0 * mysquare(Nside * 2.0 * phi_t / morph::mathconst<double>::pi);
                ky = (root <= 0.0) ? 0.0 : std::sqrt(root);

                if (north) {
                    xx = Nside - kx;
                    yy = Nside - ky;
                } else {
                    xx = ky;
                    yy = kx;
                }

                // xx, yy should be in [0, Nside].
                x = std::min(static_cast<double>(Nside-1), floor(xx));
                assert(x >= 0);
                assert(x < Nside);

                y = std::min(static_cast<double>(Nside-1), floor(yy));
                assert(y >= 0);
                assert(y < Nside);

                dx = xx - x;
                dy = yy - y;

                sector = (phi - phi_t) / (morph::mathconst<double>::pi_over_2);
                offset = (int)std::round(sector);
                assert(fabs(sector - offset) < EPS);
                offset = ((offset % 4) + 4) % 4;
                assert(offset >= 0);
                assert(offset <= 3);
                column = offset;

                if (north)
                    basehp = column;
                else
                    basehp = 8 + column;

            } else {
                // could be polar or equatorial.
                double sector;
                int offset;
                double u1, u2;
                double zunits, phiunits;
                double xx, yy;

                // project into the unit square z=[-2/3, 2/3], phi=[0, pi/2]
                zunits = (vz + twothirds) / (4.0 / 3.0);
                phiunits = phi_t / morph::mathconst<double>::pi_over_2;
                // convert into diagonal units
                // (add 1 to u2 so that they both cover the range [0,2].
                u1 = zunits + phiunits;
                u2 = zunits - phiunits + 1.0;
                assert(u1 >= 0.);
                assert(u1 <= 2.);
                assert(u2 >= 0.);
                assert(u2 <= 2.);
                // x is the northeast direction, y is the northwest.
                xx = u1 * Nside;
                yy = u2 * Nside;

                // now compute which big healpix it's in.
                // (note that we subtract off the modded portion used to
                // compute the position within the healpix, so this should be
                // very close to one of the boundaries.)
                sector = (phi - phi_t) / (morph::mathconst<double>::pi_over_2);
                offset = (int)std::round(sector);
                assert(fabs(sector - offset) < EPS);
                offset = ((offset % 4) + 4) % 4;
                assert(offset >= 0);
                assert(offset <= 3);

                // we're looking at a square in z,phi space with an X dividing it.
                // we want to know which section we're in.
                // xx ranges from 0 in the bottom-left to 2Nside in the top-right.
                // yy ranges from 0 in the bottom-right to 2Nside in the top-left.
                // (of the phi,z unit box)
                if (xx >= Nside) {
                    xx -= Nside;
                    if (yy >= Nside) {
                        // north polar.
                        yy -= Nside;
                        basehp = offset;
                    } else {
                        // right equatorial.
                        basehp = ((offset + 1) % 4) + 4;
                    }
                } else {
                    if (yy >= Nside) {
                        // left equatorial.
                        yy -= Nside;
                        basehp = offset + 4;
                    } else {
                        // south polar.
                        basehp = 8 + offset;
                    }
                }

                assert(xx >= -EPS);
                assert(xx < (Nside+EPS));
                x = std::max(0.0, std::min(static_cast<double>(Nside-1), floor(xx)));
                assert(x >= 0);
                assert(x < Nside);
                dx = xx - x;

                assert(yy >= -EPS);
                assert(yy < (Nside+EPS));
                y = std::max(0.0, std::min(static_cast<double>(Nside-1), floor(yy)));
                assert(y >= 0);
                assert(y < Nside);
                dy = yy - y;
            }

            hp.bighp = basehp;
            hp.x = x;
            hp.y = y;

            if (p_dx) *p_dx = dx;
            if (p_dy) *p_dy = dy;

            return hp;
        }

        const int xyztohealpix(double x, double y, double z, int Nside) {
            return xyztohealpixf(x, y, z, Nside, NULL, NULL);
        }

        const int64_t xyztohealpixl(double x, double y, double z, int Nside) {
            return xyztohealpixlf(x, y, z, Nside, NULL, NULL);
        }

        int64_t xyztohealpixlf(double x, double y, double z, int Nside,
                               double* p_dx, double* p_dy) {
            hp_t hp = xyztohp(x,y,z, Nside, p_dx,p_dy);
            return hptointl(hp, Nside);
        }

        int xyztohealpixf(double x, double y, double z, int Nside,
                          double* p_dx, double* p_dy) {
            hp_t hp = xyztohp(x,y,z, Nside, p_dx,p_dy);
            return hptoint(hp, Nside);
        }

        // A few functions from starutil
        auto radec2x (auto r, auto d) { return std::cos (d) * std::cos (r); }
        auto radec2y (auto r, auto d) { return std::cos (d) * std::sin (r); }
        auto radec2z (auto r, auto d) { return std::sin (d); }

        auto deg2rad (auto d) { return d * morph::mathconst<double>::deg2rad; }
        auto rad2deg (auto r) { return r * morph::mathconst<double>::rad2deg; }

        double z2dec(double z) { return std::asin(z); }
        double xy2ra(double x, double y) {
            double a = std::atan2(y, x);
            if (a < 0) { a += morph::mathconst<double>::two_pi; }
            return a;
        }
        void xyz2radec(double x, double y, double z, double *ra, double *dec) {
            if (ra) { *ra = xy2ra(x, y); }
            if (dec) { *dec = z2dec(z); }
        }
        void xyzarr2radec(const double* xyz, double *ra, double *dec) {
            xyz2radec(xyz[0], xyz[1], xyz[2], ra, dec);
        }
        void xyzarr2radecdeg(const double* xyz, double *ra, double *dec) {
            xyzarr2radec(xyz, ra, dec);
            if (ra) { *ra  = rad2deg(*ra); }
            if (dec) { *dec = rad2deg(*dec); }
        }

        void radec2xyzarr(double ra, double dec, double* xyz) {
            double cosdec = std::cos(dec);
            xyz[0] = cosdec * std::cos(ra);
            xyz[1] = cosdec * std::sin(ra);
            xyz[2] = std::sin(dec);
        }
        void radecdeg2xyzarr(double ra, double dec, double* xyz) {
            radec2xyzarr(deg2rad(ra),deg2rad(dec), xyz);
        }

        double rad2distsq(double x) { return 2.0 * (1.0 - std::cos(x)); } // inverse of distsq2arc; cosine law.
        constexpr double RAD_PER_ARCSEC = 4.8481368110953598e-06;
        double arcsec2rad(double x) { return x * RAD_PER_ARCSEC; }
        double arcsec2distsq(double x) { return rad2distsq(arcsec2rad(x)); }
        double arcsec2dist(double arcInArcSec) { return std::sqrt(arcsec2distsq(arcInArcSec)); }
        constexpr double ARCSEC_PER_DEG = 3600.0;
        double deg2arcsec(double x) { return x * ARCSEC_PER_DEG; }
        double deg2dist(double arcInDegrees) { return arcsec2dist(deg2arcsec(arcInDegrees)); }

        const int radectohealpix(double ra, double dec, int Nside) {
            return xyztohealpix(radec2x(ra,dec), radec2y(ra,dec), radec2z(ra,dec), Nside);
        }

        int64_t radectohealpixlf(double ra, double dec, int Nside, double* dx, double* dy) {
            return xyztohealpixlf(radec2x(ra,dec), radec2y(ra,dec), radec2z(ra,dec), Nside, dx, dy);
        }

        const int64_t radectohealpixl(double ra, double dec, int Nside) {
            return xyztohealpixl(radec2x(ra,dec), radec2y(ra,dec), radec2z(ra,dec), Nside);
        }

        int radectohealpixf(double ra, double dec, int Nside, double* dx, double* dy) {
            return xyztohealpixf(radec2x(ra,dec), radec2y(ra,dec), radec2z(ra,dec), Nside, dx, dy);
        }

        const int radecdegtohealpix(double ra, double dec, int Nside) {
            return radectohealpix(deg2rad(ra), deg2rad(dec), Nside);
        }

        const int64_t radecdegtohealpixl(double ra, double dec, int Nside) {
            return radectohealpixl(deg2rad(ra), deg2rad(dec), Nside);
        }

        int64_t radecdegtohealpixlf(double ra, double dec, int Nside, double* dx, double* dy) {
            return radectohealpixlf(deg2rad(ra), deg2rad(dec), Nside, dx, dy);
        }

        int radecdegtohealpixf(double ra, double dec, int Nside, double* dx, double* dy) {
            return radectohealpixf(deg2rad(ra), deg2rad(dec), Nside, dx, dy);
        }

        int xyzarrtohealpix(const double* xyz, int Nside) {
            return xyztohealpix(xyz[0], xyz[1], xyz[2], Nside);
        }

        int64_t xyzarrtohealpixl(const double* xyz, int Nside) {
            return xyztohealpixl(xyz[0], xyz[1], xyz[2], Nside);
        }

        int xyzarrtohealpixf(const double* xyz,int Nside, double* p_dx, double* p_dy) {
            return xyztohealpixf(xyz[0], xyz[1], xyz[2], Nside, p_dx, p_dy);
        }

        static void hp_to_xyz(hp_t* hp, int Nside,
                              double dx, double dy,
                              double* rx, double *ry, double *rz) {
            int chp;
            bool equatorial = true;
            double zfactor = 1.0;
            int xp, yp;
            double x, y, z;
            double phi;
            double rad;

            hp_decompose(hp, &chp, &xp, &yp);

            // this is x,y position in the healpix reference frame
            x = xp + dx;
            y = yp + dy;

            if (isnorthpolar(chp)) {
                if ((x + y) > Nside) {
                    equatorial = false;
                    zfactor = 1.0;
                }
            }
            if (issouthpolar(chp)) {
                if ((x + y) < Nside) {
                    equatorial = false;
                    zfactor = -1.0;
                }
            }

            if (equatorial) {
                double zoff=0;
                double phioff=0;
                x /= (double)Nside;
                y /= (double)Nside;

                if (chp <= 3) {
                    // north
                    phioff = 1.0;
                } else if (chp <= 7) {
                    // equator
                    zoff = -1.0;
                    chp -= 4;
                } else if (chp <= 11) {
                    // south
                    phioff = 1.0;
                    zoff = -2.0;
                    chp -= 8;
                } else {
                    // should never get here
                    assert(0);
                }

                z = 2.0/3.0*(x + y + zoff);
                phi = morph::mathconst<double>::pi_over_4*(x - y + phioff + 2*chp);

            } else {
                /*
                  Rearrange eqns (19) and (20) to find phi_t in terms of x,y.

                  y = Ns - k in eq(19)
                  x - Ns - k in eq(20)

                  (Ns - y)^2 / (Ns - x)^2 = (2 phi_t)^2 / (2 phi_t - pi)^2

                  Recall than y<=Ns, x<=Ns and 0<=phi_t<pi/2, so we can choose the
                  root we want by taking square roots:

                  (Ns - y) (pi - 2 phi_t) = 2 phi_t (Ns - x)
                  (Ns - y) pi = 2 phi_t (Ns - x + Ns - y)
                  phi_t = pi (Ns-y) / (2 (Ns - x) + (Ns - y))
                */
                double phi_t;

                if (zfactor == -1.0) {
                    std::swap (x, y);
                    x = (Nside - x);
                    y = (Nside - y);
                }

                if (y == Nside && x == Nside) {
                    phi_t = 0.0;
                } else {
                    phi_t = morph::mathconst<double>::pi * (Nside-y) / (2.0 * ((Nside-x) + (Nside-y)));
                }

                if (phi_t < morph::mathconst<double>::pi_over_4) {
                    z = 1.0 - mysquare(morph::mathconst<double>::pi * (Nside - x) / ((2.0 * phi_t - morph::mathconst<double>::pi) * Nside)) / 3.0;
                } else {
                    z = 1.0 - mysquare(morph::mathconst<double>::pi * (Nside - y) / (2.0 * phi_t * Nside)) / 3.0;
                }
                assert(0.0 <= fabs(z) && fabs(z) <= 1.0);
                z *= zfactor;
                assert(0.0 <= fabs(z) && fabs(z) <= 1.0);

                // The big healpix determines the phi offset
                if (issouthpolar(chp))
                    phi = morph::mathconst<double>::pi_over_2 * (chp-8) + phi_t;
                else
                    phi = morph::mathconst<double>::pi_over_2 * chp + phi_t;
            }

            if (phi < 0.0)
                phi += morph::mathconst<double>::two_pi;

            rad = std::sqrt(1.0 - z*z);
            *rx = rad * std::cos(phi);
            *ry = rad * std::sin(phi);
            *rz = z;
        }

        void healpix_to_xyz(int ihp, int Nside,
                            double dx, double dy,
                            double* px, double *py, double *pz) {
            hp_t hp;
            inttohp(ihp, &hp, Nside);
            hp_to_xyz(&hp, Nside, dx, dy, px, py, pz);
        }

        void healpixl_to_radecdeg(int64_t ihp, int Nside, double dx, double dy,
                                  double* ra, double* dec) {
            hp_t hp;
            double xyz[3];
            intltohp(ihp, &hp, Nside);
            hp_to_xyz(&hp, Nside, dx, dy, xyz, xyz+1, xyz+2);
            xyzarr2radecdeg(xyz, ra, dec);
        }

        void healpix_to_xyzarr(int ihp, int Nside,
                               double dx, double dy,
                               double* xyz) {
            hp_t hp;
            inttohp(ihp, &hp, Nside);
            hp_to_xyz(&hp, Nside, dx, dy, xyz, xyz+1, xyz+2);
        }

        void healpixl_to_xyzarr(int64_t ihp, int Nside, double dx, double dy,
                                double* xyz) {
            hp_t hp;
            longtohp(ihp, &hp, Nside);
            hp_to_xyz(&hp, Nside, dx, dy, xyz, xyz+1, xyz+2);
        }

        void healpix_to_radec(int hp, int Nside,
                              double dx, double dy,
                              double* ra, double* dec) {
            double xyz[3];
            healpix_to_xyzarr(hp, Nside, dx, dy, xyz);
            xyzarr2radec(xyz, ra, dec);
        }

        void healpix_to_radecdeg(int hp, int Nside,
                                 double dx, double dy,
                                 double* ra, double* dec) {
            double xyz[3];
            healpix_to_xyzarr(hp, Nside, dx, dy, xyz);
            xyzarr2radecdeg(xyz, ra, dec);
        }

        void healpix_to_radecarr(int hp, int Nside,
                                 double dx, double dy,
                                 double* radec) {
            double xyz[3];
            healpix_to_xyzarr(hp, Nside, dx, dy, xyz);
            xyzarr2radec(xyz, radec, radec+1);
        }

        void healpix_to_radecdegarr(int hp, int Nside,
                                    double dx, double dy,
                                    double* radec) {
            double xyz[3];
            healpix_to_xyzarr(hp, Nside, dx, dy, xyz);
            xyzarr2radecdeg(xyz, radec, radec+1);
        }

        struct neighbour_dirn {
            double x, y;
            double dx, dy;
        };

        int healpix_get_neighbours_within_range_radec(double ra, double dec, double radius,
                                                      int* healpixes, int Nside) {
            double xyz[3];
            double r;
            radecdeg2xyzarr(ra, dec, xyz);
            r = deg2dist(radius);
            return healpix_get_neighbours_within_range(xyz, r, healpixes, Nside);
        }

        double distsq(const double* d1, const double* d2, int D) {
            double dist2;
            int i;
            dist2 = 0.0;
            for (i=0; i<D; i++) {
		dist2 += mysquare(d1[i] - d2[i]);
            }
            return dist2;
        }
        void normalize_3(double* xyz) {
            double invlen = 1.0 / std::sqrt(xyz[0]*xyz[0] + xyz[1]*xyz[1] + xyz[2]*xyz[2]);
            xyz[0] *= invlen;
            xyz[1] *= invlen;
            xyz[2] *= invlen;
        }

        int healpix_get_neighbours_within_range(double* xyz, double range, int* out_healpixes,
                                                int Nside) {
            int hp;
            int i,j;
            double fx, fy;
            int nhp = 0;

            // HACK -- temp array to avoid cleverly avoiding duplicates
            int healpixes[100];

            //assert(Nside > 0);
            if (Nside <= 0) {
                printf("healpix_get_neighbours_within_range: Nside must be > 0.\n");
                return -1;
            }

            hp = xyzarrtohealpixf(xyz, Nside, &fx, &fy);
            healpixes[nhp] = hp;
            nhp++;

            {
                struct neighbour_dirn dirs[] = {
                    // edges
                    { fx, 0,  0, -1 },
                    { fx, 1,  0,  1 },
                    { 0 , fy,-1,  0 },
                    { 1 , fy, 1,  0 },
                    // bottom corner
                    { 0, 0, -1,  1 },
                    { 0, 0, -1,  0 },
                    { 0, 0, -1, -1 },
                    { 0, 0,  0, -1 },
                    { 0, 0,  1, -1 },
                    // right corner
                    { 1, 0,  1,  1 },
                    { 1, 0,  1,  0 },
                    { 1, 0,  1, -1 },
                    { 1, 0,  0, -1 },
                    { 1, 0, -1, -1 },
                    // left corner
                    { 0, 1,  1,  1 },
                    { 0, 1,  0,  1 },
                    { 0, 1, -1,  1 },
                    { 0, 1, -1,  0 },
                    { 0, 1, -1, -1 },
                    // top corner
                    { 1, 1, -1,  1 },
                    { 1, 1,  0,  1 },
                    { 1, 1,  1,  1 },
                    { 1, 1,  1,  0 },
                    { 1, 1,  1, -1 },
                };
                int ndirs = sizeof(dirs) / sizeof(struct neighbour_dirn);

                double ptx, pty, ptdx, ptdy;
                int pthp;

                for (i=0; i<ndirs; i++) {
                    double pt[3];
                    double ptstepx[3];
                    double ptstepy[3];
                    double across[3];
                    double step = 1e-3;
                    double d2;
                    double stepdirx, stepdiry;
                    struct neighbour_dirn* dir = dirs+i;
                    ptx = dir->x;
                    pty = dir->y;
                    ptdx = dir->dx;
                    ptdy = dir->dy;

                    // pt = point on the edge nearest to the query point.
                    // FIXME -- check that this is true, esp in the polar regions!
                    healpix_to_xyzarr(hp, Nside, ptx, pty, pt);
                    d2 = distsq(pt, xyz, 3);

                    // delta vector should be outside the healpix
                    assert((ptx+step*ptdx < 0) ||
                           (ptx+step*ptdx > 1) ||
                           (pty+step*ptdy < 0) ||
                           (pty+step*ptdy > 1));

                    if (d2 > range*range)
                        continue;

                    // compute dx and dy directions that are toward the interior of
                    // the healpix.
                    stepdirx = (ptx < step) ? 1 : -1;
                    stepdiry = (pty < step) ? 1 : -1;

                    // take steps in those directions.
                    healpix_to_xyzarr(hp, Nside, ptx + stepdirx * step, pty, ptstepx);
                    healpix_to_xyzarr(hp, Nside, ptx, pty + stepdiry * step, ptstepy);

                    // convert the steps into dx,dy vectors.
                    for (j=0; j<3; j++) {
                        ptstepx[j] = stepdirx * (ptstepx[j] - pt[j]);
                        ptstepy[j] = stepdiry * (ptstepy[j] - pt[j]);
                    }

                    // take a small step in the specified direction.
                    for (j=0; j<3; j++)
                        across[j] = pt[j] + ptdx * ptstepx[j] + ptdy * ptstepy[j];

                    // see which healpix is at the end of the step.
                    normalize_3(across);
                    pthp = xyzarrtohealpix(across, Nside);

                    healpixes[nhp] = pthp;
                    nhp++;
                }
            }

            // Remove duplicates...
            for (i=0; i<nhp; i++) {
                for (j=i+1;  j<nhp; j++) {
                    if (healpixes[i] == healpixes[j]) {
                        int k;
                        for (k=j+1; k<nhp; k++)
                            healpixes[k-1] = healpixes[k];
                        nhp--;
                        i=-1;
                        break;
                    }
                }
            }

            for (i=0; i<nhp; i++)
                out_healpixes[i] = healpixes[i];

            return nhp;
        }

#ifdef ENABLE_FUNCTIONS_REQUIRING_QSORT // Avoiding permuted sort code, hiding these functions

        double healpix_distance_to_xyz(int hp, int Nside, const double* xyz,
                                       double* closestxyz) {
            int thehp;
            // corners
            double cdx[4], cdy[4];
            double cdists[4];
            int corder[4];
            int i;
            double dxA,dxB,dyA,dyB;
            double dxmid, dymid;
            double dist2A, dist2B;
            double midxyz[3];
            double dist2mid = 0.0;

            double EPS = 1e-16;

            // If the point is actually inside the healpix, dist = 0.
            thehp = xyzarrtohealpix(xyz, Nside);
            if (thehp == hp) {
                if (closestxyz)
                    memcpy(closestxyz, xyz, 3*sizeof(double));
                return 0;
            }

            // Find two nearest corners.
            for (i=0; i<4; i++) {
                double cxyz[3];
                cdx[i] = i/2;
                cdy[i] = i%2;
                healpix_to_xyzarr(hp, Nside, cdx[i], cdy[i], cxyz);
                cdists[i] = distsq(xyz, cxyz, 3);
            }
            permutation_init(corder, 4);
            permuted_sort(cdists, sizeof(double), compare_doubles_asc, corder, 4);
            // now "corder" [0] and [1] are the indices of the two nearest corners.

            // Binary search for closest dx,dy.
            dxA = cdx[corder[0]];
            dyA = cdy[corder[0]];
            dist2A = cdists[corder[0]];
            dxB = cdx[corder[1]];
            dyB = cdy[corder[1]];
            dist2B = cdists[corder[1]];
            // the closest two points should share an edge... unless we're in some
            // weird configuration like the opposite side of the sphere.
            if (!((dxA == dxB) || (dyA == dyB))) {
                // weird configuration -- return the closest corner.
                if (closestxyz)
                    healpix_to_xyzarr(hp, Nside, cdx[corder[0]], cdy[corder[0]], closestxyz);
                return distsq2deg(cdists[corder[0]]);
            }
            assert(dxA == dxB || dyA == dyB);
            assert(dist2A <= dist2B);

            while (1) {
                dxmid = (dxA + dxB) / 2.0;
                dymid = (dyA + dyB) / 2.0;
                // converged to EPS?
                if ((dxA != dxB && (fabs(dxmid - dxA) < EPS || fabs(dxmid - dxB) < EPS)) ||
                    (dyA != dyB && (fabs(dymid - dyA) < EPS || fabs(dymid - dyB) < EPS)))
                    break;
                healpix_to_xyzarr(hp, Nside, dxmid, dymid, midxyz);
                dist2mid = distsq(xyz, midxyz, 3);
                //printf("  dx,dy (%g,%g) %g  (%g,%g) %g  (%g,%g) %g\n", dxA, dyA, dist2A, dxmid, dymid, dist2mid, dxB, dyB, dist2B);
                if ((dist2mid >= dist2A) && (dist2mid >= dist2B))
                    break;
                if (dist2A < dist2B) {
                    dist2B = dist2mid;
                    dxB = dxmid;
                    dyB = dymid;
                } else {
                    dist2A = dist2mid;
                    dxA = dxmid;
                    dyA = dymid;
                }
            }

            // Check whether endpoint A is actually closer.
            dist2A = cdists[corder[0]];
            if (dist2A < dist2mid) {
                dxA = cdx[corder[0]];
                dyA = cdy[corder[0]];
                healpix_to_xyzarr(hp, Nside, dxA, dyA, midxyz);
                dist2mid = dist2A;
            }

            if (closestxyz)
                memcpy(closestxyz, midxyz, 3*sizeof(double));

            return distsq2deg(dist2mid);
        }

        double healpix_distance_to_radec(int hp, int Nside, double ra, double dec,
                                         double* closestradec) {
            double xyz[3];
            double closestxyz[3];
            double dist;
            radecdeg2xyzarr(ra, dec, xyz);
            dist = healpix_distance_to_xyz(hp, Nside, xyz, closestxyz);
            if (closestradec)
                xyzarr2radecdegarr(closestxyz, closestradec);
            return dist;
        }

        int healpix_within_range_of_radec(int hp, int Nside, double ra, double dec,
                                          double radius) {
            // the trivial implementation...
            return (healpix_distance_to_radec(hp, Nside, ra, dec, NULL) <= radius);
        }
        int healpix_within_range_of_xyz(int hp, int Nside, const double* xyz,
                                        double radius) {
            return (healpix_distance_to_xyz(hp, Nside, xyz, NULL) <= radius);
        }

        void healpix_radec_bounds(int hp, int nside,
                                  double* p_ralo, double* p_rahi,
                                  double* p_declo, double* p_dechi) {
            // corners!
            double ralo,rahi,declo,dechi;
            double ra,dec;
            double dx, dy;
            ralo = declo =  LARGE_VAL;
            rahi = dechi = -LARGE_VAL;
            for (dy=0; dy<2; dy+=1.0) {
                for (dx=0; dx<2; dx+=1.0) {
                    healpix_to_radecdeg(hp, nside, dx, dy, &ra, &dec);
                    // FIXME -- wrap-around.
                    ralo = std::min(ra, ralo);
                    rahi = std::max(ra, rahi);
                    declo = std::min(dec, declo);
                    dechi = std::max(dec, dechi);
                }
            }
            if (p_ralo) *p_ralo = ralo;
            if (p_rahi) *p_rahi = rahi;
            if (p_declo) *p_declo = declo;
            if (p_dechi) *p_dechi = dechi;
        }
#endif

    } // namespace am
} // namespace hp
