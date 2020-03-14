/*
 * A morphologica example: The Schnakenberg Turing-like RD system.
 *
 * Author: Seb James
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
using namespace std;

/*!
 * Include the reaction diffusion class
 */
#include "rd_schnakenberg.h"

#ifdef COMPILE_PLOTTING
/*!
 * If COMPILE_PLOTTING is defined at compile time, then include the display and
 * plotting code.
 */
# include "morph/display.h"
# include "morph/RD_Plot.h"
#endif

/*!
 * Included for directory manipulation code
 */
#include "morph/tools.h"
using morph::Tools;

/*!
 * A jsoncpp-wrapping class for configuration.
 */
#include <morph/Config.h>
using morph::Config;

/*!
 * main(): Run a simulation, using parameters obtained from a JSON file.
 *
 * The path to this JSON file is the only argument required for the program. An
 * example JSON file is provided with this example (see schnak.json).
 */
int main (int argc, char **argv)
{
    if (argc < 2) {
        cerr << "Usage: " << argv[0] << " /path/to/params.json" << endl;
        return 1;
    }
    string paramsfile (argv[1]);

    /*
     * Set up morph::Config (JSON reader/writer) for reading the parameters
     */
    Config conf(paramsfile);
    if (!conf.ready) {
        cerr << "Error setting up JSON config: " << conf.emsg << endl;
        return 1;
    }

    /*
     * Get simulation-wide parameters from JSON
     */
    const unsigned int steps = conf.getUInt ("steps", 1000UL);
    if (steps == 0) {
        cerr << "Not much point simulating 0 steps! Exiting." << endl;
        return 1;
    }
    const unsigned int logevery = conf.getUInt ("logevery", 100UL);

    bool overwrite_logs = conf.getBool ("overwrite_logs", false);

    // Handling of log path requires a few lines of code:
    string logpath = conf.getString ("logpath", "fromfilename");
    string logbase = "";
    if (logpath == "fromfilename") {
        // Using json filename as logpath
        string justfile = paramsfile;
        // Remove trailing .json and leading directories
        vector<string> pth = Tools::stringToVector (justfile, "/");
        justfile = pth.back();
        Tools::searchReplace (".json", "", justfile);
        // Use logbase as the subdirectory into which this should go
        logbase = conf.getString ("logbase", "logs/");
        if (logbase.back() != '/') {
            logbase += '/';
        }
        logpath = logbase + justfile;
    }
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

    const FLOATTYPE dt = static_cast<FLOATTYPE>(conf.getDouble ("dt", 0.00001));

    cout << "steps to simulate: " << steps << endl;

#ifdef COMPILE_PLOTTING

    // Parameters from the config that apply only to plotting:
    const unsigned int plotevery = conf.getUInt ("plotevery", 10);

    // Should the plots be saved as png images?
    const bool saveplots = conf.getBool ("saveplots", false);

    // If true, then write out the logs in consecutive order numbers,
    // rather than numbers that relate to the simulation timestep.
    const bool vidframes = conf.getBool ("vidframes", false);
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

    string worldName("schnak");

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
    RD_Schnakenberg<FLOATTYPE> RD;

    RD.svgpath = ""; // We'll do an elliptical boundary
    RD.ellipse_a = conf.getDouble ("ellipse_a", 0.8);
    RD.ellipse_b = conf.getDouble ("ellipse_b", 1.2);
    RD.logpath = logpath;

    // Control the size of the hexes, and therefore the number of hexes in the grid
    RD.hextohex_d = conf.getFloat ("hextohex_d", 0.01f);

    // Boundary fall-off distance
    RD.boundaryFalloffDist = conf.getFloat ("boundaryFalloffDist", 0.01f);


    // After setting the first few features, we can set up all the vectors in RD:
    RD.allocate();

    // After allocate(), we can set up parameters:
    RD.set_dt(dt);

    // Set the Schakenberg model parameters
    RD.k1 = conf.getDouble ("k1", 1);
    RD.k2 = conf.getDouble ("k2", 1);
    RD.k3 = conf.getDouble ("k3", 1);
    RD.k4 = conf.getDouble ("k4", 1);
    RD.D_A = conf.getDouble ("D_A", 0.1);
    RD.D_B = conf.getDouble ("D_B", 0.1);

    // Now parameters are set, call init()
    RD.init();

    /*
     * Now create a log directory if necessary, and exit on any
     * failures.
     */
    if (Tools::dirExists (logpath) == false) {
        Tools::createDir (logpath);
        if (Tools::dirExists (logpath) == false) {
            cerr << "Failed to create the logpath directory "
                 << logpath << " which does not exist."<< endl;
            return 1;
        }
    } else {
        // Directory DOES exist. See if it contains a previous run and
        // exit without overwriting to avoid confusion.
        if (overwrite_logs == false
            && (Tools::fileExists (logpath + "/params.json") == true
                || Tools::fileExists (logpath + "/positions.h5") == true)) {
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
    conf.set ("float_width", (unsigned int)sizeof(FLOATTYPE));
    string tnow = Tools::timeNow();
    conf.set ("sim_ran_at_time", tnow.substr(0,tnow.size()-1));
    conf.set ("hextohex_d", RD.hextohex_d);
    conf.set ("D_A", RD.D_A);
    conf.set ("D_B", RD.D_B);
    conf.set ("k1", RD.k1);
    conf.set ("k2", RD.k2);
    conf.set ("k3", RD.k3);
    conf.set ("k4", RD.k4);
    conf.set ("dt", RD.get_dt());
    // Store the binary name and command argument into root, too.
    if (argc > 0) { conf.set("argv0", argv[0]); }
    if (argc > 1) { conf.set("argv1", argv[1]); }

    // We'll save a copy of the parameters for the simulation in the log directory as params.json
    const string paramsCopy = logpath + "/params.json";
    conf.write (paramsCopy);
    if (conf.ready == false) {
        cerr << "Warning: Something went wrong writing a copy of the params.json: " << conf.emsg << endl;
    }

#ifdef COMPILE_PLOTTING
    // Ask for a keypress before exiting so that the final images can be studied
    int a;
    cout << "Press any key[return] to exit.\n";
    cin >> a;
#endif

    return 0;
};
