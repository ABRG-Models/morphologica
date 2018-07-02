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
#include <utility>
#include "rapidxml.hpp"
#include "BezCurve.h"
#include "AllocAndRead.h"

using std::string;
using std::list;
using std::pair;
using rapidxml::xml_node;
using rapidxml::xml_document;

namespace morph
{
    /*!
     * Read a .svg file containing information about curves in a
     * neocortex.
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
        list<BezCurve> getCorticalPath (void);

        /*!
         * Get the path of an enclosed structure by name, as a list of
         * BezCurves.
         */
        list<BezCurve> getEnclosedRegion (const string& structName);

        /*!
         * Get all the paths of enclosed structures. This is a list of
         * pairs, in which the name and the structure path are the two
         * parts of the pair.
         */
        list<pair<string, list<BezCurve> > > getEnclosedRegions (void);

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
         * <path> element.
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
         * Read a <line> element. x1,y1,x2,y2 attributes from which
         * line length can be determined.
         */
        void readLine (xml_node<>* line_node, const string& layerName);

        /*!
         * The neocortical path.
         */
        list<BezCurve> corticalPath;

        /*!
         * A list of paths marking out structures within the
         * neocortex.
         */
        list<pair<string, list<BezCurve> > > enclosedRegions;

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
         * Do I need this? SpineML_PreFlight/modelpreflight keeps a
         * copy for the first (population) node.
         */
        xml_node<>* first_g_node;

    };

} // namespace morph

#endif // _READCURVES_H_
