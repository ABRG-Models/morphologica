#include <morph/bnn/population.h>
#include <morph/bnn/projection.h>

int main()
{
    morph::bnn::population p1;
    morph::bnn::population p2;

    morph::bnn::projection p1_p2(p1, p2);

    for (;;) {
        p1_p2.step();
    }
}
