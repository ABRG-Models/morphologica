/*
 * Gierer-Meinhardt Turing like RD system
 *
 * FIXME: Needs to be re-written to work with morph::Visual. i.e. This won't work unless
 * you compile against the tag/display0 branch of morphologica.
 */

/*!
 * This will be passed as the template argument for RD_classes and
 * should be defined when compiling.
 */
#ifndef FLOATTYPE
// Check CMakeLists.txt to change to double or float
# error "Please define FLOATTYPE when compiling (hint: See CMakeLists.txt)"
#endif

/*!
 * General STL includes
 */
#include <iostream>
#include <fstream>
#include <vector>
#include <list>
#include <string>
#include <limits>

/*!
 * Include the reaction diffusion class
 */
#include "rd_gierermeinhardt.h"

#ifdef COMPILE_PLOTTING
/*!
 * Include display and plotting code
 */
# include "morph/display.h"
# include "morph/RD_Plot.h"
#endif

/*!
 * Included for directory manipulation code
 */
#include "morph/tools.h"

/*!
 * I'm using JSON to read in simulation parameters
 */
#include <json/json.h>

using namespace std;

/*!
 * main(): Run a simulation, using parameters obtained from a JSON
 * file.
 *
 * Open and read a simple JSON file which contains the parameters for
 * the simulation, such as number of guidance molecules (M), guidance
 * parameters (probably get M from these) and so on.
 *
 * Sample JSON:
 * {
 *   // Overall parameters
 *   "steps":5000,                // Number of steps to simulate for
 *   "logevery":20,               // Log data every logevery steps.
 *   "svgpath":"./ellipse.svg",   // The boundary shape to use
 *   "hextohex_d":0.01,           // Hex to hex distance, determines num hexes
 *   "boundaryFalloffDist":0.01,
 *   "DA":0.1,                    // Diffusion constant
 * }
 *
 * A file containing JSON similar to the above should be saved and its
 * path provided as the only argument to the binary here.
 */
int main (int argc, char **argv)
{
    // Randomly set the RNG seed
    srand (morph::Tools::randomSeed());

    if (argc < 2) {
        cerr << "Usage: " << argv[0] << " /path/to/params.json [/path/to/logdir]" << endl;
        return 1;
    }
    string paramsfile (argv[1]);

    /*
     * Set up JSON code for reading the parameters
     */

    // Test for existence of the JSON file.
    ifstream jsonfile_test;
    int srtn = system ("pwd");
    if (srtn) {
        cerr << "system call returned " << srtn << endl;
    }
    jsonfile_test.open (paramsfile, ios::in);
    if (jsonfile_test.is_open()) {
        // Good, file exists.
        jsonfile_test.close();
    } else {
        cerr << "json config file " << paramsfile << " not found." << endl;
        return 1;
    }

    // Parse the JSON
    ifstream jsonfile (paramsfile, ifstream::binary);
    Json::Value root;
    string errs;
    Json::CharReaderBuilder rbuilder;
    rbuilder["collectComments"] = false;
    bool parsingSuccessful = Json::parseFromStream (rbuilder, jsonfile, &root, &errs);
    if (!parsingSuccessful) {
        // report to the user the failure and their locations in the document.
        cerr << "Failed to parse JSON: " << errs;
        return 1;
    }

    /*
     * Get simulation-wide parameters from JSON
     */
    const unsigned int steps = root.get ("steps", 1000).asUInt();
    if (steps == 0) {
        cerr << "Not much point simulating 0 steps! Exiting." << endl;
        return 1;
    }
    const unsigned int logevery = root.get ("logevery", 100).asUInt();
    if (logevery == 0) {
        cerr << "Can't log every 0 steps. Exiting." << endl;
        return 1;
    }
    const float hextohex_d = root.get ("hextohex_d", 0.01).asFloat();
    const float boundaryFalloffDist = root.get ("boundaryFalloffDist", 0.01).asFloat();
    const string svgpath = root.get ("svgpath", "./ellipse.svg").asString();
    bool overwrite_logs = root.get ("overwrite_logs", false).asBool();
    string logpath = root.get ("logpath", "logs/james1").asString();
    if (argc == 3) {
        string argpath(argv[2]);
        cerr << "Overriding the config-given logpath " << logpath << " with " << argpath << endl;
        logpath = argpath;
        if (overwrite_logs == true) {
            cerr << "WARNING: You set a command line log path.\n"
                 << "       : Note that the parameters config permits the program to OVERWRITE LOG\n"
                 << "       : FILES on each run (\"overwrite_logs\" is set to true)." << endl;
        }
    }
    const FLOATTYPE dt = root.get ("dt", 0.00001).asDouble();

    // model parameters
    const FLOATTYPE D_A = root.get ("D_A", 0.1).asDouble();
    const FLOATTYPE D_B = root.get ("D_A", 0.1).asDouble();
    const FLOATTYPE k1 = root.get ("k1", 1).asDouble();
    const FLOATTYPE k2 = root.get ("k2", 1).asDouble();
    const FLOATTYPE k3 = root.get ("k3", 1).asDouble();
    const FLOATTYPE k4 = root.get ("k4", 1).asDouble();
    const FLOATTYPE k5 = root.get ("k5", 1).asDouble();

    cout << "steps to simulate: " << steps << endl;

#ifdef COMPILE_PLOTTING

    // Parameters from the config that apply only to plotting:
    const unsigned int plotevery = root.get ("plotevery", 10).asUInt();

    // Should the plots be saved as png images?
    const bool saveplots = root.get ("saveplots", false).asBool();

    // If true, then write out the logs in consecutive order numbers,
    // rather than numbers that relate to the simulation timestep.
    const bool vidframes = root.get ("vidframes", false).asBool();
    unsigned int framecount = 0;

    // Create some displays
    vector<morph::Gdisplay> displays;
    vector<double> fix(3, 0.0);
    vector<double> eye(3, 0.0);
    eye[2] = 0.12; // This also acts as a zoom. more +ve to zoom out, more -ve to zoom in.
    vector<double> rot(3, 0.0);

    // A plot object.
    morph::RD_Plot<FLOATTYPE> plt(fix, eye, rot);

    double rhoInit = 1; // This is effectively a zoom control. Increase to zoom out.
    double thetaInit = 0.0;
    double phiInit = 0.0;

    string worldName("gm");

    string winTitle = worldName + ": A"; // 0
    displays.push_back (morph::Gdisplay (340, 300, 100, 1800, winTitle.c_str(),
                                         rhoInit, thetaInit, phiInit));
    displays.back().resetDisplay (fix, eye, rot);
    displays.back().redrawDisplay();

    winTitle = worldName + ": B"; // 1
    displays.push_back (morph::Gdisplay (340, 300, 100, 1800, winTitle.c_str(),
                                         rhoInit, thetaInit, phiInit, displays[0].win));
    displays.back().resetDisplay (fix, eye, rot);
    displays.back().redrawDisplay();
#endif

    /*
     * Instantiate and set up the model object
     */
    RD_GM<FLOATTYPE> RD;

    RD.svgpath = svgpath;
    RD.logpath = logpath;

    // Control the size of the hexes, and therefore the number of hexes in the grid
    RD.hextohex_d = hextohex_d;

    // Boundary fall-off distance
    RD.boundaryFalloffDist = boundaryFalloffDist;

    // After setting the first few features, we can set up all the vectors in RD:
    RD.allocate();

    // After allocate(), we can set up parameters:
    RD.set_dt(dt);
    RD.k1 = k1;
    RD.k2 = k2;
    RD.k3 = k3;
    RD.k4 = k4;
    RD.k5 = k5;
    RD.D_A = D_A;
    RD.D_B = D_B;

    // Now parameters are set, call init()
    RD.init();

    /*
     * Now create a log directory if necessary, and exit on any
     * failures.
     */
    if (morph::Tools::dirExists (logpath) == false) {
        morph::Tools::createDir (logpath);
        if (morph::Tools::dirExists (logpath) == false) {
            cerr << "Failed to create the logpath directory "
                 << logpath << " which does not exist."<< endl;
            return 1;
        }
    } else {
        // Directory DOES exist. See if it contains a previous run and
        // exit without overwriting to avoid confusion.
        if (overwrite_logs == false
            && (morph::Tools::fileExists (logpath + "/params.json") == true
                || morph::Tools::fileExists (logpath + "/positions.h5") == true)) {
            cerr << "Seems like a previous simulation was logged in " << logpath << ".\n"
                 << "Please clean it out manually, choose another directory or set\n"
                 << "overwrite_logs to true in your parameters config JSON file." << endl;
            return 1;
        }
    }

    // As RD.allocate() as been called (and log directory has been
    // created/verified ready), positions can be saved to file.
    RD.savePositions();

#ifdef COMPILE_PLOTTING
    // Here's where you'd plot any one-off plots
#endif

    // Start the loop
    bool finished = false;
    while (finished == false) {
        // Step the model
        RD.step();

#ifdef COMPILE_PLOTTING
        if ((RD.stepCount % plotevery) == 0) {
            //DBG("Plot at step " << RD.stepCount);
            plt.scalarfields (displays[0], RD.hg, RD.A);
            plt.scalarfields (displays[1], RD.hg, RD.B);
            //displays[0].redrawDisplay();
            //displays[1].redrawDisplay();

            if (saveplots) {
                if (vidframes) {
                    plt.savePngs (logpath, "A", framecount, displays[0]);
                    plt.savePngs (logpath, "B", framecount, displays[1]);
                    ++framecount;
                } else {
                    plt.savePngs (logpath, "A", RD.stepCount, displays[0]);
                    plt.savePngs (logpath, "B", RD.stepCount, displays[1]);
                }
            }
        }
#endif
        // Save data every 'logevery' steps
        if ((RD.stepCount % logevery) == 0) {
            RD.save();
        }

        if (RD.stepCount > steps) {
            finished = true;
        }
    }

    // Before saving the json, we'll place any additional useful info
    // in there, such as the FLOATTYPE. If float_width is 4, then
    // results were computed with single precision, if 8, then double
    // precision was used. Also save various parameters from the RD system.
    root["float_width"] = (unsigned int)sizeof(FLOATTYPE);
    string tnow = morph::Tools::timeNow();
    root["sim_ran_at_time"] = tnow.substr(0,tnow.size()-1);
    root["hextohex_d"] = RD.hextohex_d;
    root["D_A"] = RD.D_A;
    root["D_B"] = RD.D_B;
    root["k1"] = RD.k1;
    root["k2"] = RD.k2;
    root["k3"] = RD.k3;
    root["k4"] = RD.k4;
    root["dt"] = RD.get_dt();

    // We'll save a copy of the parameters for the simulation in the
    // log directory as params.json
    const string paramsCopy = logpath + "/params.json";
    ofstream paramsConf;
    paramsConf.open (paramsCopy.c_str(), ios::out|ios::trunc);
    if (paramsConf.is_open()) {
        paramsConf << root;
        paramsConf.close();
    } else {
        cerr << "Warning: Failed to open file to write a copy of the params.json" << endl;
    }

#ifdef COMPILE_PLOTTING
    // Ask for a keypress before exiting so that the final images can be studied
    int a;
    cout << "Press any key[return] to exit.\n";
    cin >> a;
#endif

    return 0;
};
