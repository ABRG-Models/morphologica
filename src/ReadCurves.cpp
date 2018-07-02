/*!
 * ReadCurves implementation
 */

#include "ReadCurves.h"
#include <stdexcept>
#include <sstream>

using std::runtime_error;
using std::stringstream;

using namespace rapidxml;

morph::ReadCurves::ReadCurves (const string& svgpath)
    : root_node (static_cast<xml_node<>*>(0))
{
    // Read (without parsing) the svg file text into memory:
    this->modeldata.read (svgpath);
    // Parse the XML and find the root node:
    this->init();
}

morph::ReadCurves::~ReadCurves()
{
}

void
morph::ReadCurves::init (void)
{
    if (!this->root_node) {
        // we are choosing to parse the XML declaration
        // parse_no_data_nodes prevents RapidXML from using the somewhat
        // surprising behaviour of having both values and data nodes, and
        // having data nodes take precedence over values when printing
        // >>> note that this will skip parsing of CDATA nodes <<<
        this->doc.parse<parse_declaration_node | parse_no_data_nodes>(this->modeldata.data());

        // Get the root node.
        this->root_node = this->doc.first_node ("svg");
        if (!this->root_node) {
            stringstream ee;
            ee << "No root node 'svg'!";
            throw runtime_error (ee.str());
        }
    }
}

void
morph::ReadCurves::read (void)
{
}
