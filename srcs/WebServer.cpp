#include "../includes/WebServer.hpp"

void WebServer::printServer()
{

    
    for (int i = 0; i < serverNum; i++)
    {
        std::map<std::string, std::string>::iterator it = server[i].map.begin();
        std::map<std::string, std::string>::iterator ite = server[i].map.end();
        while (it != ite)
        {
            std::cout << it->first << " = " << it->second << std::endl;
            it++;
        }
        
        std::map<std::string, loca>::iterator loc_it = server[i].locat.begin();
        std::map<std::string, loca>::iterator loc_ite = server[i].locat.end();

        while (loc_it != loc_ite)
        {
            std::cout << RED << "location : " << BLUE << loc_it->first << std::endl; 
            std::cout << "  POST          :" << (loc_it->second.post ? "yes" : "no") << std::endl;
            std::cout << "  GET           :" << (loc_it->second.get ? "yes" : "no") << std::endl;
            std::cout << "  DELETE        :" << (loc_it->second.delet ? "yes" : "no") << std::endl;
            std::cout << "  default       :" << loc_it->second.defau << std::endl;
            std::cout << "  root          :" << loc_it->second.root << std::endl;
            std::cout << "  upload        :" << loc_it->second.upload << std::endl;
            std::cout << "  autoindex     :" << (loc_it->second.autoindex ? "yes" : "no") << std::endl;
            std::cout << "  redirect      :" << loc_it->second.redirect << std::endl;
            std::cout << "  CGI           :" << (loc_it->second.cgi ? "yes" : "no") << DEF <<std::endl;
            loc_it++;
        }
    }
}

void WebServer::checkDefaulLocation()
{
    for (int i = 0; i < serverNum; i++)
    {
        std::map<std::string, loca>::iterator loc_it = server[i].locat.begin();
        std::map<std::string, loca>::iterator loc_ite = server[i].locat.end();
        int def = 0;
        while (loc_it != loc_ite)
        {
            if (loc_it->first == "/")
                def++;
            loc_it++;
        }
        if (def == 0){
            std::cout << server[i].confCherch("server_name")<< " : YOU SHOULD ENTER DEFAULT LOCATION!" << std::endl;
            exit(0);
        }
    }
}

void WebServer::servClient(int &i, int &fd)
{
    if(client[fd].endOf != (size_t)-1)
    {
        try
        {
            client[fd].res.generateResponse(fd, client[fd].req, server[client[fd].server_index], events[i]);                            
            if (epoll_ctl(epoll_fd, EPOLL_CTL_MOD, fd, &events[i]) == -1) {
                perror("epoll_ctl");
                if(client[fd].res.pid != -1)
                {
                    kill(client[fd].res.pid , SIGTERM);
                    waitpid(client[fd].res.pid, NULL, 0);
                }
                client[fd].req.connexion = true;
                throw (client[fd].res.serverError(server[client[fd].server_index].confCherch("500"), client[fd].req));
            }
        }
        catch(bool a)
        {
            if(a)
            {
                std::cout << BLUE << server[client[fd].server_index].confCherch("server_name") << RED << "     Client disconnected     	    " << DEF << inet_ntoa(client[fd].addr.sin_addr) << std::endl;
                if (epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, NULL) == -1) 
                {
                    
                    perror("epoll_ctl");
                    if(client[fd].res.pid != -1)
                    {
                        kill(client[fd].res.pid, 9);
                        waitpid(client[fd].res.pid, NULL, 0);
                    }
                }
                client[fd].clear();
                clientOut[fd] = -1;
                close(fd);
            }
        }
        catch(std::string &content)
        {
            if(!client[fd].res.firstExcep)
            {

                if(send(fd, content.c_str(), content.size(), 0) < 0)
                {
                    if(client[fd].res.pid != -1)
                    {
                        kill(client[fd].res.pid , SIGTERM);
                        waitpid(client[fd].res.pid, NULL, 0);
                    }
                    client[fd].req.connexion = true;
                }
                client[fd].req.firstTime = false;
                client[fd].res.firstExcep = true;

            }
            else
            {
                std::string cont = client[fd].res.getResource(client[fd].res.file, client[fd].req, server[client[fd].server_index]);

                if(send(fd, cont.c_str(), cont.size(), 0) < 0)
                {
                    if(client[fd].res.pid != -1)
                    {
                        kill(client[fd].res.pid , SIGTERM);
                        waitpid(client[fd].res.pid, NULL, 0);
                    }
                    client[fd].req.connexion = true;
                }
            }
        }
    }
}

void WebServer::createServer(const char * os)
{
    if(!os)
    {
        conf.defaultConfic();
        conf.displayLocation();
        server[0] = conf;
        serverNum = 1;
    }
    else
    {
        std::ifstream fg(os);
        if (fg.is_open())
        {
            serverNum = 0;
            while (!fg.eof()){
                Conf conf(fg);
                server[serverNum] = conf;
                serverNum += conf.numOfserver;
            }
            fg.close();
        }
        else
        {
            std::cerr << "ERROR : FILE NOT FOUND!" << std::endl;
            exit(0);
        }
    }
    if(serverNum == 0)
    {
        std::cerr << "ERROR : SERVER NOT FOUND!" << std::endl;
        exit(0);
    }
    checkDefaulLocation();
    int opt = 1;
    epoll_fd = epoll_create1(0);
    if (epoll_fd == -1){
        perror("epoll create");
        exit(0);
    }
    addrlen = sizeof(socketadress);
    for(int i = 0; i < serverNum; i++)
    {   
        socket_fd[i] = socket(AF_INET,SOCK_STREAM,0);
        socket_acc.push_back(socket_fd[i]);
        setsockopt(socket_fd[i],SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt));
        socketadress.sin_family = AF_INET;
        socketadress.sin_port = htons(atoi(server[i].confCherch("port").c_str()));
        socketadress.sin_addr.s_addr = inet_addr(server[i].confCherch("host").c_str());
        int bin = bind(socket_fd[i],(struct  sockaddr*)&socketadress,sizeof(socketadress));
        if (bin < 0)
        {
            perror("bind");
            if(serverNum == 1)
                exit(0);
        }
        else
        {
            std::cout << "listning [*]\n";
            std::cout << "";
            std::cout << "      host :"<< BLUE << ((server[i].confCherch("host")) == "0" ? "127.0.0.1" : inet_ntoa(socketadress.sin_addr)) << DEF << std::endl;
            std::cout << "      port :"<< BLUE  << server[i].confCherch("port") << DEF << std::endl;
        }
        listen(socket_fd[i],SOMAXCONN);
        if (socket_fd[i] == -1){
            perror("could not create socket");
			if(serverNum == 1)
            	exit(0);
        }
        event.data.fd = socket_fd[i];
        event.events = EPOLLIN ;
        if (epoll_ctl(epoll_fd,EPOLL_CTL_ADD,socket_fd[i],&event) == -1){
            perror("epoll_ctl");
            if(serverNum == 1)
				exit(0);
        }
    }
}


int WebServer::wait_event()
{
    int wait_fd = epoll_wait(epoll_fd, events, MAX_EVENTS, 10000);
    if(wait_fd == 0)
    {
        for (int i = 0; i < MAX_EVENTS; i++)
        {
            if(clientOut[i] != -1 && client[i].endOf == (size_t)-1)
            {
                client[i].req.firstTime = true;
                std::string res = client[i].res.timeOut(server[client[i].server_index].confCherch("408"), client[i].req);
                send(i, res.c_str(), res.size(), 0);
                std::cout << BLUE << server[client[i].server_index].confCherch("server_name")<< " " << RED << "     Client disconnected         " << DEF << inet_ntoa(client[i].addr.sin_addr) << std::endl;
                if (epoll_ctl(epoll_fd, EPOLL_CTL_DEL, i, NULL) == -1) {
                    perror("epoll_ctl");
                }
                client[i].clear();
                close(i);
                clientOut[i] = -1;
            }
        }
    }
    if (wait_fd == -1){
        perror("epoll_wait");
        exit(0);
    }
    return wait_fd;
}

int WebServer::acccept_newconnection(std::vector<int>::iterator it_serv,int & fd)
{
    new_socketfd = accept(fd, (struct sockaddr *)&clientaddr, &addrlen);
    
    if (new_socketfd == -1){
        if ((errno == EAGAIN) || (errno == EWOULDBLOCK)){
            perror("accept");
            return 1;
        }
    }
    event.data.fd = new_socketfd;
    clientOut[new_socketfd] = new_socketfd;
    event.events = EPOLLIN;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, new_socketfd, &event) == -1)
        return 1;
    client[new_socketfd].addr = clientaddr;
    client[new_socketfd].server_index = std::distance(socket_acc.begin(), it_serv);
    std::string name = server[client[new_socketfd].server_index].confCherch("server_name");
    std::cout << BLUE << name << GREEN << "     Received connection from        " << DEF << inet_ntoa(client[new_socketfd].addr.sin_addr) << DEF << std::endl;
    return 0;
}

void WebServer::checkRequest(int &fd,ssize_t & bytes_read,int & i,char *buffer)
{
    if(client[fd].endOf == (size_t)-1 && bytes_read > -1 && events[i].events == EPOLLIN)
    {
        buffer[bytes_read] = '\0';
        client[fd].buf.write(buffer, bytes_read);
        client[fd].endOf = findEndOfHeaders(const_cast<char *>(client[fd].buf.str().c_str()) , (ssize_t)client[fd].buf.str().size());


        if(client[fd].endOf != (size_t)-1)
        {
            client[fd].req.pars(client[fd].buf, client[fd].endOf);
            client[fd].req.ra += (client[fd].buf.str().size() - client[fd].endOf);
            client[fd].req.body_limit = std::atof(server[0].confCherch("body_size_limit").c_str());
            if(client[fd].req.get_method() == "GET")
            {
                events[i].events = EPOLLOUT;
                if (epoll_ctl(epoll_fd, EPOLL_CTL_MOD, fd, &events[i]) == -1) {
                    
                    perror("epoll_ctl");
                    if(client[fd].res.pid != -1)
                    {
                        kill(client[fd].res.pid , SIGTERM);
                        waitpid(client[fd].res.pid, NULL, 0);
                    }
                    client[fd].req.connexion = true;
                }
            }
        }
    }
}

WebServer::WebServer(const char *os)
{
    createServer(os);
    std::cout<<"-----------------------------------------------------------\n";
	std::cout << "Server Name		 Event			address\n";
    for (size_t i = 0; i < MAX_EVENTS; i++)
    {
        client[i].endOf = -1;
        clientOut[i] = -1;
    }
    int lop = 1;
    while (1)
    {
        signal(SIGPIPE, SIG_IGN);
        int wait_fd = wait_event();
        for (int i = 0; i < wait_fd; i++)
        {
            int  fd = events[i].data.fd;
            std::vector<int>::iterator it_serv = std::find(socket_acc.begin(),socket_acc.end(),fd);
            if (it_serv != socket_acc.end())
            {
                if (acccept_newconnection(it_serv,fd) == 1)
                    continue;
            }
            else if ((events[i].events & EPOLLERR) || (events[i].events & EPOLLHUP)) {
                std::cout << BLUE << server[client[fd].server_index].confCherch("server_name") << RED << "     Client disconnected     	        " << DEF << inet_ntoa(client[fd].addr.sin_addr) << std::endl;
                if (epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, NULL) == -1) 
                {
                    
                    if(client[fd].res.pid != -1)
                    {
                        kill(client[fd].res.pid, SIGTERM);
                        waitpid(client[fd].res.pid, NULL, 0);
                    }
                    perror("epoll_ctl");
                }
                client[fd].clear();
                close(fd);
                clientOut[fd] = -1;
            }
            else
            {
                char buffer[1024];
                ssize_t bytes_read = -1;
                if(events[i].events == EPOLLIN && client[fd].endOf == (size_t)-1)
                {
                    if ((bytes_read = recv(fd, buffer, sizeof(buffer) - 1, 0)) == -1) 
                    {
                        
                        perror("recv");
                        if(client[fd].res.pid != -1)
                        {
                            kill(client[fd].res.pid , SIGTERM);
                            waitpid(client[fd].res.pid, NULL, 0);
                        }
                        bytes_read = 0;
                    }
                }

                if (bytes_read == 0) 
				{
                    std::cout << BLUE << server[client[fd].server_index].confCherch("server_name") << RED << "     Client disconnected     	    " << DEF << inet_ntoa(client[fd].addr.sin_addr) << std::endl;
                    if (epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, NULL) == -1) 
                    {
                        
                        if(client[fd].res.pid != -1)
                        {
                            kill(client[fd].res.pid, 9);
                            waitpid(client[fd].res.pid, NULL, 0);
                        }
                        perror("epoll_ctl");
                    }
                    client[fd].clear();
                    close(fd);
                    clientOut[fd] = -1;
                }
                else 
                {
                    checkRequest(fd,bytes_read,i,buffer);
                    servClient(i,fd);
                }
                if(client[fd].req.connexion)
                {
                    std::cout << BLUE << server[client[fd].server_index].confCherch("server_name") << RED << "     Client disconnected     	    " << DEF << inet_ntoa(client[fd].addr.sin_addr) << std::endl;
                    if (epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, NULL) == -1) 
                    {
                        
                        perror("epoll_ctl");
                        if(client[fd].res.pid != -1)
                        {
                            kill(client[fd].res.pid, 9);
                            waitpid(client[fd].res.pid, NULL, 0);
                        }
                    }
                    client[fd].clear();
                    clientOut[fd] = -1;
                    close(fd);
                }
            }
        }
        lop++;
    }
    close(socket_fd[MAX_EVENTS]);
}

WebServer::~WebServer(){}

