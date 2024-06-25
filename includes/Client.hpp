#ifndef CLIENT
#define CLIENT
#include <sstream>
#include "Request.hpp"
#include "Response.hpp"
#include <arpa/inet.h>

class Client
{
    public:
        std::stringstream buf;
        bool toRespons;
        int server_index;
        size_t endOf;
        Request req;
        Response res;
        struct  sockaddr_in addr;
        
        Client();
        void clear();
        ~Client();

};

#endif
