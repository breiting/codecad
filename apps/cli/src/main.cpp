#include <ccad/version.hpp>
#include <iostream>

#include "App.hpp"
using namespace std;

int main(int argc, char** argv) {
    cout << "CodeCAD Kernel Version: " << CODECAD_VERSION_STRING << endl;
    App a;
    a.start(argc, argv);
    return 0;
}
