#ifndef STM23IP_H
#define STM23IP_H

#include <string>
#include <array>
#include <sys/types.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <netinet/in.h> 

#define MAX_RECV_BUF_SIZE 20
#define UDP_MOTOR_PORT 7775
#define TCP_MOTOR_PORT 7776

#define BIND_PORT 7777

#define CMD_ELECTRONIC_GEARING "EG"
#define CMD_SET_VELOCITY "VE"
#define CMD_SET_ACCELERATION "AC"
#define CMD_SET_DECELERATION "DE"
#define CMD_SET_POSITION "SP"
#define CMD_FEED_TO_POS "FP"
#define CMD_SET_MOVE_POS "DI"
#define CMD_MOTOR_ENABLE "ME"
#define CMD_MOTOR_DISABLE "MD"
#define CMD_IMMEDIATE_FORMAT_DEC "IFD"
#define CMD_IMMEDIATE_FORMAT_HEX "IFH"
#define CMD_IMMEDIATE_POSITION "IP"
#define CMD_CHANGE_CURRENT "CC"

#define STM23IP_INFO_STR "\u001b[32m[STM23IP INFO] \u001b[0m"
#define STM23IP_ERR_STR "\u001b[31m[STM23IP ERROR] \u001b[0m"
#define STM23IP_WARN_STR "\u001b[33m[STM23IP WARN] \u001b[0m"

typedef enum {
    STM23IP_OK = 0,
    STM23IP_TIMEOUT = 1,
    STM23IP_ERROR = 2
} STM23IP_Status_t;

class STM23IP {
    public:
        STM23IP(std::string ip_address);

        STM23IP_Status_t send_recv_cmd(std::string cmd, std::string& resp, double param);
        STM23IP_Status_t send_recv_cmd(std::string cmd, std::string& resp);
        
        STM23IP_Status_t send_cmd(std::string cmd, double param);
        STM23IP_Status_t send_cmd(std::string cmd);
        
        STM23IP_Status_t enable();
        STM23IP_Status_t disable();
        STM23IP_Status_t alarm_reset();

        static double eSCL_read_code(std::string cmd);
        static STM23IP_Status_t recv_cmd_loop(STM23IP* motor);

        static STM23IP_Status_t poll_position(STM23IP* motor, int32_t pos);

        int get_sockfd();

        ~STM23IP();

    private:
        int sockfd;
        struct sockaddr_in servaddr; 

        static void str_to_eSCL(std::string cmd, uint8_t*& eSCL_cmd, size_t &eSCL_cmd_size);
        static void eSCL_to_str(std::string& response, uint8_t* eSCL_resp, size_t eSCL_resp_size);
};

#endif