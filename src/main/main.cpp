#include "main/module.hpp"
#include "main/spine.hpp"

int main(int argc, char **argv) {

    Spine* spine = new Spine(1);

    spine->loadModules("./modules");

    spine->sendMessage("BaseModule", "Initialise");

    spine->close();
    delete spine;

    return 0;
}