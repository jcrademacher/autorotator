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
#include <netinet/tcp.h>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>

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

double STM23IP::eSCL_read_code(std::string cmd) {
    size_t parse_i = cmd.find("=");

    if(parse_i == std::string::npos) {
        parse_i = 1;
    }

    double retval;

    try {
        retval = stod(cmd.substr(parse_i+1,std::string::npos));
    }
    catch(const std::exception& e) {
        std::cout << STM23IP_ERR_STR << e.what() << std::endl;
        throw;
    }

    return retval;
}

STM23IP::STM23IP(std::string ip_address) { 
    // Creating socket file descriptor
    std::cout << STM23IP_INFO_STR << "Connecting to motor..." << std::endl;

    std::string err_msg;
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) { 
        err_msg = "Socket creation failed";
        err_msg = STM23IP_ERR_STR + err_msg;
        perror(err_msg.c_str()); 
        exit(EXIT_FAILURE); 
    } 

    memset(&servaddr,0,sizeof(servaddr));

    // Filling server information 
    servaddr.sin_family = AF_INET; 
    servaddr.sin_port = htons(TCP_MOTOR_PORT); 
    servaddr.sin_addr.s_addr = inet_addr(ip_address.c_str());

    if(connect(sockfd,(struct sockaddr *)&servaddr,sizeof(servaddr)) < 0) {
        err_msg = "Could not connect";
        err_msg = STM23IP_ERR_STR+err_msg;
        perror(err_msg.c_str());
        exit(EXIT_FAILURE);
    } 
    
    std::string cmd = "RV";
    uint8_t* cmd_to_send;
    size_t cmd_size;
    int addr_len = sizeof(servaddr);
    
    str_to_eSCL(cmd,cmd_to_send,cmd_size);
    
    // get revision number, check comms with motor
    ssize_t bytes_sent;
    if((bytes_sent = send(sockfd,cmd_to_send,cmd_size,0)) < 0) {
        err_msg = "Socket send failure";
        err_msg = STM23IP_ERR_STR+err_msg;
        perror(err_msg.c_str());
        exit(EXIT_FAILURE); 
    }

    int bytes_recvd;
    uint8_t recv_buf[MAX_RECV_BUF_SIZE];

    struct timeval timeout; 

    timeout.tv_sec = 5;
    timeout.tv_usec = 0;

    if(setsockopt(sockfd,SOL_SOCKET,SO_RCVTIMEO,&timeout,sizeof(timeout))) {
        err_msg = "Socket timeout setting error";
        err_msg = STM23IP_ERR_STR + err_msg;
        perror(err_msg.c_str());
    }
    // removed TCP keepalive since not portable to mac
    // // set TCP keepalive
    // int flags = 1;

    // if(setsockopt(sockfd,SOL_SOCKET,SO_KEEPALIVE,(void *)&flags,sizeof(flags))) {
    //     err_msg = "Socket keepalive setting error";
    //     err_msg = STM23IP_ERR_STR + err_msg;
    //     perror(err_msg.c_str());
    // }

    // // set TCP keepidle
    // int keepidle = 5; // 10 seconds before keepalive is sent
    // if(setsockopt(sockfd,SOL_TCP,TCP_KEEPIDLE,(void *)&keepidle,sizeof(keepidle))) {
    //     err_msg = "TCP keepidle setting error";
    //     err_msg = STM23IP_ERR_STR + err_msg;
    //     perror(err_msg.c_str());
    // }

    // // set TCP keepints
    // int keepint = keepidle; // 1 seconds between keepalives
    // if(setsockopt(sockfd,SOL_TCP,TCP_KEEPINTVL,(void *)&keepint,sizeof(keepint))) {
    //     err_msg = "TCP keepints setting error";
    //     err_msg = STM23IP_ERR_STR + err_msg;
    //     perror(err_msg.c_str());
    // }

    if((bytes_recvd = recv(sockfd,(char *)recv_buf,MAX_RECV_BUF_SIZE,0)) < 0) {
        err_msg = "Receive timeout, failed to initialize motor";
        err_msg = STM23IP_ERR_STR + err_msg;
        perror(err_msg.c_str());
        exit(EXIT_FAILURE);
    }

    std::string resp;
    eSCL_to_str(resp,recv_buf,bytes_recvd);

    float firmware_version = ((float)stoi(resp.substr(cmd.length()+1))) / 100.0;

    std::cout << STM23IP_INFO_STR << boost::format("Motor Connected. Firmware version: %3.2f") % firmware_version << std::endl;
}

STM23IP::~STM23IP() {
    close(sockfd);
}

STM23IP_Status_t STM23IP::send_recv_cmd(std::string cmd, std::string& resp, double param) {
    std::string param_str = boost::lexical_cast<std::string>(param);
    cmd = cmd + param_str;
    
    return this->send_recv_cmd(cmd,resp);
}

STM23IP_Status_t STM23IP::send_recv_cmd(std::string cmd, std::string& resp) {
    STM23IP_Status_t status;
    std::string msg;

    if((status=this->send_cmd(cmd)) != STM23IP_OK) {
        return status;
    }

    int bytes_recvd, addr_len;
    addr_len = sizeof(servaddr);
    uint8_t recv_buf[MAX_RECV_BUF_SIZE];

    if((bytes_recvd = recv(sockfd,(char *)recv_buf,MAX_RECV_BUF_SIZE,0)) < 0) {
        msg = "Receive timeout";
        msg = STM23IP_WARN_STR+msg;
        perror(msg.c_str());
        return STM23IP_TIMEOUT;
    }

    eSCL_to_str(resp,recv_buf,bytes_recvd);

    return STM23IP_OK;
}

STM23IP_Status_t STM23IP::send_cmd(std::string cmd, double param) {
    std::string param_str = boost::lexical_cast<std::string>(param);
    cmd = cmd + param_str;

    return this->send_cmd(cmd);
}

STM23IP_Status_t STM23IP::send_cmd(std::string cmd) {
    uint8_t* cmd_to_send;
    size_t cmd_size;
    std::string err_msg;

    str_to_eSCL(cmd,cmd_to_send,cmd_size);
    
    // get revision number, check comms with motor
    ssize_t bytes_sent = send(sockfd,cmd_to_send,cmd_size,0);
    
    if (bytes_sent < 0) {
        err_msg = "Socket send failure";
        err_msg = STM23IP_ERR_STR+err_msg;
        perror(err_msg.c_str());
        return STM23IP_ERROR;
    }

    return STM23IP_OK;
}

STM23IP_Status_t STM23IP::enable() {
    return this->send_cmd("ME");
}

STM23IP_Status_t STM23IP::disable() {
    return this->send_cmd("MD");
}

STM23IP_Status_t STM23IP::alarm_reset() {
    return this->send_cmd("AR");
}

int STM23IP::get_sockfd() {
    return this->sockfd;
}

// spawn this function in thread
STM23IP_Status_t STM23IP::recv_cmd_loop(STM23IP* motor) {
    std::string resp, msg;
    int bytes_recvd, addr_len;
    uint8_t recv_buf[MAX_RECV_BUF_SIZE];

    int sockfd = motor->get_sockfd();

    while(true) {
        memset(&recv_buf,0,sizeof(recv_buf));

        if((bytes_recvd = recv(sockfd,(char *)recv_buf,MAX_RECV_BUF_SIZE,0)) < 0) {
            msg = "Receive timeout";
            msg = STM23IP_WARN_STR+msg;
            perror(msg.c_str());
            return STM23IP_TIMEOUT;
        }

        try {
            eSCL_to_str(resp,recv_buf,bytes_recvd);
        }
        catch(std::exception& e) {
            std::cerr << STM23IP_ERR_STR << e.what() << std::endl;
            return STM23IP_ERROR;
        }

        std::cout << std::endl << STM23IP_INFO_STR << boost::format("Received response from drive: %s") % resp << std::endl;
    }

    return STM23IP_OK;
}

STM23IP_Status_t STM23IP::poll_position(STM23IP* motor, int32_t pos) {
    int32_t immediate_pos;
    std::string resp;
    STM23IP_Status_t status;

    do {
        status = motor->send_recv_cmd(CMD_IMMEDIATE_POSITION, resp);
        immediate_pos = (int32_t)round(eSCL_read_code(resp));
    } while(immediate_pos != pos && status == STM23IP_OK);

    return status;
}
