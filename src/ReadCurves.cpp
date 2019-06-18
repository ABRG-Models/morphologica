/*!
 * ReadCurves implementation
 */

#include "ReadCurves.h"
#include "BezCurvePath.h"
#include <stdexcept>
#include <sstream>
#include <vector>
#include <math.h>
#include "tools.h"
#include <cstdlib>

// To enable debug cout messages:
//#define DEBUG 1
//#define DEBUG2 1
#define DBGSTREAM std::cout
#include "MorphDbg.h"

using std::runtime_error;
using std::stringstream;
using std::make_pair;
using std::vector;
using std::cout;
using std::endl;

using std::atof;

using rapidxml::xml_node;
using rapidxml::xml_attribute;
using rapidxml::parse_declaration_node;
using rapidxml::parse_no_data_nodes;

using morph::BezCurvePath;
using morph::Tools;

morph::ReadCurves::ReadCurves (const string& svgpath)
{
    // Read (without parsing) the svg file text into memory:
    this->modeldata.read (svgpath);
    // Parse the XML and find the root node:
    this->init();
    // Read the curves:
    this->read();
    if (this->gotCortex == false) {
        cout << "WARNING: No object in SVG with id \"cortex\". Cortical boundary will be null." << endl;
    } else {
        cout << "Got a cortex id" << endl;
    }
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
    for (xml_node<>* g_node = this->root_node->first_node("g");
         g_node;
         g_node = g_node->next_sibling("g")) {
        this->readG (g_node);
    }
    // Now the file is read, set the scaling:
    this->setScale();
}

xml_node<>*
morph::ReadCurves::findNodeRecursive (xml_node<>* a_node, const string& tagname) const
{
    DBG2 ("Called for tag " << tagname);
    for (xml_node<>* path_node = a_node->first_node();
         path_node;
         path_node = path_node->next_sibling()) {

        DBG2("path_node->name(): " << path_node->name());
        if (strncmp (path_node->name(), tagname.c_str(), tagname.size()) == 0) {
            DBG2("return path_node");
            return path_node;
        }

        xml_node<>* rtn_node = findNodeRecursive (path_node, tagname);
        if (rtn_node != (xml_node<>*)0 && strncmp (rtn_node->name(), tagname.c_str(), tagname.size()) == 0) {
            DBG2("return rtn_node");
            return rtn_node;
        }
    }

    return (xml_node<>*)0;
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
    DBG("readG called, g id is " << g_id);

    if (g_id.empty()) {
        throw runtime_error ("Found a <g> element without an id attribute (i.e. a layer without a name)");
    }

    // Parse paths:
    bool gotpath = false, gotline = false;

    // Recursively search down any number of levels until a <path> node is found
    xml_node<>* path_node = this->findNodeRecursive (g_node, "path");
    if (path_node != (xml_node<>*)0) {
        this->readPath (path_node, g_id);
        gotpath = true;
    }

    // Search for a line element
    DBG2("findNodeRecursive for line...");
    xml_node<>* line_node = this->findNodeRecursive (g_node, "line");
    if (line_node != (xml_node<>*)0) {
        DBG2("readLine(line_node, g_id)");
        this->readLine (line_node, g_id);
        gotline = true;
    }

    // If g_id contains the string "mm", then treat it as a scale
    // bar. If it contains "cortex", then treat it as the special
    // outer/main boundary
    if (g_id.find("mm") != string::npos) {
        // Parse lines. Note that Inkscape will save a line as a path with
        // implicit lineto in the form of a path with 2 pairs of
        // coordinates in a move command. Adobe Illustrator uses a <line>
        // element.

        // Extract the length of the line in mm from the layer name
        // _x33_mm means .33 mm
        string mm(g_id);
        Tools::searchReplace ("x", ".", mm);
        Tools::searchReplace ("_", "", mm);
        Tools::searchReplace ("m", "", mm);
        DBG ("mm string is now: " << mm);
        float mmf = atof (mm.c_str());
        // dl is the length of the scale bar line
        float dl = 0.0f;
        dl = this->linePath.getEndToEnd();
        // Having found the length of the line from the <line> or
        // <path>, compute lineToMillimetres
        this->lineToMillimetres.first = 1;
        this->lineToMillimetres.second = dl > 0.0f ? mmf/dl : 1.0f;
        DBG ("mm per SVG unit: " << this->lineToMillimetres.second);
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

    DBG ("Path commands for layer " << layerName << ": " << d);

    BezCurvePath curves = this->parseD (d);
    curves.name = layerName;
    if (layerName == "cortex") {
        this->gotCortex = true;
        this->corticalPath = curves;
    } else if (layerName.find ("mm") != string::npos) {
        this->linePath = curves;
    } else {
        this->enclosedRegions.push_back (curves);
    }
}

vector<float>
morph::ReadCurves::splitSvgCmdString (const string& s, char cmd, unsigned int numParams, string::size_type& endOfCmd)
{
    vector<float> numbers;
    unsigned int numnum = 0; // number of numbers stored in numbers
    float n = 0.0f;

    string::size_type p0 = 0;
    string::size_type p1 = s.find_first_of ("-, ", p0);
    while (p1 != string::npos && numnum < numParams) {

        if (p1 != p0) {
            stringstream ss;
            ss << s.substr(p0, p1-p0);
            DBG2 ("ss.str(): " << ss.str() << " length " << ss.str().size());
            string ccs = ss.str();
            if (Tools::containsOnlyWhitespace (ccs)) {
                // Do nothing
            } else {
                ss >> n;
                numbers.push_back (n);
                ++numnum;
            }
        }

        if (s[p1] == ',') {
            p0 = p1+1;
        } else if (s[p1] == ' ') {
            p0 = p1+1;
        } else if (s[p1] == '-') {
            p0 = p1; // Not +1 so that we include the - in the next one
        }

        if (numnum < numParams) {
            p1 = s.find_first_of ("-, ", p1+1);
        }
    }

    if (p1 == string::npos) {
        // No minus signs. Attempt to convert s into a single float and return
        DBG2 ("Doing one more...");
        stringstream ss;
        ss << s.substr (p0, p1-p0);
        DBG2 ("Last one: ss.str(): " << ss.str());
        ss >> n;
        numbers.push_back (n);
        ++numnum;
    }

    if (numnum > numParams) {
        throw runtime_error ("splitSvgCmdString: unexpected number of params in command.");
    }

    endOfCmd = p1;
    DBG2 ("splitSvgCmdString: endOfCmd: " << endOfCmd << " s.size(): " << s.size());

    if (endOfCmd == string::npos) {
        this->lastCmd = '\0';
    } else {
        this->lastCmd = cmd;
    }

    return numbers;
}

BezCurvePath
morph::ReadCurves::parseD (const string& d)
{
    DBG2("-----");
    DBG2("NEW parseD call");
    DBG2("-----");
    BezCurvePath curves;

    // As we parse through the path, we have to keep track of the
    // current coordinate position, as curves are specified from the
    // position at the end of the previous curve.
    pair<float, float> currentCoordinate = make_pair (0.0f, 0.0f);

    // The first coordinate of the path. Can be required with a Z
    // command.
    pair<float, float> firstCoordinate = make_pair (0.0f, 0.0f);

    // The last Bezier control points, c2, especially may be required
    // in a shortcut Bezier command (s or S), hence declaring these
    // outside the scope of the while loop.
    pair<float, float> c1; // Control point 1
    pair<float, float> c2; // Control point 2
    pair<float, float> f;  // Final point of curve

    // A list of SVG command characters
    const char* svgCmds = "mMcCsSqQtTzZ";

    // Text parsing time!
    string::size_type p0 = 0;
    string::size_type p1 = d.find_first_of (svgCmds, p0);
    string::size_type p2 = 0;
    string::size_type p3 = string::npos;
    this->lastCmd = '\0';
    char cmd = '\0';
    while (p1 != string::npos) {

        // if lastCmd == '\0' switch on d, else switch on lastCmd.
        if (lastCmd == '\0') {
            cmd = d[p1];
        } else {
            cmd = this->lastCmd;
        }

        p3 = string::npos;

        switch (cmd) { // switch on the command character

        case 'M': // move command, absolution coordinates
        case 'm': // move command, deltas
        {
            p2 = d.find_first_of (svgCmds, p1+1);
            string mCmd = d.substr (p1+1, p2-p1-1);
            if (Tools::containsOnlyWhitespace (mCmd)) {
                p3 = mCmd.size()-1;
            } else {
                vector<float> v = this->splitSvgCmdString (mCmd, cmd, 2, p3);
                if (v.size() != 2) {
                    throw runtime_error ("Unexpected size of SVG path M command (expected 2 numbers)");
                }
                currentCoordinate = make_pair (v[0], v[1]);
                firstCoordinate = currentCoordinate;
                curves.initialCoordinate = currentCoordinate;
            }
            break;
        }

        case 'C': // cubic Bezier curve, abs positions
        case 'c': // cubic Bezier curve, deltas
        {
            DBG2 ("c/C; p1+1 is " << p1+1);
            p2 = d.find_first_of (svgCmds, p1+1);
            string cCmd = d.substr (p1+1, p2-p1-1); // cCmd may be either a single command (6 params) or a great long line.
            DBG2 ("cCmd: '" << cCmd << "' with p1=" << p1);
            if (Tools::containsOnlyWhitespace (cCmd)) {
                p3 = cCmd.size()-1;
            } else {
                vector<float> v = this->splitSvgCmdString (cCmd, cmd, 6, p3);
                DBG2 ("After splitSvgCmdString, lastCmd: " << this->lastCmd << " and p3: " << p3 << " which means p1+1+p3: " << p1+1+p3 << " d[]=" << d[p1+1+p3]);
                if (v.size() != 6) {
                    stringstream ee;
                    ee << "Unexpected size of SVG path C command (expected 6 numbers, got " << v.size() << ")";
                    throw runtime_error (ee.str());
                }
                if (cmd == 'c') { // delta coordinates
                    c1 = make_pair(currentCoordinate.first + v[0], currentCoordinate.second + v[1]);
                    c2 = make_pair(currentCoordinate.first + v[2], currentCoordinate.second + v[3]);
                    f = make_pair(currentCoordinate.first + v[4], currentCoordinate.second + v[5]);
                } else { // 'C', so absolute coordinates were given
                    c1 = make_pair (v[0],v[1]);
                    c2 = make_pair (v[2],v[3]);
                    f = make_pair (v[4],v[5]);
                }
                BezCurve c(currentCoordinate, f, c1, c2);
                curves.addCurve (c);
                currentCoordinate = f;
            }
            break;
        }

        case 'S': // shortcut cubic Bezier, absolute coordinates
        case 's': // shortcut cubic Bezier, deltas
        {
            p2 = d.find_first_of (svgCmds, p1+1);
            string sCmd = d.substr (p1+1, p2-p1-1);
            DBG2 ("sCmd: " << sCmd);
            if (Tools::containsOnlyWhitespace (sCmd)) {
                p3 = sCmd.size()-1;
            } else {
                vector<float> v = this->splitSvgCmdString (sCmd, cmd, 4, p3);
                if (v.size() != 4) {
                    throw runtime_error ("Unexpected size of SVG path S command (expected 4 numbers)");
                }
                // c2 and currentCoordinate are stored locally in abs. coordinates:
                c1.first = 2 * currentCoordinate.first - c2.first;
                c1.second = 2 * currentCoordinate.second - c2.second;
                if (d[p1] == 's') { // delta coordinates
                    // Deltas are determined from the currentCoordinate
                    c2 = make_pair(currentCoordinate.first + v[0], currentCoordinate.second + v[1]);
                    f = make_pair(currentCoordinate.first + v[2], currentCoordinate.second + v[3]);
                } else { // 'S', so absolute coordinates were given
                    c2 = make_pair (v[0],v[1]);
                    f = make_pair (v[2],v[3]);
                }
                BezCurve c(currentCoordinate, f, c1, c2);
                curves.addCurve (c);
                currentCoordinate = f;
            }
            break;
        }

        case 'Q':
        case 'q': // Quadratic Bezier
        {
            throw runtime_error ("Quadratic Bezier is unimplemented");
            break;
        }
        case 'T':
        case 't': // Shortcut quadratic Bezier
        {
            throw runtime_error ("Shortcut quadratic Bezier is unimplemented");
            break;
        }
        case 'Z':
        case 'z': // straight line from current position to first point of path.
        {
            if (currentCoordinate != firstCoordinate) {
                BezCurve c(currentCoordinate, firstCoordinate);
                curves.addCurve (c);
                currentCoordinate = firstCoordinate;
            }
            break;
        }

        default:
            break;
        }

        if (p3 == string::npos) {
            DBG2 ("p3 is string::npos");
            p0 = p1+1;
            p1 = d.find_first_of (svgCmds, p0);
            DBG2 ("Starting from d[" << p0 << "], found next svgCmd at p1=" << p1);
        } else {
            DBG2 ("Setting p1 from p3 = " << p3 << " to p1+1+p3: " << (p1+1+p3) << ". p2, by the way is " << p2);
            p1 = p1+1+p3;
            DBG2 ("new d[p1] is " << d[p1]);
            if (p1+1 == p2) {
                // It's a new command!
                DBG ("Reset lastCmd");
                this->lastCmd = '\0';
            }
        }
    }

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
    DBG2 ("line: (" << x1 << "," << y1 << ") to (" << x2 << "," << y2 << ")");

    // Create a BezCurve object then add this to this->linePath
    pair<float,float> p1 = make_pair (atof (x1.c_str()), atof (y1.c_str()));
    pair<float,float> p2 = make_pair (atof (x2.c_str()), atof (y2.c_str()));
    BezCurve linecurve (p1, p2);
    this->linePath.reset();
    this->linePath.initialCoordinate = p1;
    this->linePath.addCurve (linecurve);

#if 0 // This goes elsewhere
    // Compute the length of the line in the SVG coordinate system
    float dx = atof (x2.c_str()) - atof (x1.c_str());
    float dy = atof (y2.c_str()) - atof (y1.c_str());
    float dl = sqrtf (dx*dx + dy*dy);

    // Extract the length of the line in mm from the layer name
    // _x33_mm means .33 mm
    string mm(layerName);
    Tools::searchReplace ("x", ".", mm);
    Tools::searchReplace ("_", "", mm);
    Tools::searchReplace ("m", "", mm);
    DBG ("mm string is now: " << mm);
    float mmf = atof (mm.c_str());
    this->lineToMillimetres.first = 1;
    this->lineToMillimetres.second = mmf/dl;
    DBG ("mm per SVG unit: " << this->lineToMillimetres.second);
    this->foundLine = true;
#endif
}

void
morph::ReadCurves::setScale (void)
{
    this->corticalPath.setScale (this->lineToMillimetres.second);
    list<BezCurvePath>::iterator ei = this->enclosedRegions.begin();
    while (ei != this->enclosedRegions.end()) {
        ei->setScale (this->lineToMillimetres.second);
        ++ei;
    }
}

BezCurvePath
morph::ReadCurves::getCorticalPath (void) const
{
    return this->corticalPath;
}

BezCurvePath
morph::ReadCurves::getEnclosedRegion (const string& structName) const
{
    BezCurvePath nullrtn;
    list<BezCurvePath>::const_iterator i = this->enclosedRegions.begin();
    while (i != this->enclosedRegions.end()) {
        if (i->name == structName) {
            return *i;
        }
        ++i;
    }
    return nullrtn;
}

list<BezCurvePath>
morph::ReadCurves::getEnclosedRegions (void) const
{
    return this->enclosedRegions;
}

void
morph::ReadCurves::save (float step) const
{
    this->corticalPath.save (step);
    list<BezCurvePath>::const_iterator i = this->enclosedRegions.begin();
    while (i != this->enclosedRegions.end()) {
        i->save (step);
        ++i;
    }
}
