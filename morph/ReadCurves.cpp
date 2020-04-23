/*!
 * ReadCurves implementation
 */

#include "ReadCurves.h"
#include "BezCurvePath.h"
using morph::BezCurvePath;
#include <stdexcept>
using std::runtime_error;
#include <sstream>
using std::stringstream;
#include <vector>
using std::vector;
#include <list>
using std::list;
#include <math.h>
#include "tools.h"
using morph::Tools;
#include <cstdlib>
using std::atof;
#include <utility>
using std::pair;
using std::make_pair;
#include <string>
using std::string;
#include <iostream>
using std::cerr;
using std::endl;
// To enable debug cout messages:
//#define DEBUG 1
//#define DEBUG2 1
#define DBGSTREAM std::cout
#include "MorphDbg.h"
#include "rapidxml.hpp"
using rapidxml::xml_node;
using rapidxml::xml_attribute;
using rapidxml::parse_declaration_node;
using rapidxml::parse_no_data_nodes;

morph::ReadCurves::ReadCurves (const string& svgpath)
{
    // Read (without parsing) the svg file text into memory:
    this->modeldata.read (svgpath);
    // Parse the XML and find the root node:
    this->init();
    // Read the curves:
    this->read();
    if (this->gotCortex == false) {
        cerr << "WARNING: No object in SVG with id \"cortex\". Cortical boundary will be null." << endl;
    } // else DID get cortex ID
}

void
morph::ReadCurves::init (const string& svgpath)
{
    // Read (without parsing) the svg file text into memory:
    this->modeldata.read (svgpath);
    // Parse the XML and find the root node:
    this->init();
    this->read();
    if (this->gotCortex == false) {
        cerr << "WARNING: No object in SVG with id \"cortex\". Cortical boundary will be null." << endl;
    } // else DID get cortex ID
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

    // Search un-enclosed paths, as well as those enclosed in <g> elements
    for (xml_node<>* path_node = this->root_node->first_node("path");
         path_node;
         path_node = path_node->next_sibling("path")) {
        // Un-enclosed paths will need to use their id attribute
        string p_id("");
        xml_attribute<>* path_id_attr;
        if ((path_id_attr = path_node->first_attribute ("id"))) {
            p_id = path_id_attr->value();
            DBG("Un-enclosed path id is " << p_id);
            this->readPath (path_node, p_id);
        } // else failed to get p_id
    }

    // Search circles, and make up a table of all the circles along with their IDs
    for (xml_node<>* circ_node = this->root_node->first_node("circle");
         circ_node;
         circ_node = circ_node->next_sibling("circle")) {
        this->readCircle (circ_node);
    }

    // Now the file is read, set the scaling:
    this->setScale();
}

xml_node<>*
morph::ReadCurves::findNodeRecursive (xml_node<>* a_node, const string& tagname) const
{
    DBG2 ("Called for tag '" << tagname << "'");
    for (xml_node<>* path_node = a_node->first_node();
         path_node;
         path_node = path_node->next_sibling()) {

        DBG2("path_node->name(): " << path_node->name() << " compare with " << tagname);
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
morph::ReadCurves::readCircle (xml_node<>* circ_node)
{
    // Within each <circle>: Read the id attribute
    string circ_id("");
    xml_attribute<>* _attr;
    if ((_attr = circ_node->first_attribute ("id"))) {
        circ_id = _attr->value();
        bool gotx = false;
        bool goty = false;
        float cx = 0.0;
        float cy = 0.0;
        // Now, get the x and y attributes, cx and cy
        if ((_attr = circ_node->first_attribute ("cx"))) {
            gotx = true;
            cx = atof (_attr->value());
        }
        if ((_attr = circ_node->first_attribute ("cy"))) {
            goty = true;
            cy = atof (_attr->value());
        }
        if (gotx && goty) {
            DBG("Added circle " << circ_id << " with centre (" << cx << "," << cy << ")");
            this->circles[circ_id] = make_pair(cx, cy);
        }

    } // else failed to get circ_id
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

#if 0
    if (g_id.empty()) {
        throw runtime_error ("Found a <g> element without an id attribute (i.e. a layer without a name)");
    }
#endif

    // Recursively search down any number of levels until a <path>
    // node is found. Read it, then continue searching.
    xml_node<>* path_node = g_node;
    do {
        path_node = this->findNodeRecursive (path_node, "path");

        if (path_node != (xml_node<>*)0) {
            // See if path has an id that isn't the generic "path0000"
            // format. If so, use this to override the id from the <g>
            // element
            string p_id("");
            xml_attribute<>* path_id_attr;
            DBG("Check path id attribute...");
            if ((path_id_attr = path_node->first_attribute ("id"))) {
                p_id = path_id_attr->value();
                DBG("path id is " << p_id);
            } // else failed to get p_id
            if (!p_id.empty()) {
                if (!(p_id.find("path") == 0)) {
                    // p_id doesn't start with "path", so write it into g_id
                    DBG2 ("Write p_id=" << p_id << " into g_id");
                    g_id = p_id;
                }
            }
            this->readPath (path_node, g_id);

            path_node = path_node->next_sibling();

            if (path_node != (xml_node<>*)0) {
                // Check path_node, itself before passing to findNodeRecursive...
                if ((path_id_attr = path_node->first_attribute ("id"))) {
                    p_id = path_id_attr->value();
                    DBG("path id is " << p_id);
                } // else failed to get p_id
                if (!p_id.empty()) {
                    if (!(p_id.find("path") == 0)) {
                        // p_id doesn't start with "path", so write it into g_id
                        DBG2 ("Write p_id=" << p_id << " into g_id");
                        g_id = p_id;
                    }
                }
                this->readPath (path_node, g_id);
            }
        }

    } while (path_node != (xml_node<>*)0);

    // Search for a line element
    xml_node<>* line_node = this->findNodeRecursive (g_node, "line");
    if (line_node != (xml_node<>*)0) {
        if (this->foundLine == true) {
            cerr << "WARNING: Found a second <line> element in this SVG, was only expecting one (as a single scale bar)" << endl;
        }
        this->readLine (line_node, g_id);
        this->foundLine = true;
    }
}

void
morph::ReadCurves::setupScaling (const string& g_id)
{
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

    BezCurvePath<float> curves = this->parseD (d);
    curves.name = layerName;
    if (layerName == "cortex") {
        this->gotCortex = true;
        this->corticalPath = curves;
    } else if (layerName.find ("mm") != string::npos) {
        this->linePath = curves;
        this->setupScaling (layerName);
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
            if (false == (p1>0 && s[p1-1] == 'e')) {
                p0 = p1; // Not +1 so that we include the - in the next one
            } else {
                // This '-' character followed an exponent 'e' character, so it does not denote a
                // delimiter between values in the string s. So pop off the incomplete number
                // string which will just have been pushed back, decrement numnum and carry on.
                numbers.pop_back();
                --numnum;
            }
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
        if (!ss.str().empty()) {
            ss >> n;
            numbers.push_back (n);
            ++numnum;
        }
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

BezCurvePath<float>
morph::ReadCurves::parseD (const string& d)
{
    BezCurvePath<float> curves;

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
    const char* svgCmds = "mMcCsSqQtTzZlLhHvV";

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
        DBG("switch (" << cmd << ")");
        switch (cmd) { // switch on the command character

        case 'L': // lineto command, absolution coordinates
        case 'l': // lineto command, deltas
        {
            p2 = d.find_first_of (svgCmds, p1+1);
            string lCmd = d.substr (p1+1, p2-p1-1);
            if (Tools::containsOnlyWhitespace (lCmd)) {
                p3 = lCmd.size()-1;
            } else {
                vector<float> v = this->splitSvgCmdString (lCmd, cmd, 10000, p3);
                if (v.size()%2 != 0) {
                    throw runtime_error ("Unexpected size of SVG path L command (expected pairs of numbers)");
                }
                for (unsigned int i = 0; i<v.size(); i+=2) {
                    if (cmd == 'l') { // delta coordinates
                        f = make_pair (currentCoordinate.first + v[i],
                                       currentCoordinate.second + v[i+1]);
                    } else {
                        f = make_pair (v[i], v[i+1]);
                    }
                    BezCurve<float> c(currentCoordinate, f);
                    curves.addCurve (c);
                    currentCoordinate = f;
                }
            }
            break;
        }

        case 'H': // horizontal lineto command, absolution coordinates
        case 'h': // horizontal lineto command, deltas
        {
            p2 = d.find_first_of (svgCmds, p1+1);
            string lCmd = d.substr (p1+1, p2-p1-1);
            if (Tools::containsOnlyWhitespace (lCmd)) {
                p3 = lCmd.size()-1;
            } else {
                vector<float> v = this->splitSvgCmdString (lCmd, cmd, 10000, p3);
                if (v.size() == 0) {
                    throw runtime_error ("Unexpected size of SVG path H command (expected at least one number)");
                }
                for (unsigned int i = 0; i<v.size(); ++i) {
                    if (cmd == 'h') { // delta coordinates
                        f = make_pair (currentCoordinate.first + v[i],
                                       currentCoordinate.second);
                    } else {
                        f = make_pair (v[i], currentCoordinate.second);
                    }
                    BezCurve<float> c(currentCoordinate, f);
                    curves.addCurve (c);
                    currentCoordinate = f;
                }
            }
            break;
        }

        case 'V': // vertical lineto command, absolution coordinates
        case 'v': // vertical lineto command, deltas
        {
            p2 = d.find_first_of (svgCmds, p1+1);
            string lCmd = d.substr (p1+1, p2-p1-1);
            if (Tools::containsOnlyWhitespace (lCmd)) {
                p3 = lCmd.size()-1;
            } else {
                vector<float> v = this->splitSvgCmdString (lCmd, cmd, 10000, p3);
                if (v.size() == 0) {
                    throw runtime_error ("Unexpected size of SVG path V command (expected at least one number)");
                }
                DBG("v.size(): " << v.size());
                for (unsigned int i = 0; i<v.size(); ++i) {
                    if (cmd == 'v') { // delta coordinates
                        if (v[i] != 0.0f) {
                            f = make_pair (currentCoordinate.first,
                                           currentCoordinate.second + v[i]);
                            BezCurve<float> c(currentCoordinate, f);
                            curves.addCurve (c);
                            currentCoordinate = f;
                        }
                    } else {
                        f = make_pair (currentCoordinate.first, v[i]);
                        BezCurve<float> c(currentCoordinate, f);
                        curves.addCurve (c);
                        currentCoordinate = f;
                    }
                }
            }
            break;
        }

        case 'M': // move command, absolution coordinates
        case 'm': // move command, deltas
        {
            p2 = d.find_first_of (svgCmds, p1+1);
            string mCmd = d.substr (p1+1, p2-p1-1);
            DBG("mCmd: " << mCmd);
            if (Tools::containsOnlyWhitespace (mCmd)) {
                p3 = mCmd.size()-1;
            } else {
                vector<float> v = this->splitSvgCmdString (mCmd, cmd, 10000, p3);
                DBG("v.size() for M command: " << v.size());
                if (v.size()%2 != 0) {
                    throw runtime_error ("Unexpected size of SVG path M command (expected pairs of numbers)");
                }

                if (cmd == 'm') { // delta coordinates
                    currentCoordinate = make_pair (currentCoordinate.first + v[0],
                                                   currentCoordinate.second + v[1]);
                } else {
                    currentCoordinate = make_pair (v[0], v[1]);
                }
                firstCoordinate = currentCoordinate;
                curves.initialCoordinate = currentCoordinate;

                if (v.size() == 2) {
                    // Just 2 coords means it's a move command; nothing further to do
                    DBG("Just 2 coords in M command");
                } else {
                    DBG("coord pairs in M command");
                    // pairs of commands implies linetos.
                    for (unsigned int i = 2; i<v.size(); i+=2) {
                        if (cmd == 'm') { // delta coordinates
                            f = make_pair (currentCoordinate.first + v[i],
                                           currentCoordinate.second + v[i+1]);
                        } else {
                            f = make_pair (v[i], v[i+1]);
                        }
                        BezCurve<float> c(currentCoordinate, f);
                        DBG("curves.addCurve...");
                        curves.addCurve (c);
                        currentCoordinate = f;
                    }
                }
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
                BezCurve<float> c(currentCoordinate, f, c1, c2);
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
                BezCurve<float> c(currentCoordinate, f, c1, c2);
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
                BezCurve<float> c(currentCoordinate, firstCoordinate);
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
    BezCurve<float> linecurve (p1, p2);
    this->linePath.reset();
    this->linePath.initialCoordinate = p1;
    this->linePath.addCurve (linecurve);

    this->setupScaling (layerName);
}

void
morph::ReadCurves::setScale (void)
{
    if (this->lineToMillimetres.second == 0.0f) {
        throw runtime_error ("Failed to obtain scaling from the scale bar.");
    }
    this->corticalPath.setScale (this->lineToMillimetres.second);
    typename list<BezCurvePath<float>>::iterator ei = this->enclosedRegions.begin();
    while (ei != this->enclosedRegions.end()) {
        ei->setScale (this->lineToMillimetres.second);
        ++ei;
    }
    // Scale the centre points of the circles:
    for (auto& c : this->circles) {
        c.second.first *= this->lineToMillimetres.second;
        c.second.second *= this->lineToMillimetres.second;
        DBG ("ID " << c.first << " (" << c.second.first << "," << c.second.second << ")");
    }
}

float
morph::ReadCurves::getScale_mmpersvg (void)
{
    /*
     * lineToMillimetres.first is the length of the line in the
     * units of the SVG file. lineToMillimeteres.second is the
     * length in mm that the line represents.
     */
    float scale = lineToMillimetres.second/lineToMillimetres.first;
    return scale;
}

float
morph::ReadCurves::getScale_svgpermm (void)
{
    float scale = lineToMillimetres.first/lineToMillimetres.second;
    return scale;
}

BezCurvePath<float>
morph::ReadCurves::getCorticalPath (void) const
{
    return this->corticalPath;
}

BezCurvePath<float>
morph::ReadCurves::getEnclosedRegion (const string& structName) const
{
    BezCurvePath<float> nullrtn;
    typename list<BezCurvePath<float>>::const_iterator i = this->enclosedRegions.begin();
    while (i != this->enclosedRegions.end()) {
        if (i->name == structName) {
            return *i;
        }
        ++i;
    }
    return nullrtn;
}

list<BezCurvePath<float>>
morph::ReadCurves::getEnclosedRegions (void) const
{
    return this->enclosedRegions;
}

void
morph::ReadCurves::save (float step) const
{
    this->corticalPath.save (step);
    typename list<BezCurvePath<float>>::const_iterator i = this->enclosedRegions.begin();
    while (i != this->enclosedRegions.end()) {
        i->save (step);
        ++i;
    }
}
