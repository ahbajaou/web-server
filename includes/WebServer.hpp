/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   WebServer.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ahbajaou <ahbajaou@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/17 03:09:09 by himejjad          #+#    #+#             */
/*   Updated: 2024/03/31 12:05:43 by ahbajaou         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#ifndef WEBSERVER_HPP
#define WEBSERVER_HPP

#include <sys/epoll.h>
#include <sys/socket.h>
#include <vector>
#include <map>
#include <arpa/inet.h>
#include <sys/wait.h>




#include "conf.hpp"
#include "../includes/Client.hpp"
class Client;

#include "../includes/Request.hpp"
#include "../includes/Response.hpp"
#define MAX_EVENTS 1024


class WebServer{
    private :
        Conf conf;
        int clientOut[MAX_EVENTS];
        Client client[MAX_EVENTS];
        int serverNum;
        struct sockaddr_in socketadress, clientaddr;
        socklen_t addrlen;
        struct epoll_event event, events[MAX_EVENTS];
        int socket_fd[MAX_EVENTS];
        int epoll_fd;
        int new_socketfd;
        std::vector<int> socket_acc;
    
    public :
        std::map<int,Conf> server;
        void printServer();
        void checkDefaulLocation();
        void servClient(int &i, int &fd);
        void createServer(const char * os);
        int wait_event();
        int acccept_newconnection(std::vector<int>::iterator it_serv,int & fd);
        void checkRequest(int &fd,ssize_t & bytes_read,int & i,char *buffer);
        WebServer(const char *os);
        ~WebServer();
};

#endif
