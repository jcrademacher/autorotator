#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/program_options.hpp>
#include <iostream>

#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <string> 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <netinet/in.h> 

#include "stm23ip.hpp"

#define MOTOR_IP "192.168.10.10"
#define PORT    43333 
#define MAXLINE 1024 

#define PULLEY_RATIO 0.5

namespace po = boost::program_options;

int main(int argc, char *argv[]) { 
  // setup the program options
    po::options_description desc("Allowed options");
    // clang-format off
    desc.add_options()
        ("help", "help message")
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

    
    STM23IP motor(MOTOR_IP, PORT);
        
        
    // int n;
    // socklen_t len;
        
    // ssize_t bytes_sent = sendto(sockfd, (const char *)hello.c_str(), hello.length(), 
    //     MSG_CONFIRM, (const struct sockaddr *) &servaddr,  
    //         sizeof(servaddr)); 

    // std::cout << boost::format("Bytes sent: %s ...") % bytes_sent << std::endl;

    // close(sockfd); 
    return 0; 
}