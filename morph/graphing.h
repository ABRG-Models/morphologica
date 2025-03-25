/*
 * This file contains static functions in the morph::graphing namespace that are used to
 * do graph-like things used in several places (GraphVisual, ColourBarVisual)
 *
 * Seb James
 * March 2025
 */

#pragma once

#include <string>
#include <cmath>
#include <limits>
#include <deque>
#include <iostream>
#ifdef MORPH_HAVE_STD_FORMAT
# include <format>
#else
# include <sstream>
# include <iomanip>
# include <ios>
#endif

#include <morph/range.h>
#include <morph/math.h>
#include <morph/vvec.h>

namespace morph::graphing {

    // Debug number_format?
    static constexpr bool nf_debug = false;

    // Debug maketicks?
    static constexpr bool mt_debug = false;

    //! Graph-specific number formatting for tick labels. You must pass in an adjacent
    //! label (which affects the optimum precision to use for formatting)
    template <typename F>
    static std::string number_format (const F num, const F adjacent_num)
    {
        if constexpr (nf_debug) {
            std::cout << std::endl << "number_format (" << num << ", " << adjacent_num
                      << ") called. diff = " << num - adjacent_num << std::endl;
        }

        morph::range<int> num_sigcols = morph::math::significant_cols<F> (num);
        F num_diff = std::abs (num - adjacent_num);
        morph::range<int> diff_sigcols = morph::math::significant_cols<F> (num_diff);
        if constexpr (nf_debug) {
            std::cout << "num_sigcols: " << num_sigcols << ", diff_sigcols: " << diff_sigcols << std::endl;
        }

        // Whats the num_diff maxcol? is it 9.5 plus? In which case it would round up
        if (num_diff * morph::math::pow (F{10}, -diff_sigcols.max) >= F{9.5}) {
            // std::cout << "roundup diff_sigcols.max\n";
            diff_sigcols.max += 1;
        }

        // Which is the minimum column we should show?
        int min_col = std::min (num_sigcols.max, diff_sigcols.max);

        // What's the best precision value - actual value? If it's non-negligible, then
        // add to precision. I think this is the graphing specific logic that I need.
        F rounded = morph::math::round_to_col (num, min_col);
        if constexpr (nf_debug) { std::cout << num <<  " rounded to " << min_col << " is " << rounded << std::endl; }
        while (min_col > (diff_sigcols.max - 2)
               && std::abs(rounded - num) > morph::math::pow(F{10}, min_col - 1)) {
            if constexpr (nf_debug) {

                std::cout << "abs(rounded - num): abs(" << rounded << " - " << num << ")\n";

                std::cout << "Rounding comparison was true: (" << std::abs(rounded - num) << ") > "
                          << morph::math::pow(F{10}, min_col - 1) << " == true\n";

                std::cout << "Calling round_to_col again with min_col = " << (min_col - 1) << std::endl;
            }
            min_col -= 1;
            rounded = morph::math::round_to_col (num, min_col);
        }
        if constexpr (nf_debug) {
            if (min_col > (diff_sigcols.max - 2)) {
                std::cout << "Rounding comparison was false: (" << std::abs(rounded - num) << ") cf 10^min_col-1: "
                          << morph::math::pow(F{10}, min_col - 1) << "\n";
            }
        }

        if constexpr (nf_debug) { std::cout << "min_col = " << min_col << " Now format it...\n"; }

#ifdef MORPH_HAVE_STD_FORMAT
        std::string s = "0";
        if (num != F{0}) {
            if (num_sigcols.max > 3) {
                s = std::format ("{:.{}e}", num, num_sigcols.max - min_col);
            } else {
                s = std::format ("{:.{}f}", num, (min_col <= 0 ? -min_col : 0));
            }
        }
#else
        std::stringstream ss;
        if (num != F{0}) {
            if (num_sigcols.max > 3) {
                ss << std::scientific << std::setprecision (num_sigcols.max - min_col);
            } else {
                ss << std::fixed << std::setprecision ((min_col <= 0 ? -min_col : 0));
            }
            ss << num;
        } else {
            ss << "0";
        }
        std::string s = ss.str();
#endif
        if (num > F{-1} && num < F{1} && num != F{0}) {
            // It's a 0.something number. Get rid of any 0 preceding a '.'
            std::string::size_type p = s.find ('.');
            if (p != std::string::npos && p > 0) {
                if (s[--p] == '0') { s.erase (p, 1); }
            }
        }

        if constexpr (nf_debug) { std::cout << "returning '" << s << "'\n"; }
        return s;
    }

    /*!
     * Auto-computes the tick marker locations (in data space) for the data range rmin to
     * rmax. realmin and realmax gives the data range actually displayed on the graph - it's the
     * data range, plus any padding introduced by GraphVisual::dataaxisdist
     *
     * This overload accepts a morph::range for the preferred number of ticks.
     *
     * The bool arg allows the client code to either accept that _num_ticks_range is
     * guidance OR to *force* the number of ticks to be in the range, even if it
     * means irrational values. Defaults to false, which is the original behaviour
     * of maketicks.
     */
    template <typename F>
    static std::deque<F> maketicks (F rmin, F rmax, float realmin, float realmax, const morph::range<F>& _num_ticks_range)
    {
        if constexpr (mt_debug) {
            std::cout << "\nmorph::graphing::maketicks (" << rmin << "," << rmax << "," << realmin << "," << realmax << "," << _num_ticks_range << ")\n";
        }
        std::deque<F> ticks = {};

        if (std::numeric_limits<F>::has_quiet_NaN) { // If we are passed NaN for ranges, then return empty ticks
            if (std::isnan (rmin) || std::isnan (rmax) || std::isnan (realmin) || std::isnan (realmax)) { return ticks; }
        }

        F drange = rmax - rmin; // data range
        if (drange <= std::numeric_limits<F>::epsilon()
            || (_num_ticks_range.min == 2 && _num_ticks_range.max == 2)) {
            if constexpr (mt_debug) { std::cout << "data range " << rmin << " to " << rmax << " is <= eps or ticks range is {2,2}\n"; }
            // Just two ticks in this case - one at drange min and one at max.
            ticks.push_back (rmin);
            ticks.push_back (rmax);
            return ticks;
        }

        F tickspacing = F{0};
        F numtickintervals = F{0};
        F actual_numticks = F{0};

        auto subr_find_tickspacing = [drange, _num_ticks_range, &tickspacing, &numtickintervals, &actual_numticks] (const F base = F{10})
        {
            // How big should the tick spacing be? log the drange, find the floor, raise it to get candidate
            tickspacing = std::pow (base, std::floor (std::log (drange) / std::log(base))); // log(x)/log(5) gives log of x in base 5
            numtickintervals = std::floor (drange / tickspacing);

            if constexpr (mt_debug) {
                std::cout << "initial tickspacing = " << tickspacing << ", giving " << (numtickintervals + F{1}) << " ticks\n";
            }

            constexpr F mult = F{2};
            if (numtickintervals > _num_ticks_range.max) {
                if constexpr (mt_debug) { std::cout << "too many ticks, increase spacing to " << (tickspacing * mult) << "\n"; }
                while (numtickintervals > _num_ticks_range.max && numtickintervals > _num_ticks_range.min) {
                    tickspacing = tickspacing * mult; // bigger tick spacing means fewer ticks
                    numtickintervals = std::floor (drange / tickspacing);
                }

            } else if (numtickintervals < _num_ticks_range.min) {
                if constexpr (mt_debug) { std::cout << "too few ticks, decrease spacing to " << (tickspacing / mult) << "\n"; }
                while (numtickintervals < _num_ticks_range.min && numtickintervals < _num_ticks_range.max && tickspacing > std::numeric_limits<F>::epsilon()) {
                    tickspacing = tickspacing / mult;
                    numtickintervals = std::floor (drange / tickspacing);
                }
            }
            if constexpr (mt_debug) { std::cout << "now tickspacing = " << tickspacing << ", giving " << (numtickintervals + F{1}) << " ticks\n"; }

            actual_numticks = numtickintervals + F{1};
        };

        // Find a tick spacing that is 'neat'
        F tbase = F{10};
        bool recompute_required = true;
        while (recompute_required == true && tbase > F{0}) {
            if constexpr (mt_debug) { std::cout << "recomputing with base " << tbase << std::endl; }
            if constexpr (mt_debug) { std::cout << "Call subr_find_tickspacing (" << tbase << ")\n"; }
            subr_find_tickspacing (tbase);
            recompute_required = (actual_numticks > _num_ticks_range.max || actual_numticks < _num_ticks_range.min) ? true : false;
            tbase -= F{1};
        }

        // Realmax and realmin come from the full range of abscissa_scale/ord1_scale
        if (actual_numticks < _num_ticks_range.min || actual_numticks > _num_ticks_range.max) {
            // In this case our 'neat' algorithm failed, so just force some ticks with linspace
            int force_num = static_cast<int>(std::floor((_num_ticks_range.max + _num_ticks_range.min)  / F{2}));
            if constexpr (mt_debug) { std::cout << "Forcing " << force_num << " ticks\n"; }
            morph::vvec<F> linticks;
            linticks.linspace (rmin, rmax, force_num);
            for (auto lt : linticks) { ticks.push_back (lt); }

        } else {
            // Our 'neat' algo found a nice tickspacing, so create the ticks for that:
            F midrange = (rmin + rmax) * F{0.5};
            F a = std::round (midrange / tickspacing);
            F atick = a * tickspacing;
            while (atick <= realmax && ticks.size() < (10 * _num_ticks_range.max)) { // 2nd test avoids inf loop
                // This tick is smaller than 100th of the size of one whole tick to tick spacing, so it must be 0.
                ticks.push_back (std::abs(atick) < F{0.01} * std::abs(tickspacing) ? F{0} : atick);
                atick += tickspacing;
            }
            atick = (a * tickspacing) - tickspacing;
            while (atick >= realmin && ticks.size() < (10 * _num_ticks_range.max)) {
                ticks.push_front (std::abs(atick) < F{0.01} * std::abs(tickspacing) ? F{0} : atick);
                atick -= tickspacing;
            }
        }

        // If, for any reason, we ended up with just one tick (or none), revert to min/0/max
        if (ticks.size() < 2) {
            if constexpr (mt_debug) { std::cout << "Replacing ticks with min/max as there was just one tick\n"; }
            ticks.clear();
            ticks.push_back (rmin);
            if (rmin < F{0} && rmax > F{0}) { ticks.push_back (F{0}); }
            ticks.push_back (rmax);
        }
        return ticks;
    }

    /*!
     * Auto-computes the tick marker locations (in data space) for the data range rmin to
     * rmax. realmin and realmax gives the data range actually displayed on the graph - it's the
     * data range, plus any padding introduced by GraphVisual::dataaxisdist.
     *
     * This overload accepts separate min and max for the preferred number of ticks.
     */
    template <typename F>
    static std::deque<F> maketicks (F rmin, F rmax, float realmin, float realmax, const F _min_num_ticks = 3, const F _max_num_ticks = 10)
    {
        morph::range<F> _num_ticks_range(_min_num_ticks, _max_num_ticks);
        return morph::graphing::maketicks<F> (rmin, rmax, realmin, realmax, _num_ticks_range);
    }

} // namespace morph::graphing
