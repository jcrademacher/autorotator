#ifndef STM23IP_H
#define STM23IP_H

#include <string>

class STM23IP {
    public:
        STM23IP(std::string ip_address, size_t port);

    private:
        int sockfd;
};

#endif