#pragma once
#include <vector>
#include "bodyset.h"

namespace morph{ namespace softmats{

class PBD{
private:
    double h;
public:
    PBD();
    void generateConstraints( BodySet *bs, int step );
    void velocityUpdate( BodySet *bs );
    void projectConstraints( BodySet *bs );    
    void loop( BodySet *bs, int step );

};

}}