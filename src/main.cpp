#include "piksi_multi_gps.hpp"
#include <iostream>

int main() {
    piksi::PiksiMultiGPS gps("/dev/cu.usbserial-AL00KUE3", 115200);

    std::cout << "GPS: Initializing..." << std::endl;
    gps.open();
    gps.configure();
    gps.init_loop();

    while (true) {
        gps.loop();
    }

    gps.close();
    return 0;
}