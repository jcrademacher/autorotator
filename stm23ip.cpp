#include "stm23ip.hpp"

#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <string> 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <netinet/in.h> 

STM23IP::STM23IP(std::string ip_address, size_t port) { 
    std::string hello = "testing123"; 
    struct sockaddr_in servaddr; 
    
    // Creating socket file descriptor 
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) { 
        perror("socket creation failed"); 
        exit(EXIT_FAILURE); 
    } 

    // Filling server information 
    // servaddr.sin_family = AF_INET; 
    // servaddr.sin_port = htons(port); 
    // inet_aton(MOTOR_IP, &servaddr.sin_addr); 
}