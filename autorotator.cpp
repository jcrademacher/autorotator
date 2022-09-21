#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/program_options.hpp>
#include <iostream>
#include <cmath>
#include <signal.h>

#include "stm23ip.hpp"

#define MOTOR_IP "192.168.1.20"
#define PORT    43333 

#define PULLEY_RATIO 0.5

#define ANGLE_FLAG "?"

#define DEFAULT_EG 200
#define DEFAULT_AC 0.5
#define DEFAULT_DE 0.5
#define DEFAULT_VE 0.25
#define DEFAULT_SP 0

#define ANGLE_BOUND_UPPER 180
#define ANGLE_BOUND_LOWER -180

namespace po = boost::program_options;

// function headers
static int _main(int argc, char *argv[]);
void signal_callback_handler(int signum);
static void interactive_loop(STM23IP* motor, std::string& exec);
static int32_t actual_angle(double& requested_angle);
static void call_exec(std::string exec, double angle);

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
    signal(SIGINT, signal_callback_handler);

     // setup the program options
    po::options_description desc("Allowed options");
    std::string exec;

    std::string exec_help = std::string("executable to run at every rotation. To label the absolute rotation, place a '")+ std::string(ANGLE_FLAG)
        + std::string("' in the string and this program will replace it with the absolute rotation");

    double start_angle, end_angle, angle;
    double angle_step;
    bool interactive, zero, motor_disable;
    // clang-format off
    desc.add_options()
        ("help,h", "help message")
        ("exec,e", po::value<std::string>(&exec)->default_value(""), exec_help.c_str())
        ("inter,i",po::bool_switch(&interactive)->default_value(false),"Enable interactive mode")
        ("zero,z",po::bool_switch(&zero)->default_value(false),"Set zero position as current position of autorotator")
        ("disable,d",po::bool_switch(&motor_disable)->default_value(false),"Disable the motor. By default, the motor is enabled and the idling current is active, fixing the motor shaft in place. Set this flag if you want the motor shaft to not hold its position for zeroing.")
        ("start-angle", po::value<double>(&start_angle)->default_value(-90), "angle in degrees to begin the rotations at")
        ("end-angle", po::value<double>(&end_angle)->default_value(90), "angle in degrees to end the rotations at")
        ("step", po::value<double>(&angle_step)->default_value(1.8), "angle in degrees to step sweeping from start-angle to end-angle")
        ("angle",po::value<double>(&angle), "angle in degrees to immediately set to and exit")
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

    STM23IP_Status_t status = STM23IP_ERROR;
    std::string resp;

    STM23IP* motor = new STM23IP(MOTOR_IP);

    // initialize electronic gearing, position, and speeds
    double param_resp = 0;
    std::cout << "Initialiazing motor..." << std::endl;
    
    // WARNING: CHANGING THESE PARAMETERS AND OPERATING THE MOTOR MAY CAUSE PERMANENT MECHANICAL DAMAGE TO THE AUTOROTATOR
    motor->send_cmd(CMD_ELECTRONIC_GEARING,DEFAULT_EG);
    std::cout << boost::format("Set electronic gearing to %d steps/rev") % DEFAULT_EG << std::endl;

    motor->send_cmd(CMD_SET_VELOCITY,DEFAULT_VE);
    std::cout << boost::format("Set velocity to %d rev/sec") % DEFAULT_VE  << std::endl;

    motor->send_cmd(CMD_SET_ACCELERATION,DEFAULT_AC);
    std::cout << boost::format("Set acceleration to %d rev/sec/sec") % DEFAULT_AC << std::endl;

    motor->send_cmd(CMD_SET_DECELERATION,DEFAULT_DE);
    std::cout << boost::format("Set deceleration to %d rev/sec/sec") % DEFAULT_AC << std::endl;

    motor->send_cmd(CMD_IMMEDIATE_FORMAT_DEC);
    std::cout << "Set immediate format to decimal" << std::endl;

    if(motor_disable) {
        motor->send_cmd(CMD_MOTOR_DISABLE);
        std::cout << "Disabled motor" << std::endl;

        return 0;
    }
    else {
        motor->send_cmd(CMD_MOTOR_ENABLE);
        std::cout << "Enabled motor" << std::endl;
    }
    
    if(zero) {
        motor->send_cmd(CMD_SET_POSITION,0);
        std::cout << "Zeroed autorotator at current position" << std::endl;
    }
    // status = motor->send_cmd("SP0");

    // 
    // status = motor->send_cmd("AC1");
    // status = motor->send_cmd("DE1");
    // status = motor->send_cmd("VE0.25");

    if(interactive)
        interactive_loop(motor, exec);
    else {
        int32_t di_pos;
        if(vm.count("angle")) {
            di_pos = actual_angle(angle);
            std::cout << boost::format("Positioning autorotator to %.1f degrees...") % angle << std::endl;

            // tell motor to move to desired position
            motor->send_cmd(CMD_SET_MOVE_POS,(double)di_pos);
            motor->send_cmd(CMD_FEED_TO_POS);

            STM23IP::poll_position(motor, di_pos);

            if(exec.length() > 0) {
                call_exec(exec, angle);
            }
        }
        else {
            double setting_angle;
            for(double cur_ang = start_angle; cur_ang < end_angle; cur_ang += angle_step) {
                setting_angle = cur_ang;
                di_pos = actual_angle(setting_angle);
                std::cout << boost::format("Positioning autorotator to %.1f degrees...") % setting_angle << std::endl;

                // tell motor to move to desired position
                motor->send_cmd(CMD_SET_MOVE_POS,(double)di_pos);
                motor->send_cmd(CMD_FEED_TO_POS);

                STM23IP::poll_position(motor, di_pos);

                if(exec.length() > 0) {
                    call_exec(exec, setting_angle);
                }
            }
        }
    }

    return 0; 
}

void signal_callback_handler(int signum) {
   std::cout << "Caught signal " << signum << std::endl;
   // Terminate program
   exit(signum);
}

void interactive_loop(STM23IP* motor, std::string& exec) {
    std::string input, eSCL_cmd;
    std::string::size_type sz;
    double angle;
    bool is_eSCL_cmd, input_failed, contains_param;

    STM23IP_Status_t status = STM23IP_ERROR;

    while(true) {
        input_failed = false;
        contains_param = false;
        std::cout << "Enter an angle or SP command: ";
        std::cin >> input;

        try {
            angle = stof(input,&sz);
            is_eSCL_cmd = false;
        }
        catch(...) {
            eSCL_cmd = input;
            is_eSCL_cmd = true;
            
            if(eSCL_cmd.length() < 2) {
                input_failed = true;
            }
            else if(eSCL_cmd.substr(2,std::string::npos).length() > 0) {
                contains_param = true;
            }
        }

        if(!is_eSCL_cmd && (angle > ANGLE_BOUND_UPPER || angle < ANGLE_BOUND_LOWER)) {
            std::cout << boost::format("Angle must be between %d and +%d degrees") % ANGLE_BOUND_LOWER % ANGLE_BOUND_UPPER << std::endl;
            input_failed = true;

            continue;
        }

        if(!input_failed && !is_eSCL_cmd) {
            int32_t di_pos = actual_angle(angle);

            std::cout << boost::format("Positioning autorotator to %.1f degrees...") % angle << std::endl; 

            // tell motor to move to desired position
            motor->send_cmd(CMD_SET_MOVE_POS,(double)di_pos);
            motor->send_cmd(CMD_FEED_TO_POS);

            // blocks until position is met
            STM23IP::poll_position(motor, di_pos);

            if(exec.length() > 0) {
                call_exec(exec,angle);
            }
        }
        else if(is_eSCL_cmd && !input_failed) {
            std::string resp;
    
            if(contains_param) {
                if(eSCL_cmd.compare(CMD_SET_POSITION) >= 0) {
                    status = motor->send_cmd(eSCL_cmd);
                }
                else {
                    std::cout << "WARNING: Will not send eSCL 'set' commands except SP" << std::endl; 
                }
            }
            else {
                if(eSCL_cmd.compare(CMD_MOTOR_DISABLE) == 0 || eSCL_cmd.compare(CMD_MOTOR_ENABLE) == 0) {
                    status = motor->send_cmd(eSCL_cmd);
                }
                else {
                    status = motor->send_recv_cmd(eSCL_cmd, resp);
                }
            }

            if(status != STM23IP_OK) {
                std::cout << "eSCL command could not be sent" << std::endl;
            }

            if(resp.length() > 0) {
                std::cout << boost::format("Received response from drive: %s") % resp << std::endl;
            }
        }
        else {
            std::cerr << "Error: ill-formed command" << std::endl;
        }
    }
}

int32_t actual_angle(double& requested_angle) {
    int32_t di_pos = (int32_t) round((double)requested_angle / double(PULLEY_RATIO) * ((double)DEFAULT_EG) / 360.0); 
    requested_angle = ((double)di_pos) * 360.0 / ((double)DEFAULT_EG) * double(PULLEY_RATIO);

    return di_pos;
}

void call_exec(std::string exec, double angle) {
    size_t angle_string_location = exec.find(ANGLE_FLAG);
    std::string exec_to_call;
    std::ostringstream angle_str;

    int ipart;
    int fpart;

    ipart = ((int)round(angle*10.0))/10;
    fpart = abs((int)(round((angle - ipart)*10.0)));

    angle_str << boost::format("%d,%1d") % ipart % fpart;
    exec_to_call = exec;
    
    if(angle_string_location != std::string::npos)
        exec_to_call.replace(angle_string_location, 1, angle_str.str());

    std::cout << boost::format("Calling executable: %s") % exec_to_call << std::endl;
    system(exec_to_call.c_str());

    angle_str.str("");
    angle_str.clear();
}