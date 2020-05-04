#include "Mnist.h"
int main()
{
    try {
        Mnist m;
        std::cout << "training size:" << m.num_training() << "\n";
        m.showall();
    } catch (const std::exception& e) {
        std::cout << e.what() << std::endl;
    }
    return 0;
}
