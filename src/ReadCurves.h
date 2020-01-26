/*!
 * A class for reading SVG files containing paths defining the outline of a neocortex.
 *
 * Author: Seb James
 * Date: July 2018
 */

#ifndef _READCURVES_H_
#define _READCURVES_H_

#include <string>
#include <list>
#include <vector>
#include <map>
#include <utility>
#include "rapidxml.hpp"
#include "BezCurvePath.h"
#include "AllocAndRead.h"

using std::string;
using std::list;
using std::vector;
using std::map;
using std::pair;
using rapidxml::xml_node;
using rapidxml::xml_document;

namespace morph
{
    /*!
     * Read a .svg file containing information about curves in a neocortex. The SVG file should
     * conform to the format which we agreed in Davis, when Drew created trial.svg as a sample
     * drawing of a curve outline.
     */
    class ReadCurves
    {
    public: // methods

        /*!
         * Default constructor does nothing, but client code then has to call init(const string&).
         */
        ReadCurves () {}

        /*!
         * Construct using the SVG file at svgpath. The text of the file is read into memory and the
         * XML root node is found (an <svg> element). All initialisation is done; not need to call
         * init(const string& svgpath)
         */
        ReadCurves (const string& svgpath);

        /*!
         * Initialise using the SVG file at svgpath. The text of the file is read into memory and
         * the XML root node is found (an <svg> element). If you constructed with a const string&
         * svgpath, then you don't need to call this init function.
         */
        void init (const string& svgpath);

        /*!
         * Get the cortical path as a list of BezCurves
         */
        BezCurvePath<float> getCorticalPath (void) const;

        /*!
         * Get the path of an enclosed structure by name, as a list of BezCurves.
         */
        BezCurvePath<float> getEnclosedRegion (const string& structName) const;

        /*!
         * Get all the paths of enclosed structures. This is a list of pairs, in which the name and
         * the structure path are the two parts of the pair.
         */
        list<BezCurvePath<float>> getEnclosedRegions (void) const;

        /*!
         * Save the paths to named files, with the step size being approximately step in Cartesian
         * space along the path.
         */
        void save (float step = 1.0f) const;

        /*!
         * Get the scaling in mm per SVG unit.
         */
        float getScale_mmpersvg (void);

        /*!
         * Get the scaling in SVG units per mm.
         */
        float getScale_svgpermm (void);

        /*!
         * A key-value list of coordinates, obtained from reading any circles in the SVG. The ID of
         * the circle is the key, the location of its centre gives the coordinate. I dreamed this
         * scheme up so that I could incorporate a set of coordinates marking out structures in the
         * cortex (specifically, barrels).
         */
        map<string, pair<float, float>> circles;

    private:

        /*!
         * Some initialisation - parse the doc and find the root node.
         */
        void init (void);

        /*!
         * Do the work of reading the file and populating corticalPath, enclosedRegions and
         * lineToMillimetres.
         */
        void read (void);

        /*!
         * Read a <g> element; its id attribute and its enclosed <path> or <line> element.
         */
        void readG (xml_node<>* g_node);

        /*!
         * Read a circle element and store it in circles.
         */
        void readCircle (xml_node<>* circ_node);

        /*!
         * Recursively search in a_node until a node with the tag name
         * @tagname is found. Return it.
         */
        xml_node<>* findNodeRecursive (xml_node<>* a_node, const string& tagname) const;

        /*!
         * Read a <path> element. A path will contain a series of commands in a d attribute. These
         * commands can be interpreted as a series of Bezier curves, and the Beziers can be expanded
         * out into BezCoords. I should save the Bezier curves in this class object, and generate
         * the coordinates when the client code requests it, possibly providing scaling information
         * at that time.
         *
         * layerName contains the name of the layer in which the path was found.
         */
        void readPath (xml_node<>* path_node, const string& layerName);

        /*!
         * If g_id contains the string "mm", then treat it as a scale bar. If it contains "cortex",
         * then treat it as the special outer/main boundary
         */
        void setupScaling (const string& g_id);

        /*!
         * Read a <path> element, assuming that it contains an implicit set of lines encoded as
         * moveto commands.
         */
        float readPathAsLine (xml_node<>* path_node);

        /*!
         * Split up a string of SVG command numbers. These are delimited either by a comma, a space
         * or by a minus sign. Interpret them as floats and return in a vector.
         *
         * This version also sets this->lastCmd where appropriate, and ensures that numParams are
         * extracted from s. When s contains a longer than numParams list of numbers, endOfCmd is
         * set to point to the end of the commands read into the return value.
         */
        vector<float> splitSvgCmdString (const string& s, char cmd,
                                         unsigned int numParams,
                                         string::size_type& endOfCmd);

        /*!
         * This parses the d attribute string in an SVG path. I'm assuming this will always be a
         * list of Bezier Curves.
         *
         * NB: The SVG is encoded in a left-hand coordinate system, with x positive right and y
         * positive down. This parsing does not change that coordinate system, and so the BezCoords
         * in the path may need to have their y coordinates reversed.
         */
        BezCurvePath<float> parseD (const string& d);

        /*!
         * Read a <line> element. Read x1,y1,x2,y2 attributes from which line length can be
         * determined and lineToMillimetres populated.
         */
        void readLine (xml_node<>* line_node, const string& layerName);

        /*!
         * Set up the scaling in all BezCurvePaths based on lineToMillimetres. Do this after file
         * has been read.
         */
        void setScale (void);

        /*!
         * The neocortical path.
         */
        BezCurvePath<float> corticalPath;

        /*!
         * Init to false, set true if we find the "cortex" layer in the svg file.
         */
        bool gotCortex = false;

        /*!
         * A list of paths marking out structures within the neocortex.
         */
        list<BezCurvePath<float>> enclosedRegions;

        /*!
         * To hold the scale bar line.
         */
        BezCurvePath<float> linePath;

        /*!
         * lineToMillimetres.first is the length of the line in the units of the SVG
         * file. lineToMillimeteres.second is the length in mm that the line represents.
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
         * Records the last command. Used when a string of identical commands needs to be parsed by
         * parseD.
         */
        char lastCmd = '\0';
    };

} // namespace morph

#endif // _READCURVES_H_
