#include "stm23ip.hpp"

#include <iostream>
#include <array>
#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <string> 
#include <cstring>
#include <sys/types.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <netinet/in.h> 
#include <boost/format.hpp>

void STM23IP::str_to_eSCL(std::string cmd, uint8_t*& eSCL_cmd, size_t &eSCL_cmd_size) {
    eSCL_cmd_size = cmd.size()+3;
    eSCL_cmd = new uint8_t[eSCL_cmd_size]; 
    eSCL_cmd[0] = 0;
    eSCL_cmd[1] = 7;
    eSCL_cmd[eSCL_cmd_size-1] = 13;
    
    memcpy(&eSCL_cmd[2],cmd.c_str(),cmd.size());
}

void STM23IP::eSCL_to_str(std::string& response, uint8_t* eSCL_resp, size_t eSCL_resp_size) {
    response = std::string((const char *)eSCL_resp, eSCL_resp_size);
    response.erase(response.begin(),response.begin()+2);
    response.pop_back();
}

STM23IP::STM23IP(std::string ip_address, size_t port) { 
    // Creating socket file descriptor 
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) { 
        perror("Socket creation failed"); 
        exit(EXIT_FAILURE); 
    } 

    // Filling server information 
    servaddr.sin_family = AF_INET; 
    servaddr.sin_port = htons(port); 
    // inet aton converts IP c string into network ordered IP address
    inet_aton(ip_address.c_str(), &servaddr.sin_addr); 
    
    std::string cmd = "RV";
    uint8_t* cmd_to_send;
    size_t cmd_size;
    
    str_to_eSCL(cmd,cmd_to_send,cmd_size);
    
    // get revision number, check comms with motor
    ssize_t bytes_sent;
    if((bytes_sent = sendto(sockfd,cmd_to_send,cmd_size,MSG_CONFIRM, (const struct sockaddr *) &servaddr,sizeof(servaddr))) < 0) {
        perror("Socket send failure");
        exit(EXIT_FAILURE); 
    }

    int bytes_recvd, addr_len;
    uint8_t recv_buf[MAX_RECV_BUF_SIZE];

    struct timeval timeout;

    timeout.tv_sec = 1;
    timeout.tv_usec = 0;

    setsockopt(sockfd,SOL_SOCKET,SO_RCVTIMEO,&timeout,sizeof(timeout));

    if((bytes_recvd = recvfrom(sockfd,(char *)recv_buf,MAX_RECV_BUF_SIZE,MSG_WAITALL,(struct sockaddr *)&servaddr,(socklen_t *)&addr_len)) < 0) {
        perror("Receive timeout, failed to initialize motor");
        exit(EXIT_FAILURE);
    }

    std::string resp;
    eSCL_to_str(resp,recv_buf,bytes_recvd);

    float firmware_version = ((float)stoi(resp.substr(3))) / 100.0;

    std::cout << boost::format("STM23IP Connected. Firmware version: %f") % firmware_version << std::endl;
}

STM23IP::~STM23IP() {
    close(sockfd);
}

STM23IP_Status_t STM23IP::send_recv_cmd(std::string cmd, std::string& resp, const int num_retries=0) {
    uint8_t* cmd_to_send;
    size_t cmd_size;
    
    str_to_eSCL(cmd,cmd_to_send,cmd_size);
    
    // get revision number, check comms with motor
    ssize_t bytes_sent;
    int attempts = 0;
    while((bytes_sent = sendto(sockfd,cmd_to_send,cmd_size,MSG_CONFIRM, (const struct sockaddr *) &servaddr,sizeof(servaddr))) < 0 && attempts <= num_retries) {
        ++attempts;
    }
    
    if (bytes_sent < 0) {
        perror("Socket send failed");
        return STM23IP_ERROR;
    }

    int bytes_recvd, addr_len;
    uint8_t recv_buf[MAX_RECV_BUF_SIZE];

    while((bytes_recvd = recvfrom(sockfd,(char *)recv_buf,MAX_RECV_BUF_SIZE,MSG_WAITALL,(struct sockaddr *)&servaddr,(socklen_t *)&addr_len)) < 0) {
        perror("Receive timeout, failed to initialize motor");
        return STM23IP_TIMEOUT;
    }

    eSCL_to_str(resp,recv_buf,bytes_recvd);
    return STM23IP_OK;
}
