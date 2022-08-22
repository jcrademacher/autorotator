#ifndef STM23IP_H
#define STM23IP_H

#include <string>
#include <array>
#include <sys/types.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <netinet/in.h> 

#define MAX_RECV_BUF_SIZE 10

typedef enum {
    STM23IP_OK = 0,
    STM23IP_TIMEOUT = 1,
    STM23IP_ERROR = 2
} STM23IP_Status_t;

class STM23IP {
    public:
        STM23IP(std::string ip_address, size_t port);

        STM23IP_Status_t send_recv_cmd(std::string cmd, std::string& resp, const int num_retries);

        ~STM23IP();

    private:
        int sockfd;
        struct sockaddr_in servaddr; 

        static void str_to_eSCL(std::string cmd, uint8_t*& eSCL_cmd, size_t &eSCL_cmd_size);
        static void eSCL_to_str(std::string& response, uint8_t* eSCL_resp, size_t eSCL_resp_size);
};

#endif