/*!
 * \file
 * \brief Bezier curve path class (path made of Bezier curves).
 * \author Seb James
 * \date 2019-2020
 */

#pragma once

#include "morph/BezCurve.h"

#include <limits>
#include <list>
#include <vector>
#include <utility>
#include <string>
#include <iostream>
#include <fstream>
#include <cmath>

namespace morph
{
    /*!
     * A class defining a path made up of Bezier curves. This has an
     * initial position, and then a list of curves that make up the
     * path. I've kept this very simple with all public member
     * attributes.
     */
    template <typename Flt>
    class BezCurvePath
    {
    public:
        /*!
         * The name of this BezCurvePath. This is intended to be taken
         * from the layer name of the drawing from which the path was
         * read.
         */
        std::string name = "";

        /*!
         * The initial coordinate for the BezCurvePath.
         */
        std::pair<Flt, Flt> initialCoordinate = std::make_pair (Flt{0}, Flt{0});

        /*!
         * A list of the BezCurves that make up the full BezCurvePath.
         */
        std::list<BezCurve<Flt>> curves;

        /*!
         * A scaling factor that's used to convert the path into mm.
         */
        Flt scale = Flt{1};

        /*!
         * This can be filled with a set of points on the path made up by the Bezier
         * curves. Do so with computePoints.
         */
        std::vector<BezCoord<Flt>> points;

        /*!
         * As for points, store tangents and normals.
         */
        //@{
        std::vector<BezCoord<Flt>> tangents;
        std::vector<BezCoord<Flt>> normals;
        //@}

        /*!
         * A null BezCurvePath is one which has no curves. If curves
         * is empty then the BezCurvePath is null.
         */
        bool isNull (void) const {
            return this->curves.empty();
        }

        /*!
         * Reset this BezCurvePath
         */
        void reset (void) {
            this->curves.clear();
            this->initialCoordinate = std::make_pair (Flt{0}, Flt{0});
            this->scale = Flt{1};
            this->name = "";
        }

        /*!
         * Set scaling on all member Bezier curves.
         */
        void setScale (const Flt s) {
            this->scale = s;
            this->initialCoordinate.first = this->initialCoordinate.first * this->scale;
            this->initialCoordinate.second = this->initialCoordinate.second * this->scale;
            typename std::list<BezCurve<Flt>>::iterator i = this->curves.begin();
            while (i != this->curves.end()) {
                i->setScale (this->scale);
                ++i;
            }
        }

        /*!
         * Add a curve to this->curves.
         */
        void addCurve (BezCurve<Flt>& c) {
            if (c.getOrder() == 0) {
                std::cout << "Not adding 0th order curve." << std::endl;
            } else {
                if (this->curves.empty()) {
                    this->initialCoordinate = c.getInitialPointScaled();
                }
                this->curves.push_back (c);
            }
        }

        void removeCurve (void) {
            if (!this->curves.empty()) {
                this->curves.pop_back();
            }
        }

        /*!
         * Output for debugging.
         */
        void output (void) const {
            std::cout << "------ BezCurvePath ------" << std::endl;
            std::cout << "Name: " << this->name << std::endl;
            std::cout << "Initial coord: (" << this->initialCoordinate.first
                      << "," << this->initialCoordinate.second << ")" << std::endl;
            std::cout << "Number of curves: " << this->curves.size() << std::endl;
            typename std::list<BezCurve<Flt>>::const_iterator i = this->curves.begin();
            while (i != this->curves.end()) {
                std::cout << i->output(static_cast<unsigned int>(20));
                ++i;
            }
            std::cout << "------ End BezCurvePath ------" << std::endl;
        }

        /*!
         * Save to a file for debugging. Using distance step which is
         * assumed to have been pre-scaled - step is in mm, not in SVG
         * drawing units.
         */
        void save (Flt step) const {
            std::ofstream f;
            std::string fname = this->name + ".csv";
            f.open (fname.c_str(), std::ios::out|std::ios::trunc);
            if (f.is_open()) {
                typename std::list<BezCurve<Flt>>::const_iterator i = this->curves.begin();
                // Don't forget to set the scaling factor in each
                // BezCurve before generating points:
                while (i != this->curves.end()) {
                    f << i->output (step);
                    ++i;
                }
                f.close();
            }
        }

        /*!
         * Compute the as-the-crow-flies distance from the initial
         * coordinate of this BezCurvePath to the final
         * coordinate. Uses the scale factor.
         */
        Flt getEndToEnd (void) const {
            // Distance from this->initialCoordinate to:
            if (this->curves.empty()) {
                return Flt{0};
            }
            std::pair<Flt,Flt> cend = this->curves.back().getFinalPointScaled();
            Flt dx = cend.first - initialCoordinate.first;
            Flt dy = cend.second - initialCoordinate.second;
            return std::sqrt (dx * dx + dy * dy);
        }

        /*!
         * Compute the centroid of the passed in set of positions.
         */
        static std::pair<Flt,Flt> getCentroid (const std::vector<BezCoord<Flt>>& points) {
            Flt c_x = Flt{0};
            Flt c_y = Flt{0};
            for (const BezCoord<Flt>& i : points) {
                c_x += i.x();
                c_y += i.y();
            }
            c_x = c_x / points.size();
            c_y = c_y / points.size();

            return std::make_pair (c_x, c_y);
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
        void computePoints (Flt step, bool invertY = false) {

            this->points.clear();
            this->tangents.clear();
            this->normals.clear();

            // First the very start point:
            BezCoord<Flt> startPt = this->curves.front().computePoint (Flt{0});
            if (invertY) {
                startPt.invertY();
            }
            this->points.push_back (startPt);

            // Make cp a complete set of points for the current curve *including
            // the point in the curve for t=0*
            std::pair<BezCoord<Flt>, BezCoord<Flt>> tn0 = this->curves.front().computeTangentNormal(Flt{0});
            this->tangents.push_back (tn0.first);
            this->normals.push_back (tn0.second);

            typename std::list<BezCurve<Flt>>::const_iterator i = this->curves.begin();
            // Don't forget to set the scaling factor in each
            // BezCurve before generating points:
            Flt firstl = Flt{0};
            while (i != this->curves.end()) {
                std::vector<BezCoord<Flt>> cp = i->computePoints (step, firstl);
                if (cp.back().isNull()) {
                    firstl = step - cp.back().getRemaining();
                    cp.pop_back();
                }
                if (invertY) {
                    typename std::vector<BezCoord<Flt>>::iterator bci = cp.begin();
                    while (bci != cp.end()) {
                        bci->invertY();
                        ++bci;
                    }
                }
                this->points.insert (this->points.end(), cp.begin(), cp.end());

                // Now compute tangents and normals
                for (BezCoord<Flt> bp : cp) {
                    std::pair<BezCoord<Flt>, BezCoord<Flt>> tn = i->computeTangentNormal(bp.t());
                    this->tangents.push_back (tn.first);
                    this->normals.push_back (tn.second);
                }
                ++i;
            }
        }

        //! Getters
        //@{
        std::vector<BezCoord<Flt>> getPoints (void) const {
            return this->points;
        }
        std::vector<BezCoord<Flt>> getTangents (void) const {
            return this->tangents;
        }
        std::vector<BezCoord<Flt>> getNormals (void) const {
            return this->normals;
        }
        //@}

        /*!
         * Similar to the above, but ensure that there are @nPoints evenly spaced
         * points along the curve. @invertY has the same meaning as in the other
         * overload of this function.
         */
        void computePoints (unsigned int nPoints, bool invertY = false) {
            // Get end-to-end distance and compute a candidate step, then call other
            // overload.
            if (nPoints == 0) {
                std::cout << "nPoints should be >0, returning" << std::endl;
                return;
            }
            if (this->curves.empty()) {
                std::cout << "Curve is empty, returning" << std::endl;
                return;
            }

            this->points.clear();

            Flt etoe = this->getEndToEnd();
            Flt step = etoe/(nPoints-1);
            unsigned int actualPoints = 0;

            while (actualPoints != nPoints) {
                this->points.clear();
                // std::cout << "Getting points with step size " << step << std::endl;
                this->computePoints (step, invertY);
                actualPoints = this->points.size();
                if (actualPoints != nPoints) {

                    // Modify step
                    Flt steptrial = Flt{0};
                    if (actualPoints > nPoints) {
                        // Increase step size, starting with a doubling, then a half
                        // extra, then a quarter extra, etc
                        actualPoints = 0;
                        Flt stepinc = step;
                        while (actualPoints < nPoints) {
                            steptrial = step + stepinc;
                            this->points.clear();
                            this->computePoints (steptrial, invertY);
                            actualPoints = this->points.size();
                            stepinc /= 2.0f;
                        }

                        if (std::abs(step-steptrial) < std::numeric_limits<Flt>::epsilon()) {
                            std::cout << "Numeric limit reached; can't change step a small "
                                      << "enough amount to change the number of points" << std::endl;
                            return;
                        }
                        step = steptrial;

                    } else { // actualPoints < nPoints
                        // Decrease step size, starting with a halving, then a
                        // quartering until we exceed nPoints
                        actualPoints = 0;
                        Flt stepinc = step/2.0f;
                        while (actualPoints < nPoints) {
                            steptrial = step - stepinc;
                            this->points.clear();
                            this->computePoints (steptrial, invertY);
                            actualPoints = this->points.size();
                            stepinc /= 2.0f;
                        }
                        if (std::abs(step-steptrial) < std::numeric_limits<Flt>::epsilon()) {
                            std::cout << "Numeric limit reached; can't change step a small "
                                      << "enough amount to change the number of points" << std::endl;
                            return;
                        }
                        step = steptrial;
                    }
                }
            }
        }
    };

} // namespace morph
