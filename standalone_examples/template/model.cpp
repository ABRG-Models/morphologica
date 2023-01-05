#include <morph/HdfData.h>
#include <morph/Config.h>
#include <morph/Visual.h>
#include <morph/GraphVisual.h>
#include <morph/VisualDataModel.h>
#include <morph/vec.h>

// basic model class
template<class Flt>
class Model {

public:
    int time;
    morph::Config* conf;
    Flt x, r;

    Model(morph::Config* config){
        conf = config;
        r = conf->getFloat("r",0.7);    // bifurcation parameter
        x = conf->getFloat("x",0.5);    // initial condition
        time = 0;
    }

    // step through the model dynamics (logistic map)
    void step(){
        // logistic map equation
        x = 4.0*r*x*(1.0-x);
        time++;
    }
};


int main(int argc, char **argv){

        if (argc < 4) { std::cerr << "\nUsage: ./model configfile logdir seed"; return -1; }

        // set random seed (NB: redundant in this sim - no rng used)
        std::srand(std::stoi(argv[3]));

        // access the config file
        std::string paramsfile (argv[1]);
        morph::Config conf(paramsfile);
        if (!conf.ready) { std::cerr << "Error setting up JSON config: " << conf.emsg << std::endl; return 1; }

        // Load in some data from config file
        unsigned int T = conf.getUInt("T",1000);

        // setup the log file
        std::string logpath = argv[2];
        std::ofstream logfile;
        morph::Tools::createDir (logpath);
        { std::stringstream ss; ss << logpath << "/log.txt"; logfile.open(ss.str());}
        logfile<<"Hello world!"<<std::endl;

        // Start the clock
        std::chrono::steady_clock::time_point lastrender = std::chrono::steady_clock::now();

        // Setup visuals
        const unsigned int win_height = conf.getUInt ("win_height", 600);
        const unsigned int win_width = conf.getUInt ("win_width", win_height);
        float graph_size = conf.getFloat ("graph_size", 3.5);
        float graph_offset = graph_size*0.5;

        morph::Visual v (win_width, win_height, "model");
        v.backgroundWhite();
        v.sceneLocked = conf.getBool ("sceneLocked", false);
        v.scenetrans_stepsize = 0.1;
        v.fov = 50;

        // Create storage vectors
        std::vector<float> X(1,0);
        std::vector<float> Y(1,0);

        // add a graph. make_unique<>() will return a std::unique_ptr<GraphVisual<float>>
        auto gv = std::make_unique<morph::GraphVisual<float>> (v.shaders, morph::vec<float>{-graph_offset,-graph_offset,0.0f});
        morph::DatasetStyle ds;
        ds.linewidth = 0.01;
        ds.linecolour = {0.0, 0.0, 0.0};
        ds.markersize = 0.0;
        gv->xlabel="time";
        gv->ylabel="X";
        gv->setsize(graph_size,graph_size);
        gv->setlimits (0,T,0,1);
        gv->setdata (X, Y, ds);
        gv->finalize();
        // Pass the graph (and ownership of its memory to the morph::Visual. The returned pointer can be used to interact with the GraphVisual
        morph::GraphVisual<float>* gv_ptr = v.addVisualModel (gv);

        // Create a basic model object
        Model<float> M(&conf);

        // Run the main loop
        for(int t=0;t<T;t++){

            // Step the model
            M.step();

            // Store the data for saving out
            X.push_back((float)M.time);
            Y.push_back(M.x);

            // update the display
            gv_ptr->update (X, Y, 0);

            // poll gl events and re-render
            std::chrono::steady_clock::duration sincerender = std::chrono::steady_clock::now() - lastrender;
            if (std::chrono::duration_cast<std::chrono::milliseconds>(sincerender).count() > 17) {
                glfwPollEvents();
                v.render();
                lastrender = std::chrono::steady_clock::now();
            }

        }

        v.keepOpen();

        // Save out the data
        std::stringstream fname;
        fname << logpath << "/out.h5";
        morph::HdfData data(fname.str());
        std::stringstream path;
        path.str(""); path.clear(); path << "/X";
        data.add_contained_vals (path.str().c_str(), X);
        path.str(""); path.clear(); path << "/Y";
        data.add_contained_vals (path.str().c_str(), Y);

        return 0.;
}
