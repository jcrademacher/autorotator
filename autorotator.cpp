#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/program_options.hpp>
#include <iostream>
#include <cmath>
#include <uhd/exception.hpp>
#include <uhd/types/tune_request.hpp>
#include <uhd/usrp/multi_usrp.hpp>
#include <uhd/utils/safe_main.hpp>
#include <uhd/utils/static.hpp>
#include <uhd/utils/thread.hpp>

#include "stm23ip.hpp"

#define MOTOR_IP "192.168.10.2"
#define PORT    43333 

#define PULLEY_RATIO 0.5

#define ANGLE_FLAG "?"

namespace po = boost::program_options;

static int _main(int argc, char *argv[]);

int main(int argc, char *argv[]) {                                                      
    try {                                                     
        return _main(argc, argv);                             
    } catch (const std::exception& e) {                       
        std::cerr << "Error: " << e.what() << std::endl;      
    } catch (...) {                                           
        std::cerr << "Error: unknown exception" << std::endl; 
    }                                                        
    return ~0;                                                
}

int _main(int argc, char *argv[]) { 
     // setup the program options
    po::options_description desc("Allowed options");
    std::string exec;

    std::string exec_help = std::string("executable to run at every rotation. To label the absolute rotation, place a '")+ std::string(ANGLE_FLAG)
        + std::string("' in the string and this program will replace it with the absolute rotation");

    int start_angle, end_angle;
    double angle_step;
    // clang-format off
    desc.add_options()
        ("help", "help message")
        ("exec", po::value<std::string>(&exec)->default_value("?"), exec_help.c_str())
        ("start-angle", po::value<int>(&start_angle)->default_value(-90), "angle in degrees to begin the rotations at")
        ("end-angle", po::value<int>(&end_angle)->default_value(90), "angle in degrees to end the rotations at")
        ("angle-step", po::value<double>(&angle_step)->default_value(1.8), "angle in degrees to step sweeping from start-angle to end-angle")
    ;
    // clang-format on
    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    // print the help message
    if (vm.count("help")) {
        std::cout << boost::format("Autorotator %s") % desc << std::endl;
        std::cout << std::endl
                  << "This application provides control over the mechanical autorotator using a stepper motor controlled over Ethernet"
                  << std::endl;
        return ~0;
    }
    std::string args;
    uhd::usrp::multi_usrp::sptr usrp = uhd::usrp::multi_usrp::make(args);

    std::cout << "Connecting to motor..." << std::endl;
    //STM23IP* motor = new STM23IP(MOTOR_IP);

    double angle;
    size_t angle_string_location = exec.find(ANGLE_FLAG);
    std::string exec_to_call;
    std::ostringstream angle_str;

    int ipart;
    int fpart;

    for(angle = start_angle; angle <= end_angle; angle += angle_step) {
        ipart = ((int)round(angle*10.0))/10;
        fpart = abs((int)(round((angle - ipart)*10.0)));

        angle_str << boost::format("%d,%1d") % ipart % fpart;
        exec_to_call = exec;
        exec_to_call.replace(angle_string_location, std::string::npos, angle_str.str());

        //std::cout << boost::format("%.1f") % angle << boost::format(" | %s") % angle_str.str() << std::endl;
        system(exec_to_call.c_str());

        angle_str.str("");
        angle_str.clear();
    }
        

    //std::cout << boost::format("Bytes sent: %s ...") % bytes_sent << std::endl;

    // close(sockfd); 
    return 0; 
}