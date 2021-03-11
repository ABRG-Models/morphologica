#include <iostream>
#include "../../Config.h"


using namespace morph;

int main(void){
    Config c("test.json");
    int id = c.getInt("id", 0);
    std::string name = c.getString("name", "");

    std::cout << "id: " << id << ", name: " << name << std::endl;
    return 0;
}