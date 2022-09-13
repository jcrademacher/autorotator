#ifndef STM23IP_H
#define STM23IP_H

#include <string>
#include <array>
#include <sys/types.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <netinet/in.h> 

#define MAX_RECV_BUF_SIZE 10
#define UDP_MOTOR_PORT 7775
#define TCP_MOTOR_PORT 7776

#define BIND_PORT 7777

#define CMD_ELECTRONIC_GEARING "EG"
#define CMD_SET_VELOCITY "VE"
#define CMD_SET_ACCELERATION "AC"
#define CMD_SET_DECELERATION "DE"
#define CMD_SET_POSITION "SP"
#define CMD_FEED_TO_POS "FP"

typedef enum {
    STM23IP_OK = 0,
    STM23IP_TIMEOUT = 1,
    STM23IP_ERROR = 2
} STM23IP_Status_t;

class STM23IP {
    public:
        STM23IP(std::string ip_address);

        STM23IP_Status_t send_recv_cmd(std::string cmd, std::string& resp, double param=-1, const int num_retries=0);
        STM23IP_Status_t send_cmd(std::string cmd, double param=-1, const int num_retries=0);
        STM23IP_Status_t enable();
        STM23IP_Status_t disable();
        STM23IP_Status_t alarm_reset();

        static double eSCL_read_code(std::string cmd);

        ~STM23IP();

    private:
        int sockfd;
        struct sockaddr_in servaddr; 

        static void str_to_eSCL(std::string cmd, uint8_t*& eSCL_cmd, size_t &eSCL_cmd_size);
        static void eSCL_to_str(std::string& response, uint8_t* eSCL_resp, size_t eSCL_resp_size);
};

#endif