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
#include "BezCoord.h"
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
        ReadCurves (const string& svgpath);
        ~ReadCurves ();

        void read (void);

    private:

        /*!
         * Some initialisation - parse the doc and find the root node.
         */
        void init (void);

        /*!
         * The neocortical path.
         */
        list<BezCoord> corticalPath;

        /*!
         * A list of paths marking out structures within the
         * neocortex.
         */
        list<pair<string, list<BezCoord> > > enclosedRegions;

        /*!
         * lineToMillimeteres.first is the length of the line in the
         * units of the SVG file. lineToMillimeteres.second is the
         * length in mm that the line represents.
         */
        pair<float, float> lineToMillimetres;

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
        xml_node<>* root_node;
    };

} // namespace morph

#endif _READCURVES_H_
