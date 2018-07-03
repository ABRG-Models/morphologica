#ifndef _BEZCURVEPATH_H_
#define _BEZCURVEPATH_H_

#include "BezCurve.h"

#include <list>
#include <utility>
#include <iostream>
#include <fstream>

using std::pair;
using std::make_pair;
using std::list;
using std::string;
using std::cout;
using std::endl;
using std::ofstream;

namespace morph
{
    /*!
     * A class defining a path made up of Bezier curves. This has an
     * initial position, and then a list of curves that make up the
     * path. I've kept this very simple with all public member
     * attributes.
     */
    class BezCurvePath
    {
    public:
        string name = "";
        pair<float, float> initialCoordinate = make_pair (0.0f, 0.0f);
        list<BezCurve> curves;
        void addCurve (BezCurve& c) {
            this->curves.push_back (c);
        }
        void output (void) const {
            cout << "------ BezCurvePath ------" << endl;
            cout << "Name: " << this->name << endl;
            cout << "Initial coord: (" << this->initialCoordinate.first
                 << "," << this->initialCoordinate.second << ")" << endl;
            cout << "Number of curves: " << this->curves.size() << endl;
            list<BezCurve>::const_iterator i = this->curves.begin();
            while (i != this->curves.end()) {
                cout << i->output(static_cast<unsigned int>(20));
                ++i;
            }
            cout << "------ End BezCurvePath ------" << endl;
        }
        void save (float step) const {
            ofstream f;
            string fname = this->name + ".csv";
            f.open (fname.c_str(), std::ios::out|std::ios::trunc);
            if (f.is_open()) {
                list<BezCurve>::const_iterator i = this->curves.begin();
                while (i != this->curves.end()) {
                    f << i->output (step);
                    ++i;
                }
                f.close();
            }
        }
    };

} // namespace morph

#endif // _BEZCURVEPATH_H_
