#include "piksi_multi_gps.hpp"
#include <iostream>
#include <chrono>
#include <thread>
#include <iomanip>
#include <sstream>

// Function to print GPS data, extracted from the callback
void print_gps_data(const piksi::PiksiMultiGPS& gps) {
    const piksi::PiksiData& data = gps.get_data();

    // Determine status string and color based on fix mode (lower 3 bits of status)
    std::string status_str;
    std::string color;
    int fix_mode = data.status & 0x07;
    switch (fix_mode) {
        case 1: status_str = "SPP"; color = "\033[31m"; break; // Red
        case 6: status_str = "SBAS"; color = "\033[35m"; break; // Purple
        case 2: status_str = "DGPS"; color = "\033[36m"; break; // Cyan
        case 3: status_str = "RTK Float"; color = "\033[34m"; break; // Blue
        case 4: status_str = "RTK Fixed"; color = "\033[32m"; break; // Green
        case 5: status_str = "DR"; color = "\033[37m"; break; // White
        default: status_str = "Unknown/Invalid"; color = "\033[31m"; break; // Red for invalid
    }

    // Calculate latency
    std::string latency_str;
    if (data.utc_timestamp >= 0.0) {
        double now_time = static_cast<double>(std::time(nullptr));
        double latency = now_time - data.utc_timestamp;
        std::ostringstream oss;
        oss << std::setprecision(3) << latency << " seconds";
        latency_str = oss.str();
    } else {
        latency_str = "N/A";
    }

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
    std::cout << color << "Status: " << status_str << (data.rtk_solution ? " (RTK Solution)" : "") << "\033[0m" << std::endl;
    std::cout << "Update Frequency: " << std::setprecision(2) << data.frequency << " Hz" << std::endl;
    std::cout << "Data Transmission Latency: " << latency_str << std::endl;
}

int main() {
    piksi::PiksiMultiGPS gps("../config.cfg");

    std::cout << "GPS: Initializing..." << std::endl;
    gps.open();
    gps.configure();
    gps.init_loop();

    while (true) {
        gps.loop();
        // Check if there is new data to print
        if (gps.has_new_data()) {
            print_gps_data(gps);
        }
    }

    gps.close();
    return 0;
}