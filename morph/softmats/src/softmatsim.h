#pragma once

#include <iostream>
#include <vector>
#include "core/animat.h"
#include "core/ground.h"
#include "core/pbdim.h"
#include "visual/view.h"
#include "core/bodyset.h"
#include "core/animatsource.h"
#include "visual/video.h"

namespace morph{ namespace softmats{
/**
 * Interface for the simulator
 * 
 * This class provides the common interface to the simulation. It is 
 * intended to be included and used by external programs. Interaction with the API
 * is perform by defining some listeners for the main events in the simulation.
 * 
 * @author Alejandro Jimenez Rodriguez
 */
class SoftmatSim{
private:
    // Sources of animats
    std::vector<AnimatSource*> sources;
    // Set of animats in the scene
    BodySet *animats;
    // Ground
    Ground *theGround;
    // Position-based-dynamics solver
    PBD *solver;
    // Renderer
    View *view;
    // Video Renderer
    VideoRecorder* videoRecorder;
    // ------ user defined listeners --------
    void (*setup)(SoftmatSim *);
    void (*update)(SoftmatSim *);
    void(*draw)(SoftmatSim *);
    void (*finishFn)(const SoftmatSim *s);
    void (*contactFn)(const SoftmatSim *s, const std::vector<Animat *>& animats);

    /**
     * Generates new sources based on the corresponding period.
     * 
     * @param step - Current step of the simulation
     */
    void spawnSources(int step);
    // Frees memory etc.
    void cleanup();
    // Is the simulation still running?
    bool running;
public:
    SoftmatSim( void (*setup)(SoftmatSim *), void (*update)(SoftmatSim *), void(*draw)(SoftmatSim *) );

    // Creation members
    /**
     * Creates an animat in the scene
     * 
     * @param x - x coordinate
     * @param y - y coordinate
     * @param z - z coordinate
     */
    Animat *animat(float x, float y, float z, double mass);
    /**
     * Creates a new animat source
     * 
     * @param n - Number of animats to generate
     * @param period - Period at which new animats should be spawn
     * @param x - x coordinate
     * @param y - y coordinate
     * @param z - z coordinate
     */
    AnimatSource* animatSource( int n, int period, float x, float y, float z);
    /**
     * Creates a ground a given height
     * 
     * @param height
     */
    Ground *ground( float height );

    // Enviroment control
    // Turns on/off lights
    void light( bool v );
    // Sets the gravity value
    void gravity( float v );
    // Controls the camera - TO IMPROVE
    void camera( float az, float ev );
    // Records video
    void video();

    // Listener
    // Sets the onfinish listener
    void onFinish( void (*f)(const SoftmatSim *s) );
    // Sets the onContact Listener
    void onContact( void (*f)(const SoftmatSim *s, const std::vector<Animat *>& animats) );
    // ----------- For future implementation -------------
    // void onAnimatContact( void (*f)(const Animat* a, std::vector<Point*> points) );
    // Investigation
    // vector<ContactRegion *> contacts();
    // bool hasContact( const Animat* a, const Animat* b );
    // constexpr ContactRegion *contactRegion( const Animat* a, const Animat *b );

    // Initializes the simulation
    void initialize();
    // Draws all the entities
    void drawAll();
    // Runs the simulation
    void run();

    ~SoftmatSim();
};

}}