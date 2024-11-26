/*
 * A morphologica example: The Schnakenberg Turing-like RD system.
 *
 * Author: Seb James
 */

/*!
 * This will be passed as the template argument for RD_classes and
 * should be defined when compiling.
 */
#ifndef FLT
// Check CMakeLists.txt to change to double or float
# error "Please define FLT when compiling (hint: See CMakeLists.txt)"
#endif

/*!
 * Include the reaction diffusion class
 */
#include "rd_schnakenberg.h"

/*!
 * General STL includes used here
 */
#include <iostream>
#include <fstream>
#include <vector>
#include <list>
#include <string>
#include <sstream>
#include <limits>
#include <chrono>

#ifdef COMPILE_PLOTTING
/*!
 * If COMPILE_PLOTTING is defined at compile time, then include the display and
 * plotting code. I usually put all the plotting code inside #defines like this so
 * that I can compile a version of the binary without plotting, for parameter searches
 * in which I am only going to be saving out HDF5 data.
 */
# include <morph/Visual.h>
# include <morph/HexGridVisual.h>
# include <morph/ColourMap.h>
# include <morph/VisualDataModel.h>
# include <morph/scale.h>
# include <morph/vec.h>

//! Helper function to save PNG images with a suitable name
void savePngs (const std::string& logpath, const std::string& name,
               unsigned int frameN, morph::Visual<>& v)
{
    std::stringstream ff1;
    ff1 << logpath << "/" << name<< "_";
    ff1 << std::setw(5) << std::setfill('0') << frameN;
    ff1 << ".png";
    v.saveImage (ff1.str());
}
#endif

/*!
 * Included for directory manipulation code
 */
#include <morph/tools.h>

/*!
 * A jsoncpp-wrapping class for configuration.
 */
#include <morph/Config.h>

/*
 * using directives just before main()
 */
#ifdef COMPILE_PLOTTING
using morph::Visual;
using morph::HexGridVisual;
using morph::ColourMap;
using morph::ColourMapType;
using morph::VisualDataModel;
using morph::scale;
using morph::vec;
#endif
using morph::Config;
using std::string;
using std::stringstream;
using std::cerr;
using std::cout;
using std::endl;
using std::vector;
using std::chrono::milliseconds;
using std::chrono::duration_cast;
using std::chrono::steady_clock;

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
    // After how many simulation steps should a log of the simulation data be written?
    const unsigned int logevery = conf.getUInt ("logevery", 100UL);
    // If true, write over an existing set of logs
    bool overwrite_logs = conf.getBool ("overwrite_logs", false);

    // Handling of log path requires a few lines of code:
    string logpath = conf.getString ("logpath", "fromfilename");
    string logbase = "";
    if (logpath == "fromfilename") {
        // Using json filename as logpath
        string justfile = paramsfile;
        // Remove trailing .json and leading directories
        vector<string> pth = morph::tools::stringToVector (justfile, "/");
        justfile = pth.back();
        morph::tools::searchReplace (".json", "", justfile);
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

    // The length of one timestep
    const FLT dt = static_cast<FLT>(conf.getDouble ("dt", 0.00001));

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

    // Window width and height
    const unsigned int win_width = conf.getUInt ("win_width", 1025UL);
    unsigned int win_height_default = static_cast<unsigned int>(0.8824f * (float)win_width);
    const unsigned int win_height = conf.getUInt ("win_height", win_height_default);

    // Set up the morph::Visual object which provides the visualization scene (and
    // a GLFW window to show it in)
    Visual v1 (win_width, win_height, "Schnakenberg RD");
    // Set a dark blue background (black is the default). This value has the order
    // 'RGBA', though the A(alpha) makes no difference.
    v1.bgcolour = {conf.getFloat("bgR", 0.2f), conf.getFloat("bgG", 0.2f), conf.getFloat("bgB", 0.2f), 1.0f};
    // You can tweak the near and far clipping planes
    v1.zNear = 0.001;
    v1.zFar = 10000;
    // And the field of view of the visual scene.
    v1.fov = 45;
    // You can lock movement of the scene
    v1.sceneLocked = conf.getBool ("sceneLocked", false);
    // You can set the default scene x/y/z offsets
    v1.setSceneTrans (conf.getFloat ("x_default", 0.0f),
                      conf.getFloat ("y_default", 0.0f),
                      conf.getFloat ("z_default", -5.0f));
    // Make this larger to "scroll in and out of the image" faster
    v1.scenetrans_stepsize = 0.5;

    // if using plotting, then set up the render clock
    steady_clock::time_point lastrender = steady_clock::now();
#endif

    /*
     * Instantiate and set up the model object
     */
    RD_Schnakenberg<FLT> RD;

    RD.svgpath = conf.getString ("svgpath", ""); // We'll do an elliptical boundary if svgpath is empty
    RD.ellipse_a = conf.getDouble ("ellipse_a", 0.8);
    RD.ellipse_b = conf.getDouble ("ellipse_b", 0.6);
    RD.logpath = logpath;

    // Control the size of the hexes, and therefore the number of hexes in the grid
    RD.hextohex_d = conf.getFloat ("hextohex_d", 0.01f);
    RD.hexspan = conf.getFloat ("hexspan", 4.0f);

    // Boundary fall-off distance, this is used to ensure that initial noise in the
    // system rolls off to zero at the boundary.
    RD.boundaryFalloffDist = conf.getFloat ("boundaryFalloffDist", 0.01f);

    // After setting the first few features, we can call the allocate function to set
    // up all the vectors in the RD object:
    RD.allocate();

    // After allocate(), we can set up parameters:
    RD.set_dt(dt);

    // Set the Schakenberg model parameters:
    RD.k1 = conf.getDouble ("k1", 1);
    RD.k2 = conf.getDouble ("k2", 1);
    RD.k3 = conf.getDouble ("k3", 1);
    RD.k4 = conf.getDouble ("k4", 1);
    RD.D_A = conf.getDouble ("D_A", 0.1);
    RD.D_B = conf.getDouble ("D_B", 0.1);

    // Now parameters are set, call init(), which in this example simply initializes A
    // and B with noise.
    RD.init();

    /*
     * Now create a log directory if necessary, and exit on any
     * failures.
     */
    if (morph::tools::dirExists (logpath) == false) {
        morph::tools::createDir (logpath);
        if (morph::tools::dirExists (logpath) == false) {
            cerr << "Failed to create the logpath directory "
                 << logpath << " which does not exist."<< endl;
            return 1;
        }
    } else {
        // Directory DOES exist. See if it contains a previous run and
        // exit without overwriting to avoid confusion.
        if (overwrite_logs == false
            && (morph::tools::fileExists (logpath + "/params.json") == true
                || morph::tools::fileExists (logpath + "/positions.h5") == true)) {
            cerr << "Seems like a previous simulation was logged in " << logpath << ".\n"
                 << "Please clean it out manually, choose another directory or set\n"
                 << "overwrite_logs to true in your parameters config JSON file." << endl;
            return 1;
        }
    }

    // As RD.allocate() as been called (and log directory has been created/verified
    // ready), positions can be saved to an HDF5 file:
    RD.savePositions();

#ifdef COMPILE_PLOTTING
    // Before starting the simulation, create the HexGridVisuals.

    // Spatial offset, for positioning of HexGridVisuals
    vec<float> spatOff;
    float xzero = 0.0f;

    // A. Offset in x direction to the left.
    xzero -= 0.5*RD.hg->width();
    spatOff = { xzero, 0.0, 0.0 };
    morph::ColourMapType cmt = morph::ColourMap<FLT>::strToColourMapType (conf.getString ("colourmap", "Jet"));

    // Create a new HexGridVisual then set its parameters (zScale, colourScale, etc.
    auto hgv1 = std::make_unique<morph::HexGridVisual<FLT>> (RD.hg.get(), spatOff);
    v1.bindmodel (hgv1);
    hgv1->setScalarData (&RD.A);
    // Z position scaling - how hilly/bumpy the visual will be.
    hgv1->zScale.setParams (0.2f, 0.0f);
    // The second is the colour scaling. Set this to autoscale.
    hgv1->colourScale.do_autoscale = true;
    hgv1->cm.setType (cmt);
    //hgv1->hexVisMode = morph::HexVisMode::Triangles;
    hgv1->addLabel ("Variable A", { -0.2f, RD.ellipse_b*-1.4f, 0.01f }, morph::TextFeatures(0.1f, 48, morph::colour::white));
    // "finalize" is required before adding the HexGridVisual to the morph::Visual.
    hgv1->finalize();
    auto hgv1p = v1.addVisualModel (hgv1);


    // B. Offset in x direction to the right.
    xzero += RD.hg->width();
    spatOff = { xzero, 0.0, 0.0 };
    auto hgv2 = std::make_unique<morph::HexGridVisual<FLT>> (RD.hg.get(), spatOff);
    v1.bindmodel (hgv2);
    hgv2->setScalarData (&RD.B);
    hgv2->zScale.setParams (0.2f, 0.0f);
    hgv2->colourScale.do_autoscale = true;
    hgv2->cm.setType (cmt);
    hgv2->addLabel ("Variable B", { -0.2f, RD.ellipse_b*-1.4f, 0.01f }, morph::TextFeatures(0.1f, 48, morph::colour::white));
    hgv2->finalize();
    auto hgv2p = v1.addVisualModel (hgv2);
#endif

    // Start the loop
    bool finished = false;
    while (finished == false) {
        // Step the model
        RD.step();

#ifdef COMPILE_PLOTTING
        if ((RD.stepCount % plotevery) == 0) {
            // These two lines update the data for the two hex grids. That leads to
            // the CPU recomputing the OpenGL vertices for the visualizations.
            hgv1p->updateData (&(RD.A));
            hgv1p->clearAutoscaleColour();

            hgv2p->updateData (&(RD.B));
            hgv2p->clearAutoscaleColour();

            if (saveplots) {
                if (vidframes) {
                    savePngs (logpath, "schnak", framecount, v1);
                    ++framecount;
                } else {
                    savePngs (logpath, "schnak", RD.stepCount, v1);
                }
            }
        }

        // rendering the graphics. After each simulation step, check if enough time
        // has elapsed for it to be necessary to call v1.render().
        steady_clock::duration sincerender = steady_clock::now() - lastrender;
        if (duration_cast<milliseconds>(sincerender).count() > 17) { // 17 is about 60 Hz
            v1.poll();
            v1.render();
            lastrender = steady_clock::now();
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
    // in there, such as the FLT. If float_width is 4, then
    // results were computed with single precision, if 8, then double
    // precision was used. Also save various parameters from the RD system.
    conf.set ("float_width", (unsigned int)sizeof(FLT));
    string tnow = morph::tools::timeNow();
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
    cout << "Ctrl-c or press x in graphics window to exit.\n";
    v1.keepOpen();
#endif

    return 0;
};
