#include "../includes/Client.hpp"


Client::Client()
{
    server_index = 0;
}

void Client::clear()
{
    buf.str("");
    buf.clear();
    toRespons = false;
    endOf = -1;
    server_index = 0;
    req.clear();
    res.clear();
    // std::cout << RED <<"clear Client object" << DEF<< std::endl;
}


Client::~Client()
{
    
}
