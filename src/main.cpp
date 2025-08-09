#include "piksi_multi_gps.hpp"
#include <iostream>
#include <iomanip>
#include <thread>
#include <chrono>

int main() {
    piksi::PiksiMultiGPS gps("/dev/cu.usbserial-AL00KUE3", 115200);

    std::cout << "GPS: Initializing..." << std::endl;
    gps.open();
    gps.init_loop();

    while (true) {
        gps.loop();
        const auto& data = gps.get_data();

        std::cout << "\n=== GPS Data ===" << std::endl;
        std::cout << "UTC Time: " << std::setprecision(2) << data.utc << " (" 
                  << static_cast<int>(data.hr) << ":" 
                  << static_cast<int>(data.min) << ":" 
                  << static_cast<int>(data.sec) << "." << data.ms << ")" << std::endl;
        std::cout << "LLH: Lat=" << std::setprecision(9) << data.lat << " deg, Lon=" << data.lon 
                  << " deg, Height=" << data.h << " m" << std::endl;
        std::cout << "LLH Accuracy: Horizontal=" << data.S_llh_h << " m, Vertical=" << data.S_llh_v << " m" << std::endl;
        std::cout << "ECEF: X=" << data.ecef_x << " m, Y=" << data.ecef_y << " m, Z=" << data.ecef_z << " m" << std::endl;
        std::cout << "ECEF Accuracy: " << data.S_ecef << " m" << std::endl;
        std::cout << "Baseline NED: N=" << data.n << " m, E=" << data.e << " m, D=" << data.d << " m" << std::endl;
        std::cout << "RTK Accuracy: Horizontal=" << data.S_rtk_x_h << " m, Vertical=" << data.S_rtk_x_v << " m" << std::endl;
        std::cout << "Velocity NED: N=" << data.v_n << " m/s, E=" << data.v_e << " m/s, D=" << data.v_d << " m/s" << std::endl;
        std::cout << "Velocity Accuracy: Horizontal=" << data.S_rtk_v_h << " m/s, Vertical=" << data.S_rtk_v_v << " m/s" << std::endl;
        std::cout << "Satellites: " << data.sats << std::endl;
        std::cout << "Status: " << data.status << (data.rtk_solution ? " (RTK Solution)" : " (No RTK)") << std::endl;

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    gps.close();
    return 0;
}