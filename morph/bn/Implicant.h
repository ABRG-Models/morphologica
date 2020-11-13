#pragma once

#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>

namespace morph {
    namespace bn {

        //! An implicant, as used in the Quine-McCluskey algorithm
        struct Implicant
        {
            int implicant;
            int mask;
            int ones;
            int vars;
            bool used;
            std::string minterms;
            std::string bits;
            std::vector<int> mints;

            Implicant(int i = 0,
                      int _vars = 1,
                      std::vector<int> min = std::vector<int>(),
                      std::string t = "",
                      int m = 0, bool
                      u = false)
                : implicant(i)
                , mask(m)
                , ones(0)
                , vars(_vars)
                , used(u)
            {
                if (t == "") {
                    std::stringstream ss;
                    ss << 'm' << i;
                    minterms = ss.str();
                } else {
                    minterms = t;
                }
                if (min.empty()) {
                    mints.push_back(i);
                } else {
                    mints = min;
                }
                int bit = 1 << vars;
                while (bit >>= 1) {
                    if (m & bit) {
                        bits += '-';
                    } else if (i & bit) {
                        bits += '1'; ++ones;
                    } else {
                        bits += '0';
                    }
                }
            }

            bool operator<(const Implicant& b) const { return ones < b.ones; }

            std::vector<int> cat (const Implicant& b)
            {
                std::vector<int> v = mints;
                v.insert (v.end(), b.mints.begin(), b.mints.end());
                return v;
            }

            // An output function. Takes two boolean args for formatting.
            std::string str (const bool& pr, const bool& fin)
            {
                int bit = 1 << this->vars, lit = 0;
                std::ostringstream ss;
                if (fin) {
                    ss << right << setw(16);
                }
                while (bit >>= 1) {
                    if (!(this->mask & bit)) {
                        ss << char(lit + 'A') << (this->implicant & bit ? ' ' : '\'');
                    }
                    ++lit;
                }
                if (pr) {
                    ss << '\t' << setw(16) << left << this->minterms << ' ' << this->bits << '\t' << this->ones;
                }
                return ss.str();
            }
        };

    } // namespace bn (Boolean Nets)
} // namespace morph
