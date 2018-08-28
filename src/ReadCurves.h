/*!
 * A class for reading SVG files containing paths defining the outline
 * of a neocortex.
 *
 * Author: Seb James
 * Date: July 2018
 */

#ifndef _READCURVES_H_
#define _READCURVES_H_

#include <string>
#include <list>
#include <vector>
#include <utility>
#include "rapidxml.hpp"
#include "BezCurvePath.h"
#include "AllocAndRead.h"

using std::string;
using std::list;
using std::vector;
using std::pair;
using rapidxml::xml_node;
using rapidxml::xml_document;

namespace morph
{
    /*!
     * Read a .svg file containing information about curves in a
     * neocortex. The SVG file should conform to the format which we
     * agreed in Davis, when Drew created trial.svg as a sample
     * drawing of a curve outline.
     */
    class ReadCurves
    {
    public: // methods

        /*!
         * Construct using the SVG file at svgpath. The text of the
         * file is read into memory and the XML root node is found (an
         * <svg> element)
         */
        ReadCurves (const string& svgpath);

        /*!
         * Get the cortical path as a list of BezCurves
         */
        BezCurvePath getCorticalPath (void) const;

        /*!
         * Get the path of an enclosed structure by name, as a list of
         * BezCurves.
         */
        BezCurvePath getEnclosedRegion (const string& structName) const;

        /*!
         * Get all the paths of enclosed structures. This is a list of
         * pairs, in which the name and the structure path are the two
         * parts of the pair.
         */
        list<BezCurvePath> getEnclosedRegions (void) const;

        /*!
         * Save the paths to named files, with the step size being
         * approximately step in Cartesian space along the path.
         */
        void save (float step = 1.0f) const;

    private:

        /*!
         * Some initialisation - parse the doc and find the root node.
         */
        void init (void);

        /*!
         * Do the work of reading the file and populating
         * corticalPath, enclosedRegions and lineToMillimetres.
         */
        void read (void);

        /*!
         * Read a <g> element; its id attribute and its enclosed
         * <path> or <line> element.
         */
        void readG (xml_node<>* g_node);

        /*!
         * Read a <path> element. A path will contain a series of
         * commands in a d attribute. These commands can be
         * interpreted as a series of Bezier curves, and the Beziers
         * can be expanded out into BezCoords. I should save the
         * Bezier curves in this class object, and generate the
         * coordinates when the client code requests it, possibly
         * providing scaling information at that time.
         *
         * layerName contains the name of the layer in which the path was
         * found.
         */
        void readPath (xml_node<>* path_node, const string& layerName);

        /*!
         * Split up a string of SVG command numbers. These are
         * delimited either by a comma, a space or by a minus
         * sign. Interpret them as floats and return in a vector.
         *
         * This version also sets this->lastCmd where appropriate, and
         * ensures that numParams are extracted from s. When s
         * contains a longer than numParams list of numbers, endOfCmd
         * is set to point to the end of the commands read into the
         * return value.
         */
        vector<float> splitSvgCmdString (const string& s, char cmd,
                                         unsigned int numParams,
                                         string::size_type& endOfCmd);

        /*!
         * This parses the d attribute string in an SVG path. I'm
         * assuming this will always be a list of Bezier Curves.
         *
         * NB: The SVG is encoded in a left-hand coordinate system,
         * with x positive right and y positive down. This parsing
         * does not change that coordinate system, and so the
         * BezCoords in the path may need to have their y coordinates
         * reversed.
         */
        BezCurvePath parseD (const string& d);

        /*!
         * Read a <line> element. Read x1,y1,x2,y2 attributes from
         * which line length can be determined and lineToMillimetres
         * populated.
         */
        void readLine (xml_node<>* line_node, const string& layerName);

        /*!
         * Set up the scaling in all BezCurvePaths based on
         * lineToMillimetres. Do this after file has been read.
         */
        void setScale (void);

        /*!
         * The neocortical path.
         */
        BezCurvePath corticalPath;

        /*!
         * A list of paths marking out structures within the
         * neocortex.
         */
        list<BezCurvePath> enclosedRegions;

        /*!
         * lineToMillimeteres.first is the length of the line in the
         * units of the SVG file. lineToMillimeteres.second is the
         * length in mm that the line represents.
         */
        pair<float, float> lineToMillimetres;

        /*!
         * Set to true once a line was found to set lineToMillimetres.
         */
        bool foundLine = false;

        /*!
         * An object into which to read the xml text prior to parsing.
         */
        morph::AllocAndRead modeldata;

        /*!
         * Main xml_document object.
         */
        xml_document<> doc;

        /*!
         * the root node pointer.
         */
        xml_node<>* root_node = static_cast<xml_node<>*>(0);

        /*!
         * Records the last command. Used when a string of identical
         * commands needs to be parsed by parseD.
         */
        char lastCmd = '\0';
    };

} // namespace morph

#endif // _READCURVES_H_
