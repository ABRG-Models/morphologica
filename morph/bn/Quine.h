#pragma once

#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include <morph/bn/Implicant.h>

namespace morph {
    namespace bn {

        //! Implements the Quine-McCluskey algorithm to find the minimal form for a
        //! Boolean function.
        class Quine
        {
        public:
            int combs;
            std::vector<int> minterms;
            std::vector<Implicant> implicants;
            int vars;
            bool pr = true;
            bool fin = true;
            //! Stores results of expression compression
            std::vector<size_t> M0, M1;
            //! Stores results of expression compression
            std::vector<Implicant> primes;
            //! The final complexity value.
            unsigned int cplexity = 0;
            unsigned int outof = 0;
            size_t ind = 0;

            // Debugging
            constexpr bool verboseout = false;

            Quine (int _vars): vars(_vars) { combs = 1 << this->vars; }

            void addMinterm (int m)
            {
                minterms.push_back (m);
                implicants.push_back (Implicant(m, vars));
            }

            int count1s (size_t x)
            {
                int o = 0;
                while (x) {
                    o += x % 2;
                    x >>= 1;
                }
                return o;
            }

            void mul (std::vector<size_t> &a, const std::vector<size_t> &b)
            {
                std::vector<size_t> v;
                for (size_t i = 0; i < a.size(); ++i) {
                    for (size_t j = 0; j < b.size(); ++j) {
                        v.push_back(a[i] | b[j]);
                    }
                }
                sort (v.begin(), v.end());
                v.erase (unique (v.begin(), v.end()), v.end());
                for (size_t i = 0; i < v.size() - 1; ++i) {
                    for (size_t j = v.size() - 1; j > i ; --j) {
                        size_t z = v[i] & v[j];
                        if ((z & v[i]) == v[i]) {
                            v.erase (v.begin() + j);
                        } else if ((z & v[j]) == v[j]) {
                            size_t t = v[i];
                            v[i] = v[j];
                            v[j] = t;
                            v.erase(v.begin() + j);
                            j = v.size();
                        }
                    }
                }
                a = v;
            }

            void go (void)
            {
                if constexpr (verboseout) { if (!minterms.size()) { cout << "\n\tF = 0\n"; } }

                sort (minterms.begin(), minterms.end());
                minterms.erase( unique( minterms.begin(), minterms.end() ), minterms.end() );

                sort (implicants.begin(), implicants.end());
                if constexpr (verboseout) {
                    for (size_t i = 0; i < implicants.size(); ++i) {
                        cout << implicants[i].output(pr, fin) << endl;
                    }
                    cout << "-------------------------------------------------------\n";
                }

                std::vector<Implicant> aux;
                while (implicants.size() > 1) {
                    for (size_t i = 0; i < implicants.size() - 1; ++i) {
                        for (size_t j = implicants.size() - 1; j > i ; --j) {
                            if (implicants[j].bits == implicants[i].bits) {
                                implicants.erase (implicants.begin() + j);
                            }
                        }
                    }
                    aux.clear();
                    for (size_t i = 0; i < implicants.size() - 1; ++i) {
                        for (size_t j = i + 1; j < implicants.size(); ++ j) {
                            if (implicants[j].ones == implicants[i].ones + 1 &&
                                implicants[j].mask == implicants[i].mask &&
                                count1s(implicants[i].implicant ^
                                        implicants[j].implicant) == 1) {
                                implicants[i].used = true;
                                implicants[j].used = true;
                                aux.push_back(Implicant(implicants[i].implicant,
                                                        this->vars,
                                                        implicants[i].cat(implicants[j]),
                                                        implicants[i].minterms + ',' +
                                                        implicants[j].minterms,
                                                        (implicants[i].implicant ^
                                                         implicants[j].implicant) | implicants[i].mask));
                            }
                        }
                    }
                    for (size_t i = 0; i < implicants.size(); ++i) {
                        if (!implicants[i].used) {
                            primes.push_back(implicants[i]);
                        }
                    }
                    implicants = aux;
                    sort (implicants.begin(), implicants.end());
                    if constexpr (verboseout) {
                        for (size_t i = 0; i < implicants.size(); ++i) {
                            cout << implicants[i].output(pr, fin) << endl;
                        }
                        cout << "-------------------------------------------------------\n";
                    }
                }
                for (size_t i = 0; i < implicants.size(); ++i) {
                    primes.push_back (implicants[i]);
                }
                if constexpr (verboseout) {
                    if (primes.back().mask == combs - 1) {
                        cout << "\n\tF = 1\n";
                    }
                }
                this->pr = false;
                bool table[primes.size()][minterms.size()];
                for (size_t i = 0; i < primes.size(); ++i) {
                    for (size_t k = 0; k < minterms.size(); ++k) {
                        table[i][k] = false;
                    }
                }
                for (size_t i = 0; i < primes.size(); ++i) {
                    for (size_t j = 0; j < primes[i].mints.size(); ++j) {
                        for (size_t k = 0; k < minterms.size(); ++k) {
                            if (primes[i].mints[j] == minterms[k]) {
                                table[i][k] = true;
                            }
                        }
                    }
                }
                if constexpr (verboseout) {
                    for (int k = 0; k < 18; ++k) { cout << " "; }
                    for (size_t k = 0; k < minterms.size(); ++k) {
                        cout << right << setw(2) << minterms[k] << ' ';
                    }
                    cout << endl;
                    for (int k = 0; k < 18; ++k) { cout << " "; }
                    for (size_t k = 0; k < minterms.size(); ++k) { cout << "---"; }
                    cout << endl;
                    for (size_t i = 0; i < primes.size(); ++i) {
                        cout << primes[i].output (pr, fin) << " |";
                        for (size_t k = 0; k < minterms.size(); ++k) {
                            cout << (table[i][k] ? " X " : " ");
                        }
                        cout << endl;
                    }
                }

                for (size_t i = 0; i < primes.size(); ++i) {
                    if (table[i][0]) {
                        M0.push_back(1 << i);
                    }
                }
                for (size_t k = 1; k < minterms.size(); ++k) {
                    M1.clear();
                    for (size_t i = 0; i < primes.size(); ++i) {
                        if (table[i][k]) {
                            M1.push_back(1 << i);
                        }
                    }
                    this->mul (M0, M1);
                }
                int min = count1s(M0[0]);
                this->ind = 0;
                for (size_t i = 1; i < M0.size(); ++i) {
                    if (min > count1s(M0[i])) {
                        min = count1s(M0[i]);
                        this->ind = i;
                    }
                }
                this->fin = false;
                if constexpr (verboseout) {
                    bool f;
                    cout << "-------------------------------------------------------\n";
                    for (size_t j = 0; j < M0.size(); ++j) {
                        cout << "\tF = ";
                        f = false;
                        for (size_t i = 0; i < primes.size(); ++i)
                            if (M0[j] & (1 << i)) {
                                if (f) { cout << " + "; }
                                f = true;
                                cout << primes[i].output (this->pr, this->fin);
                            }
                        cout << endl;
                    }
                    cout << "-------------------------------------------------------\n";

                    // minimal solution
                    cout << "F = ";
                    f = false;
                    for (size_t i = 0; i < primes.size(); ++i) {
                        if (M0[this->ind] & (1 << i)) {
                            if (f) { cout << " + "; }
                            f = true;
                            cout << primes[i].output (this->pr, this->fin);
                        }
                    }
                    cout << endl;
                }
            }

            //! Run after go()
            double complexity (void)
            {
                this->cplexity = 0;

                this->outof = 1 << this->vars;

                for (size_t i = 0; i < primes.size(); ++i) {
                    if (M0[this->ind] & (1 << i)) {
                        this->cplexity++;
                    }
                }
                return (double)this->cplexity/(double)this->outof;
            }

            //! Run after go()
            string min (void)
            {
                string s("F = ");
                bool f = false;
                for (size_t i = 0; i < primes.size(); ++i) {
                    if (M0[this->ind] & (1 << i)) {
                        if (f) { s += " + "; }
                        f = true;
                        s += primes[i].output (this->pr, this->fin);
                    }
                }
                return s;
            }
        };

    } // namespace bn (Boolean Nets)
} // namespace morph
