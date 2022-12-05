/*
 * Implements 'Subbarrel Patterns in Somatosensory Cortical Barrels Can Emerge from
 * Local Dynamic Instabilities', Ermentrout, Simons and Land, PLOS Comp Biol 2009
 */

#ifndef FLT
# error "Please define FLT as float or double when compiling (hint: See CMakeLists.txt)"
#endif

#include <iostream>
#include <vector>
#include <string>
#include <chrono>
#include <utility>

// Morphologica headers: visualization and JSON config
#include <morph/tools.h>
#include <morph/Config.h>
#include <morph/Scale.h>
#include <morph/ColourMap.h>
#include <morph/Visual.h>
#include <morph/VisualDataModel.h>
// Alias VisualDataModel<FLT>* as VdmPtr, to neaten code
typedef morph::VisualDataModel<FLT>* VdmPtr;
#include <morph/HexGridVisual.h>
#include <morph/GraphVisual.h>
#include <morph/vec.h>
#include <morph/MathAlgo.h>

// Simulation code header
#include "rd_erm.h"

int main (int argc, char **argv)
{
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " /path/to/params.json [/path/to/logdir]" << std::endl;
        return 1;
    }
    std::string paramsfile (argv[1]);

    // Get parameters from config file
    morph::Config conf(paramsfile);
    if (!conf.ready) {
        std::cerr << "Failed to read config " << paramsfile << ". Exiting.\n";
        return 1;
    }

    const unsigned int steps = conf.getUInt ("steps", 1000);
    if (steps == 0) {
        std::cerr << "Finished simulating 0 steps. Exiting.\n";
        return 1;
    }
    const unsigned int logevery = conf.getUInt ("logevery", 100);
    if (logevery == 0) {
        std::cerr << "Can't log every 0 steps. Exiting.\n";
        return 1;
    }
    bool overwrite_logs = conf.getBool ("overwrite_logs", false);
    std::string logpath = conf.getString ("logpath", "logs/erm2");
    if (argc == 3) {
        std::string argpath(argv[2]);
        std::cerr << "Overriding the config-given logpath " << logpath << " with " << argpath << std::endl;
        logpath = argpath;
        if (overwrite_logs == true) {
            std::cerr << "WARNING: You set a command line log path.\n"
                      << "       : Note that the parameters config permits the program to OVERWRITE LOG\n"
                      << "       : FILES on each run (\"overwrite_logs\" is set to true).\n";
        }
    }

    std::cout << "steps to simulate: " << steps << std::endl;

    const unsigned int plotevery = conf.getUInt ("plotevery", 10);
    const unsigned int win_width = conf.getUInt ("win_width", 340);
    unsigned int win_height = static_cast<unsigned int>(0.8824f * (float)win_width);

    // Instantiate the model object
    RD_Erm<FLT> RD;
    RD.svgpath = conf.getString ("svgpath", ""); // We'll do an elliptical boundary if svgpath is empty
    RD.ellipse_a = conf.getDouble ("ellipse_a", 0.8);
    RD.ellipse_b = conf.getDouble ("ellipse_b", 0.6);
    RD.logpath = logpath;
    RD.set_dt(static_cast<FLT>(conf.getDouble ("dt", 0.00001)));
    RD.hextohex_d = conf.getFloat ("hextohex_d", 0.01f);
    RD.boundaryFalloffDist = conf.getFloat ("boundaryFalloffDist", 0.01f);
    RD.N = 1;
    RD.Dn = conf.getDouble ("Dn", 0.3);
    RD.Dc = conf.getDouble ("Dc", 0.3*RD.Dn); // 0.3 * Dn
    RD.beta = conf.getDouble ("beta", 5.0);
    RD.a = conf.getDouble ("a", 1.0);
    RD.b = conf.getDouble ("b", 1.0);
    RD.mu = conf.getDouble ("mu", 1.0);
    RD.chi = conf.getDouble ("chi", -1.0);
    if (RD.chi < 0.0) {
        // Set chi to Dn, as in the paper (see linear analysis)
        RD.chi = RD.Dn; // or: conf.getDouble ("chi", 0.3);
    }

    // Allocate and initialise the model
    RD.hexspan = 3.0f  * (RD.ellipse_a > RD.ellipse_b ? RD.ellipse_a : RD.ellipse_b);
    std::cout << "RD.hexspan: " << RD.hexspan << ", Dn=" << RD.Dn << ", Dc=" << RD.Dc << ", chi=" << RD.chi << std::endl;
    RD.allocate();
    RD.init();

    // Autoscale the colour map on each step?
    bool do_autoscale = conf.getBool ("autoscale", false);

    // Now create a log directory if necessary, and exit on any failures.
    if (morph::Tools::dirExists (logpath) == false) {
        morph::Tools::createDir (logpath);
        if (morph::Tools::dirExists (logpath) == false) {
            std::cerr << "Failed to create the logpath directory "
                      << logpath << " which does not exist.\n";
            return 1;
        }
    } else {
        // Directory DOES exist. See if it contains a previous run and
        // exit without overwriting to avoid confusion.
        if (overwrite_logs == false
            && (morph::Tools::fileExists (logpath + "/params.json") == true
                || morph::Tools::fileExists (logpath + "/positions.h5") == true)) {
            std::cerr << "Seems like a previous simulation was logged in " << logpath << ".\n"
                      << "Please clean it out manually, choose another directory or set\n"
                      << "overwrite_logs to true in your parameters config JSON file.\n";
            return 1;
        }
    }

    // As RD.allocate() as been called (and log directory has been
    // created/verified ready), positions can be saved to file.
    RD.savePositions();

    // Set up the morph::Visual object
    morph::Visual v (win_width, win_height, "Ermentrout (Keller-Segel)");
    v.zNear = 0.001;
    v.zFar = 500;
    v.fov = 45;
    v.showCoordArrows = true;
    v.showTitle = false;
    // You can lock movement of the scene
    v.sceneLocked = conf.getBool ("sceneLocked", false);
    // You can set the default scene x/y/z offsets
    v.setZDefault (conf.getFloat ("z_default", -10.0f));
    v.setSceneTransXY (conf.getFloat ("x_default", 0.0f), conf.getFloat ("y_default", 0.0f));
    // Make this larger to "scroll in and out of the image" faster
    v.scenetrans_stepsize = 0.5;

    // Add two morph::HexGridVisuals to the morph::Visual.

    // A 2D scaling to apply to the visuals
    float myscale = conf.getFloat ("size_scale", 1.0f);

    // A z position to place the hexgrid visuals
    float _Z = 0.0f;

    morph::vec<float, 3> spatOff = {0,0,_Z}; // spatial offset
    // Data scaling parameters
    float _m = 0.2;
    float _c = 0.0;
    morph::Scale<FLT, float> cscale;
    cscale.setParams (_m, _c);

    // Set up a 3D map of the surface RD.n[0] using a morph::HexGridVisual
    spatOff[0] -= 0.6 * (RD.hg->width());
    morph::HexGridVisual<FLT>* hgv1 = new morph::HexGridVisual<FLT> (v.shaderprog, v.tshaderprog, RD.hg, spatOff);
    hgv1->setSizeScale (myscale, myscale);
    hgv1->setScalarData (&RD.n[0]);
    // You can directly set VisualDataModel::zScale and ::colourScale:
    hgv1->zScale.setParams (_m/10.0f, _c/10.0f);
    // ...or use setters to copy one in:
    hgv1->setCScale (cscale);
    hgv1->cm.setType (morph::ColourMapType::Jet);
    hgv1->hexVisMode = morph::HexVisMode::Triangles;
    hgv1->addLabel ("n (axon density)", {-0.6f, RD.hg->width()/2.0f, 0},
                    morph::colour::white, morph::VisualFont::Vera, 0.12f, 64);
    hgv1->finalize();
    v.addVisualModel (hgv1);

    // Set up a 3D map of the surface RD.c[0]
    spatOff[0] *= -1;
    morph::HexGridVisual<FLT>* hgv2 = new morph::HexGridVisual<FLT> (v.shaderprog, v.tshaderprog, RD.hg, spatOff);
    hgv2->setSizeScale (myscale, myscale);
    hgv2->setScalarData (&RD.c[0]);
    hgv2->zScale.setParams (_m/10.0f, _c/10.0f);
    hgv2->setCScale (cscale);
    hgv2->cm.setType (morph::ColourMapType::Jet);
    hgv2->hexVisMode = morph::HexVisMode::HexInterp;
    hgv2->addLabel ("c (chemoattractant)", {-0.7f, RD.hg->width()/2.0f, 0},
                    morph::colour::white, morph::VisualFont::Vera, 0.12f, 64);
    hgv2->finalize();
    v.addVisualModel (hgv2);

    // Set up a 2D graph with morph::GraphVisual
    spatOff = {0.5f, -2.0f, 0.0f};
    morph::GraphVisual<FLT>* graph1 = new morph::GraphVisual<FLT> (v.shaderprog, v.tshaderprog, spatOff);
    graph1->setdarkbg(); // colours axes and text
    graph1->twodimensional = true;
    graph1->setlimits (0, steps*RD.get_dt(), 0, conf.getFloat("graph_ymax", 40000.0f));
    graph1->policy = morph::stylepolicy::lines;
    graph1->ylabel = "Sum";
    graph1->xlabel = "Sim time (s)";
    graph1->prepdata ("n");
    graph1->prepdata ("c");
    graph1->finalize();
    v.addVisualModel (static_cast<morph::VisualModel*>(graph1));

    // Set up the render clock
    std::chrono::steady_clock::time_point lastrender = std::chrono::steady_clock::now();

    // Start the loop
    bool doing = true;
    static constexpr bool debug_ranges = false;
    std::pair<FLT, FLT> mm; // maxmin
    while (doing) {
        // Step the model:
        RD.step();

        if ((RD.stepCount % plotevery) == 0) {
            // Plot n and c
            if (do_autoscale == true) {
                mm = morph::MathAlgo::maxmin (RD.n[0]);
                hgv1->colourScale.compute_autoscale (mm.second, mm.first);
                mm = morph::MathAlgo::maxmin (RD.c[0]);
                hgv2->colourScale.compute_autoscale (mm.second, mm.first);
            }

            if constexpr (debug_ranges) {
                mm = morph::MathAlgo::maxmin (RD.n[0]);
                std::cout << "n range: " << std::abs(mm.second - mm.first) << std::endl;
                mm = morph::MathAlgo::maxmin (RD.c[0]);
                std::cout << "c range: " << std::abs(mm.second - mm.first) << std::endl;
            }

            hgv1->updateData (&RD.n[0]);
            hgv2->updateData (&RD.c[0]);
            // Append to the 2D graph of sums:
            graph1->append ((float)RD.stepCount*RD.get_dt(), RD.sum_n[0], 0);
            graph1->append ((float)RD.stepCount*RD.get_dt(), RD.sum_c[0], 1);
        }

         // Save data every 'logevery' steps
        if ((RD.stepCount % logevery) == 0) {
            std::cout << "Logging data at step " << RD.stepCount << std::endl;
            RD.saveState();
        }

        // After a while, stop:
        if (RD.stepCount > steps) { doing = false; }

        // rendering the graphics.
        std::chrono::steady_clock::duration sincerender = std::chrono::steady_clock::now() - lastrender;
        if (std::chrono::duration_cast<std::chrono::milliseconds>(sincerender).count() > 17) { // 17 is about 60 Hz
            glfwPollEvents();
            v.render();
            lastrender = std::chrono::steady_clock::now();
        }
    }

    // Before exit, save data
    RD.saveState();

    // Add simulation runtime information to the config, before saving it out as params.json
    conf.set ("float_width", (unsigned int)sizeof(FLT));
    std::string tnow = morph::Tools::timeNow();
    conf.set ("sim_ran_at_time", tnow.substr(0,tnow.size()-1));
    conf.set ("final_step", RD.stepCount);
    conf.set ("hextohex_d", RD.hextohex_d);
    conf.set ("dt", RD.get_dt());
#ifdef __GLN__ // Currently Config::insertGitInfo fails on Macs and Windows
    conf.insertGitInfo ("sim/");
#endif
    // Store the binary name and command argument into root, too.
    if (argc > 0) { conf.set("argv0", argv[0]); }
    if (argc > 1) { conf.set("argv1", argv[1]); }
    const std::string paramsCopy = logpath + "/params.json";
    conf.write (paramsCopy);
    if (conf.ready == false) {
        std::cerr << "Warning: Something went wrong writing a copy of the params.json: "
                  << conf.emsg << std::endl;
    }

    std::cout << "Press x in graphics window to exit.\n";
    v.keepOpen();

    return 0;
};
