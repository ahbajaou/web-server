
#include "../includes/WebServer.hpp"



int main(int ac,char *av[]) 
{

    (void)av;
    (void)ac;

    if (ac == 1)
        WebServer WebServer(NULL);
    else
        WebServer WebServer(av[1]);
    return 0;
}
