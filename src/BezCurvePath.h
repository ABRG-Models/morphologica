#ifndef _BEZCURVEPATH_H_
#define _BEZCURVEPATH_H_

#include "BezCurve.h"

#include <list>
#include <vector>
#include <utility>
#include <iostream>
#include <fstream>

using std::pair;
using std::make_pair;
using std::list;
using std::vector;
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
        /*!
         * The name of this BezCurvePath. This is intended to be taken
         * from the layer name of the drawing from which the path was
         * read.
         */
        string name = "";

        /*!
         * The initial coordinate for the BezCurvePath.
         */
        pair<float, float> initialCoordinate = make_pair (0.0f, 0.0f);

        /*!
         * A list of the BezCurves that make up the full BezCurvePath.
         */
        list<BezCurve> curves;

        /*!
         * A scaling factor that's used to convert the path into mm.
         */
        float scale = 1.0f;

        /*!
         * A null BezCurvePath is one which has no curves. If curves
         * is empty then the BezCurvePath is null.
         */
        bool isNull (void) const {
            return this->curves.empty();
        }

        /*!
         * Set scaling on all member Bezier curves.
         */
        void setScale (const float s) {
            this->scale = s;
            this->initialCoordinate.first = this->initialCoordinate.first * this->scale;
            this->initialCoordinate.second = this->initialCoordinate.second * this->scale;
            list<BezCurve>::iterator i = this->curves.begin();
            while (i != this->curves.end()) {
                i->setScale (this->scale);
                ++i;
            }
        }

        /*!
         * Add a curve to this->curves.
         */
        void addCurve (BezCurve& c) {
            this->curves.push_back (c);
        }

        /*!
         * Output for debugging.
         */
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

        /*!
         * Save to a file for debugging. Using distance step which is
         * assumed to have been pre-scaled - step is in mm, not in SVG
         * drawing units.
         */
        void save (float step) const {
            ofstream f;
            string fname = this->name + ".csv";
            f.open (fname.c_str(), std::ios::out|std::ios::trunc);
            if (f.is_open()) {
                list<BezCurve>::const_iterator i = this->curves.begin();
                // Don't forget to set the scaling factor in each
                // BezCurve before generating points:
                while (i != this->curves.end()) {
                    f << i->output (step);
                    ++i;
                }
                f.close();
            }
        }

        static pair<float,float> getCentroid (const vector<BezCoord>& points) {
            float c_x = 0.0f;
            float c_y = 0.0f;
            for (const BezCoord& i : points) {
                c_x += i.x();
                c_y += i.y();
            }
            c_x = c_x / points.size();
            c_y = c_y / points.size();

            return make_pair (c_x, c_y);
        }

        /*!
         * Crunch the numbers to generate the coordinates for the
         * path, doing the right thing between curves (skipping
         * remaining, then advancing step-remaining into the next
         * curve and so on).
         *
         * If invertY is true, then multiply all the y values in the
         * coordinates by -1. SVG is encoded in a left hand coordinate
         * system, so if you're going to plot the BezCoord points in a
         * right hand system, set invertY to true.
         */
        vector<BezCoord> getPoints (float step, bool invertY = false) const {
            vector<BezCoord> rtn;

            // First the very start point:
            BezCoord startPt = this->curves.front().computePoint (0.0f);
            if (invertY) {
                startPt.invertY();
            }
            rtn.push_back (startPt);

            list<BezCurve>::const_iterator i = this->curves.begin();
            // Don't forget to set the scaling factor in each
            // BezCurve before generating points:
            float firstl = 0.0f;
            while (i != this->curves.end()) {
                vector<BezCoord> cp = i->computePoints (step, firstl);
                if (cp.back().isNull()) {
                    firstl = step - cp.back().getRemaining();
                    cp.pop_back();
                }
                if (invertY) {
                    vector<BezCoord>::iterator bci = cp.begin();
                    while (bci != cp.end()) {
                        bci->invertY();
                        ++bci;
                    }
                }
                rtn.insert (rtn.end(), cp.begin(), cp.end());
                ++i;
            }
            return rtn;
        }
    };

} // namespace morph

#endif // _BEZCURVEPATH_H_
