#pragma once

#include <iostream>
#include <vector>
#include "core/animat.h"
#include "core/ground.h"
#include "core/pbdim.h"
#include "visual/view.h"
#include "core/bodyset.h"

namespace morph{ namespace softmats{

class SoftmatSim{
private:
    BodySet *animats;
    Ground *theGround;
    PBD *solver;
    View *view;
    void (*setup)(SoftmatSim *);
    void (*update)(SoftmatSim *);
    void(*draw)(SoftmatSim *);
    void (*finishFn)(const SoftmatSim *s);
    void (*contactFn)(const SoftmatSim *s, const std::vector<Animat *>& animats);
    bool running;
public:
    SoftmatSim( void (*setup)(SoftmatSim *), void (*update)(SoftmatSim *), void(*draw)(SoftmatSim *) );

    // Creation members
    Animat *animat(float x, float y, float z, double mass); // change this
    Ground *ground( float height );
    // AnimatSource *source( Point *p );

    // Enviroment control
    void light( bool v );
    void gravity( float v );
    void camera( float az, float ev );

    // Listener
    void onFinish( void (*f)(const SoftmatSim *s) );
    void onContact( void (*f)(const SoftmatSim *s, const std::vector<Animat *>& animats) );
    // void onAnimatContact( void (*f)(const Animat* a, std::vector<Point*> points) );

    // Investigation
    // vector<ContactRegion *> contacts();
    // bool hasContact( const Animat* a, const Animat* b );
    // constexpr ContactRegion *contactRegion( const Animat* a, const Animat *b );

    void initialize();
    // Drawing
    void drawAll();
    // Run
    void run();

    void cleanup();
    ~SoftmatSim();
};

}}