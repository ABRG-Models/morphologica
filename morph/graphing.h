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
#endif

#include <morph/range.h>

namespace morph::graphing {

    static constexpr bool gv_debug = false;

    //! Graph-specific number formatting for tick labels.
    template <typename F>
    static std::string number_format (const F num)
    {
#ifdef MORPH_HAVE_STD_FORMAT
        std::string s = std::format ("{:.4g}", num);
#else
        std::stringstream ss;
        ss << num;
        std::string s = ss.str();
#endif
        if (num > F{-1} && num < F{1} && num != F{0}) {
            // It's a 0.something number. Get rid of any 0 preceding a '.'
            std::string::size_type p = s.find ('.');
            if (p != std::string::npos && p>0) {
                if (s[--p] == '0') { s.erase(p, 1); }
            }
        }

        return s;
    }

    //! Graph-specific number formatting for tick labels, when you can pass in the adjacent label (which affects formatting)
    template <typename F>
    static std::string number_format (const F num, const F adjacent_num)
    {
        if constexpr (gv_debug) { std::cout << std::endl; }
        F num_diff = std::abs (num - adjacent_num);
        int expnt_diff = static_cast<int>(std::floor (std::log10 (num_diff))); // may be negative
        int expnt_num = 0;
        int precn = 0;

        if (num != F{0}) {
            expnt_num = static_cast<int>(std::floor (std::log10 (std::abs (num))));
        }

        if constexpr (gv_debug) {
            std::cout << "expnt_diff: " << expnt_diff << " for num_diff: " << num_diff
                      << " [log10(" << num_diff << ") = " << std::log10 (num_diff) << "]\n";
            std::cout << "expnt_num: " << expnt_num << " for num: " << num
                      << " [log10(|" << num << "|) = " << std::log10 (std::abs (num)) << "]\n";
        }

        // If there is additional data beyond the critical precision, then show one extra col
        // e.g. if we have  0.000, 0.250, 0.500 then we want to show it as:
        // 0* 0.25, 0.50  (* 0 always gets special treatment)
        // If we have 0.000, 1.000, 2.000 we want to show it as
        // 0, 1, 2
        F pf = std::pow (F{10}, expnt_diff);
        F col_unit = pf * std::floor (std::abs (num) / pf);
        if ((std::abs (num) - col_unit) > F{0}) { expnt_diff -= 1; }

#ifdef MORPH_HAVE_STD_FORMAT
        std::string s = "0";
        if (num != F{0}) {
            if (expnt_num > 3) {
                precn = expnt_num < 0 ? 0: expnt_num;
                s = std::format ("{:.{}e}", num, precn);
            } else {
                precn = expnt_diff < 0 ? -expnt_diff: 0;
                s = std::format ("{:.{}f}", num, precn);
            }
        }
#else
        std::stringstream ss;
        if (num != F{0}) {
            if (expnt_num > 3) {
                precn = expnt_num < 0 ? 0: expnt_num;
            } else {
                precn = expnt_diff < 0 ? -expnt_diff: 0;
            }
            ss << std::setprecision (precn);
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
    static std::deque<F> maketicks (F rmin, F rmax, float realmin, float realmax,
                                    const morph::range<F>& _num_ticks_range, const bool strict_num_ticks_mode = false)
    {
        if constexpr (gv_debug) {
            std::cout << "\nmorph::graphing::maketicks (" << rmin << "," << rmax << "," << realmin << "," << realmax << "," << _num_ticks_range << ")\n";
        }
        std::deque<F> ticks = {};

        if (std::numeric_limits<F>::has_quiet_NaN) { // If we are passed NaN, then return empty ticks
            if (std::isnan (rmin) || std::isnan (rmax) || std::isnan (realmin) || std::isnan (realmax)) { return ticks; }
        }

        F drange = rmax - rmin; // data range
        if (drange <= std::numeric_limits<F>::epsilon()) {
            if constexpr (gv_debug) { std::cout << "data range " << rmin << " to " << rmax << " is <= eps\n"; }
            // Just two ticks in this case - one at drange min and one at max.
            ticks.push_back (rmin);
            ticks.push_back (rmax);
            return ticks;
        }

        F tickspacing = F{0};
        F numtickintervals = F{0};
        F actual_numticks = F{0};

        // A subroutine lambda to find a suitable tickspacing
        auto subr_find_tickspacing = [drange, _num_ticks_range, &tickspacing, &numtickintervals, &actual_numticks] (const F mult = F{2})
        {
            // How big should the tick spacing be? log the drange, find the floor, raise it to get candidate
            tickspacing = std::pow (F{10}, std::floor (std::log10 (drange)));
            numtickintervals = std::floor (drange / tickspacing);
            if constexpr (gv_debug) { std::cout << "initial tickspacing = " << tickspacing << ", giving " << (numtickintervals + F{1}) << " ticks\n"; }
            if (numtickintervals > _num_ticks_range.max) {
                if constexpr (gv_debug) { std::cout << "too many ticks, increase spacing to " << (tickspacing * mult) << "\n"; }
                while (numtickintervals > _num_ticks_range.max && numtickintervals > _num_ticks_range.min) {
                    tickspacing = tickspacing * mult; // bigger tick spacing means fewer ticks
                    numtickintervals = std::floor (drange / tickspacing);
                }

            } else if (numtickintervals < _num_ticks_range.min) {
                if constexpr (gv_debug) { std::cout << "too few ticks, decrease spacing to " << (tickspacing / mult) << "\n"; }
                while (numtickintervals < _num_ticks_range.min && numtickintervals < _num_ticks_range.max && tickspacing > std::numeric_limits<F>::epsilon()) {
                    tickspacing = tickspacing / mult;
                    numtickintervals = std::floor (drange / tickspacing);
                }
            }
            if constexpr (gv_debug) { std::cout << "now tickspacing = " << tickspacing << ", giving " << (numtickintervals + F{1}) << " ticks\n"; }

            actual_numticks = numtickintervals + F{1};
        };

        // Run the find tickspacing routine with a multiplier that guarantees 'nice' tick values
        F tmult = F{2};
        subr_find_tickspacing (tmult);

        // Optionally, be strict about keeping in range, at cost of 'nice' tick values
        if (strict_num_ticks_mode == true) {
            tmult = F{1.2};
            if constexpr (gv_debug) { std::cout << "strict_num_ticks_mode == true\n"; }
            bool recompute_required = (actual_numticks > _num_ticks_range.max || actual_numticks < _num_ticks_range.min) ? true : false;
            unsigned int attempts = 0;
            while (recompute_required == true && attempts < 6) {
                tmult *= F{1.2};
                if constexpr (gv_debug) { std::cout << "Call subr_find_tickspacing (" << tmult << ")\n"; }
                subr_find_tickspacing (tmult);
                recompute_required = (actual_numticks > _num_ticks_range.max || actual_numticks < _num_ticks_range.min) ? true : false;
                ++attempts;
            }
        }

        if constexpr (gv_debug) {
            if (actual_numticks < _num_ticks_range.min) { std::cout << "Too few ticks!\n"; }
            if (actual_numticks > _num_ticks_range.max) { std::cout << "Too many ticks!\n"; }
        }

        // Realmax and realmin come from the full range of abscissa_scale/ord1_scale
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

        // If we ended up with just one tick (or none), revert to min and max ticks, whether or not we're in strict mode
        if (ticks.size() < 2) {
            ticks.clear();
            ticks.push_back (rmin);
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
    static std::deque<F> maketicks (F rmin, F rmax, float realmin, float realmax,
                                    const F _min_num_ticks = 3, const F _max_num_ticks = 10, const bool strict_num_ticks_mode = false)
    {
        morph::range<F> _num_ticks_range(_min_num_ticks, _max_num_ticks);
        return morph::graphing::maketicks<F> (rmin, rmax, realmin, realmax, _num_ticks_range, strict_num_ticks_mode);
    }

} // namespace morph::graphing
