#pragma once
#include "body.h"
#include "bodyset.h"

namespace morph{ namespace softmats{
class Body;
class BodySet;

class Constraint{

public:
    virtual void init( Body *b ) = 0;
    virtual void init( BodySet* bs ) = 0;
    virtual void generate( int step = 0 ) = 0;
    virtual void solve() = 0;
    virtual void updateVelocity() = 0;
};

}}