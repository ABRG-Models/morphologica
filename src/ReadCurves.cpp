/*!
 * ReadCurves implementation
 */

#include "ReadCurves.h"
#include "BezCurve.h"
#include <stdexcept>
#include <sstream>

using std::runtime_error;
using std::stringstream;
using std::make_pair;

using rapidxml::xml_node;
using rapidxml::xml_attribute;
using rapidxml::parse_declaration_node;
using rapidxml::parse_no_data_nodes;

using morph::BezCurve;

morph::ReadCurves::ReadCurves (const string& svgpath)
{
    // Read (without parsing) the svg file text into memory:
    this->modeldata.read (svgpath);
    // Parse the XML and find the root node:
    this->init();
    // Read the curves:
    this->read();
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
    // Search each layer - these are called <g> elements in the SVG.
    this->first_g_node = this->root_node->first_node("g");
    //xml_node<>* g_node = this->first_g_node;
    for (xml_node<>* g_node = this->first_g_node;//g_node = this->root_node->first_node("g");
         g_node;
         g_node = g_node->next_sibling("g")) {
        this->readG (g_node);
    }

}

void
morph::ReadCurves::readG (xml_node<>* g_node)
{
    // Within each <g>: Read the id attribute, then search out <path>
    // elements and read those.
    string g_id("");
    xml_attribute<>* id_attr;
    if ((id_attr = g_node->first_attribute ("id"))) {
        g_id = id_attr->value();
    } // else failed to get g_id

    if (g_id.empty()) {
        throw runtime_error ("Found a <g> element without an id attribute (i.e. a layer without a name)");
    }

    // Parse paths:
    for (xml_node<>* path_node = g_node->first_node("path");
         path_node;
         path_node = path_node->next_sibling("path")) {
        this->readPath (path_node, g_id);
    }

    // Parse lines:
    for (xml_node<>* line_node = g_node->first_node("line");
         line_node;
         line_node = line_node->next_sibling("line")) {
        this->readLine (line_node, g_id);
    }
}

void
morph::ReadCurves::readPath (xml_node<>* path_node, const string& layerName)
{
    string d("");
    xml_attribute<>* d_attr;
    if ((d_attr = path_node->first_attribute ("d"))) {
        d = d_attr->value();
    } // else failed to get d

    if (d.empty()) {
        throw runtime_error ("Found a <path> element without a d attribute");
    }

    std::cout << "Path commands for layer " << layerName << ": " << d << std::endl;

    list<BezCurve> curves = this->parseD (d);
    if (layerName == "cortex") {
        this->corticalPath = curves;
    } else {
        this->enclosedRegions.push_back (make_pair(layerName, curves));
    }
}

list<BezCurve>
morph::ReadCurves::parseD (const string& d)
{
    list<BezCurve> curves;

    // Text parsing time!

    return curves;
}

void
morph::ReadCurves::readLine (xml_node<>* line_node, const string& layerName)
{
    string x1("");
    xml_attribute<>* x1_attr;
    if ((x1_attr = line_node->first_attribute ("x1"))) {
        x1 = x1_attr->value();
    } // else failed to get x1
    if (x1.empty()) {
        throw runtime_error ("Found a <line> element without a x1 attribute");
    }
    string x2("");
    xml_attribute<>* x2_attr;
    if ((x2_attr = line_node->first_attribute ("x2"))) {
        x2 = x2_attr->value();
    } // else failed to get x2
    if (x2.empty()) {
        throw runtime_error ("Found a <line> element without a x2 attribute");
    }
    string y1("");
    xml_attribute<>* y1_attr;
    if ((y1_attr = line_node->first_attribute ("y1"))) {
        y1 = y1_attr->value();
    } // else failed to get y1
    if (y1.empty()) {
        throw runtime_error ("Found a <line> element without a y1 attribute");
    }
    string y2("");
    xml_attribute<>* y2_attr;
    if ((y2_attr = line_node->first_attribute ("y2"))) {
        y2 = y2_attr->value();
    } // else failed to get y2
    if (y2.empty()) {
        throw runtime_error ("Found a <line> element without a y2 attribute");
    }

    // Now do something with x1,y1,x2,y2
    std::cout << "line: (" << x1 << "," << y1 << ") to (" << x2 << "," << y2 << ")" << std::endl;
}

list<BezCurve>
morph::ReadCurves::getCorticalPath (void)
{
    return this->corticalPath;
}

list<BezCurve>
morph::ReadCurves::getEnclosedRegion (const string& structName)
{
    list<BezCurve> nullrtn;
    list<pair<string, list<BezCurve> > >::const_iterator i = this->enclosedRegions.begin();
    while (i != this->enclosedRegions.end()) {
        if (i->first == structName) {
            return i->second;
        }
        ++i;
    }
    return nullrtn;
}

list<pair<string, list<BezCurve> > >
morph::ReadCurves::getEnclosedRegions (void)
{
    return this->enclosedRegions;
}
