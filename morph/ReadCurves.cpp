/*!
 * ReadCurves implementation
 */

#include "morph/ReadCurves.h"
#include "morph/BezCurvePath.h"
#include <stdexcept>
#include <sstream>
#include <vector>
#include <list>
#include <math.h>
#include "tools.h"
#include <cstdlib>
#include <utility>
#include <string>
#include <iostream>
#include "morph/MorphDbg.h"
#include "rapidxml.hpp"

morph::ReadCurves::ReadCurves (const std::string& svgpath)
{
    // Read (without parsing) the svg file text into memory:
    this->modeldata.read (svgpath);
    // Parse the XML and find the root node:
    this->init();
    // Read the curves:
    this->read();
    if (this->gotCortex == false) {
        std::cerr << "WARNING: No object in SVG with id \"cortex\". Cortical boundary will be null.\n";
    } // else DID get cortex ID
}

void
morph::ReadCurves::init (const std::string& svgpath)
{
    // Read (without parsing) the svg file text into memory:
    this->modeldata.read (svgpath);
    // Parse the XML and find the root node:
    this->init();
    this->read();
    if (this->gotCortex == false) {
        std::cerr << "WARNING: No object in SVG with id \"cortex\". Cortical boundary will be null.\n";
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
        this->doc.parse<rapidxml::parse_declaration_node|rapidxml::parse_no_data_nodes>(this->modeldata.data());

        // Get the root node.
        this->root_node = this->doc.first_node ("svg");
        if (!this->root_node) {
            std::stringstream ee;
            ee << "No root node 'svg'!";
            throw std::runtime_error (ee.str());
        }
    }
}

void
morph::ReadCurves::read (void)
{
    // Search each layer - these are called <g> elements in the SVG.
    for (rapidxml::xml_node<>* g_node = this->root_node->first_node("g");
         g_node;
         g_node = g_node->next_sibling("g")) {
        this->readG (g_node);
    }

    // Search un-enclosed paths, as well as those enclosed in <g> elements
    for (rapidxml::xml_node<>* path_node = this->root_node->first_node("path");
         path_node;
         path_node = path_node->next_sibling("path")) {
        // Un-enclosed paths will need to use their id attribute
        std::string p_id("");
        rapidxml::xml_attribute<>* path_id_attr;
        if ((path_id_attr = path_node->first_attribute ("id"))) {
            p_id = path_id_attr->value();
            DBG("Un-enclosed path id is " << p_id);
            this->readPath (path_node, p_id);
        } // else failed to get p_id
    }

    // Search circles, and make up a table of all the circles along with their IDs
    for (rapidxml::xml_node<>* circ_node = this->root_node->first_node("circle");
         circ_node;
         circ_node = circ_node->next_sibling("circle")) {
        this->readCircle (circ_node);
    }

    // Now the file is read, set the scaling:
    this->setScale();
}

rapidxml::xml_node<>*
morph::ReadCurves::findNodeRecursive (rapidxml::xml_node<>* a_node, const std::string& tagname) const
{
    DBG2 ("Called for tag '" << tagname << "'");
    for (rapidxml::xml_node<>* path_node = a_node->first_node();
         path_node;
         path_node = path_node->next_sibling()) {

        DBG2("path_node->name(): " << path_node->name() << " compare with " << tagname);
        if (strncmp (path_node->name(), tagname.c_str(), tagname.size()) == 0) {
            DBG2("return path_node");
            return path_node;
        }

        rapidxml::xml_node<>* rtn_node = findNodeRecursive (path_node, tagname);
        if (rtn_node != (rapidxml::xml_node<>*)0 && strncmp (rtn_node->name(), tagname.c_str(), tagname.size()) == 0) {
            DBG2("return rtn_node");
            return rtn_node;
        }
    }

    return (rapidxml::xml_node<>*)0;
}

void
morph::ReadCurves::readCircle (rapidxml::xml_node<>* circ_node)
{
    // Within each <circle>: Read the id attribute
    std::string circ_id("");
    rapidxml::xml_attribute<>* _attr;
    if ((_attr = circ_node->first_attribute ("id"))) {
        circ_id = _attr->value();
        bool gotx = false;
        bool goty = false;
        float cx = 0.0;
        float cy = 0.0;
        // Now, get the x and y attributes, cx and cy
        if ((_attr = circ_node->first_attribute ("cx"))) {
            gotx = true;
            cx = std::atof (_attr->value());
        }
        if ((_attr = circ_node->first_attribute ("cy"))) {
            goty = true;
            cy = std::atof (_attr->value());
        }
        if (gotx && goty) {
            DBG("Added circle " << circ_id << " with centre (" << cx << "," << cy << ")");
            this->circles[circ_id] = std::make_pair(cx, cy);
        }

    } // else failed to get circ_id
}

void
morph::ReadCurves::readG (rapidxml::xml_node<>* g_node)
{
    // Within each <g>: Read the id attribute, then search out <path>
    // elements and read those.
    std::string g_id("");
    rapidxml::xml_attribute<>* id_attr;
    if ((id_attr = g_node->first_attribute ("id"))) {
        g_id = id_attr->value();
    } // else failed to get g_id
    DBG("readG called, g id is " << g_id);

#if 0
    if (g_id.empty()) {
        throw std::runtime_error ("Found a <g> element without an id attribute (i.e. a layer without a name)");
    }
#endif

    // Recursively search down any number of levels until a <path>
    // node is found. Read it, then continue searching.
    rapidxml::xml_node<>* path_node = g_node;
    do {
        path_node = this->findNodeRecursive (path_node, "path");

        if (path_node != (rapidxml::xml_node<>*)0) {
            // See if path has an id that isn't the generic "path0000"
            // format. If so, use this to override the id from the <g>
            // element
            std::string p_id("");
            rapidxml::xml_attribute<>* path_id_attr;
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

            if (path_node != (rapidxml::xml_node<>*)0) {
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

    } while (path_node != (rapidxml::xml_node<>*)0);

    // Search for a line element
    rapidxml::xml_node<>* line_node = this->findNodeRecursive (g_node, "line");
    if (line_node != (rapidxml::xml_node<>*)0) {
        if (this->foundLine == true) {
            std::cerr << "WARNING: Found a second <line> element in this SVG, was only expecting one (as a single scale bar)\n";
        }
        this->readLine (line_node, g_id);
        this->foundLine = true;
    }
}

void
morph::ReadCurves::setupScaling (const std::string& g_id)
{
    if (g_id.find("mm") != std::string::npos) {
        // Parse lines. Note that Inkscape will save a line as a path with
        // implicit lineto in the form of a path with 2 pairs of
        // coordinates in a move command. Adobe Illustrator uses a <line>
        // element.

        // Extract the length of the line in mm from the layer name
        // _x33_mm means .33 mm
        std::string mm(g_id);
        morph::Tools::searchReplace ("x", ".", mm);
        morph::Tools::searchReplace ("_", "", mm);
        morph::Tools::searchReplace ("m", "", mm);
        DBG ("mm string is now: " << mm);
        float mmf = std::atof (mm.c_str());
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
morph::ReadCurves::readPath (rapidxml::xml_node<>* path_node, const std::string& layerName)
{
    std::string d("");
    rapidxml::xml_attribute<>* d_attr;
    if ((d_attr = path_node->first_attribute ("d"))) {
        d = d_attr->value();
    } // else failed to get d

    if (d.empty()) {
        throw std::runtime_error ("Found a <path> element without a d attribute");
    }

    DBG ("Path commands for layer " << layerName << ": " << d);

    morph::BezCurvePath<float> curves = this->parseD (d);
    curves.name = layerName;
    if (layerName == "cortex") {
        this->gotCortex = true;
        this->corticalPath = curves;
    } else if (layerName.find ("mm") != std::string::npos) {
        this->linePath = curves;
        this->setupScaling (layerName);
    } else {
        this->enclosedRegions.push_back (curves);
    }
}

std::vector<float>
morph::ReadCurves::splitSvgCmdString (const std::string& s, char cmd, unsigned int numParams, std::string::size_type& endOfCmd)
{
    std::vector<float> numbers;
    unsigned int numnum = 0; // number of numbers stored in numbers
    float n = 0.0f;

    std::string::size_type p0 = 0;
    std::string::size_type p1 = s.find_first_of ("-, ", p0);
    while (p1 != std::string::npos && numnum < numParams) {

        if (p1 != p0) {
            std::stringstream ss;
            ss << s.substr(p0, p1-p0);
            DBG2 ("ss.str(): " << ss.str() << " length " << ss.str().size());
            std::string ccs = ss.str();
            if (morph::Tools::containsOnlyWhitespace (ccs)) {
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

    if (p1 == std::string::npos) {
        // No minus signs. Attempt to convert s into a single float and return
        DBG2 ("Doing one more...");
        std::stringstream ss;
        ss << s.substr (p0, p1-p0);
        DBG2 ("Last one: ss.str(): " << ss.str());
        if (!ss.str().empty()) {
            ss >> n;
            numbers.push_back (n);
            ++numnum;
        }
    }

    if (numnum > numParams) {
        throw std::runtime_error ("splitSvgCmdString: unexpected number of params in command.");
    }

    endOfCmd = p1;
    DBG2 ("splitSvgCmdString: endOfCmd: " << endOfCmd << " s.size(): " << s.size());

    if (endOfCmd == std::string::npos) {
        this->lastCmd = '\0';
    } else {
        this->lastCmd = cmd;
    }

    return numbers;
}

morph::BezCurvePath<float>
morph::ReadCurves::parseD (const std::string& d)
{
    morph::BezCurvePath<float> curves;

    // As we parse through the path, we have to keep track of the
    // current coordinate position, as curves are specified from the
    // position at the end of the previous curve.
    std::pair<float, float> currentCoordinate = std::make_pair (0.0f, 0.0f);

    // The first coordinate of the path. Can be required with a Z
    // command.
    std::pair<float, float> firstCoordinate = std::make_pair (0.0f, 0.0f);

    // The last Bezier control points, c2, especially may be required
    // in a shortcut Bezier command (s or S), hence declaring these
    // outside the scope of the while loop.
    std::pair<float, float> c1; // Control point 1
    std::pair<float, float> c2; // Control point 2
    std::pair<float, float> f;  // Final point of curve

    // A list of SVG command characters
    const char* svgCmds = "mMcCsSqQtTzZlLhHvV";

    // Text parsing time!
    std::string::size_type p0 = 0;
    std::string::size_type p1 = d.find_first_of (svgCmds, p0);
    std::string::size_type p2 = 0;
    std::string::size_type p3 = std::string::npos;
    this->lastCmd = '\0';
    char cmd = '\0';
    while (p1 != std::string::npos) {

        // if lastCmd == '\0' switch on d, else switch on lastCmd.
        if (lastCmd == '\0') {
            cmd = d[p1];
        } else {
            cmd = this->lastCmd;
        }

        p3 = std::string::npos;
        DBG("switch (" << cmd << ")");
        switch (cmd) { // switch on the command character

        case 'L': // lineto command, absolution coordinates
        case 'l': // lineto command, deltas
        {
            p2 = d.find_first_of (svgCmds, p1+1);
            std::string lCmd = d.substr (p1+1, p2-p1-1);
            if (morph::Tools::containsOnlyWhitespace (lCmd)) {
                p3 = lCmd.size()-1;
            } else {
                std::vector<float> v = this->splitSvgCmdString (lCmd, cmd, 10000, p3);
                if (v.size()%2 != 0) {
                    throw std::runtime_error ("Unexpected size of SVG path L command (expected pairs of numbers)");
                }
                for (unsigned int i = 0; i<v.size(); i+=2) {
                    if (cmd == 'l') { // delta coordinates
                        f = std::make_pair (currentCoordinate.first + v[i],
                                       currentCoordinate.second + v[i+1]);
                    } else {
                        f = std::make_pair (v[i], v[i+1]);
                    }
                    morph::BezCurve<float> c(currentCoordinate, f);
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
            std::string lCmd = d.substr (p1+1, p2-p1-1);
            if (morph::Tools::containsOnlyWhitespace (lCmd)) {
                p3 = lCmd.size()-1;
            } else {
                std::vector<float> v = this->splitSvgCmdString (lCmd, cmd, 10000, p3);
                if (v.size() == 0) {
                    throw std::runtime_error ("Unexpected size of SVG path H command (expected at least one number)");
                }
                for (unsigned int i = 0; i<v.size(); ++i) {
                    if (cmd == 'h') { // delta coordinates
                        f = std::make_pair (currentCoordinate.first + v[i],
                                       currentCoordinate.second);
                    } else {
                        f = std::make_pair (v[i], currentCoordinate.second);
                    }
                    morph::BezCurve<float> c(currentCoordinate, f);
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
            std::string lCmd = d.substr (p1+1, p2-p1-1);
            if (morph::Tools::containsOnlyWhitespace (lCmd)) {
                p3 = lCmd.size()-1;
            } else {
                std::vector<float> v = this->splitSvgCmdString (lCmd, cmd, 10000, p3);
                if (v.size() == 0) {
                    throw std::runtime_error ("Unexpected size of SVG path V command (expected at least one number)");
                }
                DBG("v.size(): " << v.size());
                for (unsigned int i = 0; i<v.size(); ++i) {
                    if (cmd == 'v') { // delta coordinates
                        if (v[i] != 0.0f) {
                            f = std::make_pair (currentCoordinate.first,
                                           currentCoordinate.second + v[i]);
                            morph::BezCurve<float> c(currentCoordinate, f);
                            curves.addCurve (c);
                            currentCoordinate = f;
                        }
                    } else {
                        f = std::make_pair (currentCoordinate.first, v[i]);
                        morph::BezCurve<float> c(currentCoordinate, f);
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
            std::string mCmd = d.substr (p1+1, p2-p1-1);
            DBG("mCmd: " << mCmd);
            if (morph::Tools::containsOnlyWhitespace (mCmd)) {
                p3 = mCmd.size()-1;
            } else {
                std::vector<float> v = this->splitSvgCmdString (mCmd, cmd, 10000, p3);
                DBG("v.size() for M command: " << v.size());
                if (v.size()%2 != 0) {
                    throw std::runtime_error ("Unexpected size of SVG path M command (expected pairs of numbers)");
                }

                if (cmd == 'm') { // delta coordinates
                    currentCoordinate = std::make_pair (currentCoordinate.first + v[0],
                                                   currentCoordinate.second + v[1]);
                } else {
                    currentCoordinate = std::make_pair (v[0], v[1]);
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
                            f = std::make_pair (currentCoordinate.first + v[i],
                                           currentCoordinate.second + v[i+1]);
                        } else {
                            f = std::make_pair (v[i], v[i+1]);
                        }
                        morph::BezCurve<float> c(currentCoordinate, f);
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
            std::string cCmd = d.substr (p1+1, p2-p1-1); // cCmd may be either a single command (6 params) or a great long line.
            DBG2 ("cCmd: '" << cCmd << "' with p1=" << p1);
            if (morph::Tools::containsOnlyWhitespace (cCmd)) {
                p3 = cCmd.size()-1;
            } else {
                std::vector<float> v = this->splitSvgCmdString (cCmd, cmd, 6, p3);
                DBG2 ("After splitSvgCmdString, lastCmd: " << this->lastCmd << " and p3: " << p3 << " which means p1+1+p3: " << p1+1+p3 << " d[]=" << d[p1+1+p3]);
                if (v.size() != 6) {
                    std::stringstream ee;
                    ee << "Unexpected size of SVG path C command (expected 6 numbers, got " << v.size() << ")";
                    throw std::runtime_error (ee.str());
                }
                if (cmd == 'c') { // delta coordinates
                    c1 = std::make_pair(currentCoordinate.first + v[0], currentCoordinate.second + v[1]);
                    c2 = std::make_pair(currentCoordinate.first + v[2], currentCoordinate.second + v[3]);
                    f = std::make_pair(currentCoordinate.first + v[4], currentCoordinate.second + v[5]);
                } else { // 'C', so absolute coordinates were given
                    c1 = std::make_pair (v[0],v[1]);
                    c2 = std::make_pair (v[2],v[3]);
                    f = std::make_pair (v[4],v[5]);
                }
                morph::BezCurve<float> c(currentCoordinate, f, c1, c2);
                curves.addCurve (c);
                currentCoordinate = f;
            }
            break;
        }

        case 'S': // shortcut cubic Bezier, absolute coordinates
        case 's': // shortcut cubic Bezier, deltas
        {
            p2 = d.find_first_of (svgCmds, p1+1);
            std::string sCmd = d.substr (p1+1, p2-p1-1);
            DBG2 ("sCmd: " << sCmd);
            if (morph::Tools::containsOnlyWhitespace (sCmd)) {
                p3 = sCmd.size()-1;
            } else {
                std::vector<float> v = this->splitSvgCmdString (sCmd, cmd, 4, p3);
                if (v.size() != 4) {
                    throw std::runtime_error ("Unexpected size of SVG path S command (expected 4 numbers)");
                }
                // c2 and currentCoordinate are stored locally in abs. coordinates:
                c1.first = 2 * currentCoordinate.first - c2.first;
                c1.second = 2 * currentCoordinate.second - c2.second;
                if (d[p1] == 's') { // delta coordinates
                    // Deltas are determined from the currentCoordinate
                    c2 = std::make_pair(currentCoordinate.first + v[0], currentCoordinate.second + v[1]);
                    f = std::make_pair(currentCoordinate.first + v[2], currentCoordinate.second + v[3]);
                } else { // 'S', so absolute coordinates were given
                    c2 = std::make_pair (v[0],v[1]);
                    f = std::make_pair (v[2],v[3]);
                }
                morph::BezCurve<float> c(currentCoordinate, f, c1, c2);
                curves.addCurve (c);
                currentCoordinate = f;
            }
            break;
        }

        case 'Q':
        case 'q': // Quadratic Bezier
        {
            throw std::runtime_error ("Quadratic Bezier is unimplemented");
            break;
        }
        case 'T':
        case 't': // Shortcut quadratic Bezier
        {
            throw std::runtime_error ("Shortcut quadratic Bezier is unimplemented");
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

        if (p3 == std::string::npos) {
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
morph::ReadCurves::readLine (rapidxml::xml_node<>* line_node, const std::string& layerName)
{
    std::string x1("");
    rapidxml::xml_attribute<>* x1_attr;
    if ((x1_attr = line_node->first_attribute ("x1"))) {
        x1 = x1_attr->value();
    } // else failed to get x1
    if (x1.empty()) {
        throw std::runtime_error ("Found a <line> element without a x1 attribute");
    }
    std::string x2("");
    rapidxml::xml_attribute<>* x2_attr;
    if ((x2_attr = line_node->first_attribute ("x2"))) {
        x2 = x2_attr->value();
    } // else failed to get x2
    if (x2.empty()) {
        throw std::runtime_error ("Found a <line> element without a x2 attribute");
    }
    std::string y1("");
    rapidxml::xml_attribute<>* y1_attr;
    if ((y1_attr = line_node->first_attribute ("y1"))) {
        y1 = y1_attr->value();
    } // else failed to get y1
    if (y1.empty()) {
        throw std::runtime_error ("Found a <line> element without a y1 attribute");
    }
    std::string y2("");
    rapidxml::xml_attribute<>* y2_attr;
    if ((y2_attr = line_node->first_attribute ("y2"))) {
        y2 = y2_attr->value();
    } // else failed to get y2
    if (y2.empty()) {
        throw std::runtime_error ("Found a <line> element without a y2 attribute");
    }

    // Now do something with x1,y1,x2,y2
    DBG2 ("line: (" << x1 << "," << y1 << ") to (" << x2 << "," << y2 << ")");

    // Create a BezCurve object then add this to this->linePath
    std::pair<float,float> p1 = std::make_pair (std::atof (x1.c_str()), std::atof (y1.c_str()));
    std::pair<float,float> p2 = std::make_pair (std::atof (x2.c_str()), std::atof (y2.c_str()));
    morph::BezCurve<float> linecurve (p1, p2);
    this->linePath.reset();
    this->linePath.initialCoordinate = p1;
    this->linePath.addCurve (linecurve);

    this->setupScaling (layerName);
}

void
morph::ReadCurves::setScale (void)
{
    if (this->lineToMillimetres.second == 0.0f) {
        throw std::runtime_error ("Failed to obtain scaling from the scale bar.");
    }
    this->corticalPath.setScale (this->lineToMillimetres.second);
    typename std::list<morph::BezCurvePath<float>>::iterator ei = this->enclosedRegions.begin();
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

morph::BezCurvePath<float>
morph::ReadCurves::getCorticalPath (void) const
{
    return this->corticalPath;
}

morph::BezCurvePath<float>
morph::ReadCurves::getEnclosedRegion (const std::string& structName) const
{
    morph::BezCurvePath<float> nullrtn;
    typename std::list<morph::BezCurvePath<float>>::const_iterator i = this->enclosedRegions.begin();
    while (i != this->enclosedRegions.end()) {
        if (i->name == structName) {
            return *i;
        }
        ++i;
    }
    return nullrtn;
}

std::list<morph::BezCurvePath<float>>
morph::ReadCurves::getEnclosedRegions (void) const
{
    return this->enclosedRegions;
}

void
morph::ReadCurves::save (float step) const
{
    this->corticalPath.save (step);
    typename std::list<morph::BezCurvePath<float>>::const_iterator i = this->enclosedRegions.begin();
    while (i != this->enclosedRegions.end()) {
        i->save (step);
        ++i;
    }
}
