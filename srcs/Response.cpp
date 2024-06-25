#include "../includes/Response.hpp"
#include "../includes/Request.hpp"
#include "../includes/conf.hpp"
#include <sys/socket.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <unistd.h>
#include <ctime>
#include <sys/stat.h>
#include <sys/wait.h>


void Response::generateResponse(int &fd, Request &req, Conf &server , epoll_event &event)
{
    if(req.firstTime)
    {
        checkHeaders(req, server);
        std::string _path = req.get_path();
        while (server.locat.find(_path + "/") == server.locat.end())
        {
            size_t e = _path.rfind("/");
            _path = _path.substr(0, e);
        }
        if (server.locat.find(_path + "/") != server.locat.end())
        {
            loca _location = server.locat.find(_path + "/")->second;
            req.root_end = strlen((_location.root).c_str());
            req.get_path() = _location.root + req.get_path();
            req.red_path = _location.redirect;
            req.locationPath = (_path + "/");
        }
        if((req.get_method() == "GET" && !(server.locat.find(req.locationPath)->second.get))
            || (req.get_method() == "DELETE" && !(server.locat.find(req.locationPath)->second.delet))
            || (req.get_method() == "POST" && !(server.locat.find(req.locationPath)->second.post)))
        {
            throw (notAllow(req.get_method(), server.confCherch("405"), req));
        }
    }
    if (req.get_method() == "GET")
    {
        GET(fd, req, server);
    }
    else if (req.get_method() == "POST")
    {
        if (server.locat.find(req.locationPath) != server.locat.end() && !(server.locat.find(req.locationPath)->second.upload.empty()))
        {
            POST(fd, req, server, event);
        }
        else if(server.locat.find(req.locationPath) != server.locat.end()
            && (server.locat.find(req.locationPath)->second.cgi) && (extension(req.get_path()) == "php" || extension(req.get_path()) == "py"))
        {
            req.query = req.get_body();
            GET(fd, req, server);
            event.events = EPOLLOUT;
        }
        else
        {
            throw(notImplement(server.confCherch("501"), req));
        }
    }
    else if (req.get_method() == "DELETE")
    {
        int d = DELETE(fd, req, server, req.get_path());
        if(d == 1)
        {   
            std::string response = DeleteSuc();

            if(send(fd, response.c_str(), response.size(), 0) == -1)
            {
                perror("send :");
                throw true;
            }
            req.connexion = true;
        }
        else if(d == -1)
            throw (forbidden(server.confCherch("403"), req));
        else
        {
            std::string response = noContent();
            if (send(fd, response.c_str(), response.size(), 0) == -1)
                throw true;
            req.connexion = true;
        }
    }
    else
    {
        req.connexion = true;
    }
}

void Response::serv_file(map_iterator &type, int &fd, Request &req, Conf &server)
{

    if (req.firstTime)
    {
        file = open(req.get_path().c_str(), O_RDONLY | O_NONBLOCK);
        if (file < 0)
        {
            throw(notFound(server.confCherch("404"), req));
        }
        std::ifstream ff(req.get_path().c_str(), std::ios::binary);

        ff.seekg(0, std::ios::end);
        std::streampos size = ff.tellg();
        ff.seekg(0, std::ios::beg);
        ff.close();

        std::stringstream response;
        response << "HTTP/1.1 200 OK\r\n"
                 << "Content-Type: " << type->second << "\r\n"
                 << "Content-Length: " << size << "\r\n"
                 << "Connection: keep-alive\r\n"
                 << "Date: " << getCurrentDateTime() << "\r\n"
                 << "\r\n";
        if (send(fd, response.str().c_str(), response.str().size(), 0) ==-1)
            throw true;
        req.firstTime = false;
    }
    else
    {
        std::string content = getResource(file, req, server);
        if (send(fd, content.c_str(), content.size(), 0) == -1)
            throw true;
    }
}

void Response::serv_dir(int &fd, Request &req, Conf &server)
{
    std::string _path = (req.get_path());
    if (_path == SERVER_ROOT)
        throw(badRequest(server.confCherch("400"), req));

    else
    {
        if (*(_path.end() - 1) == '/')
        {
            if (server.locat.find(req.locationPath) != server.locat.end()
                && server.locat.find(req.locationPath)->second.autoindex)
            {
                std::string content = listDirectory(_path.c_str());
                std::stringstream response;
                response << "HTTP/1.1 200 OK\r\n"
                         << "Content-Type: text/html\r\n"
                         << "Content-Length: " << content.size() << "\r\n"
                         << "Connection: close\r\n"
                         << "Server: chabchoub\r\n"
                         << "Date: " << getCurrentDateTime() << "\r\n"
                         << "\r\n"
                         << content;
                if (send(fd, response.str().c_str(), response.str().size(), 0) == -1)
                    throw true;
                req.connexion = true;
                req.firstTime = false;
            }
            else
            {
                _path += server.locat.find(req.locationPath)->second.defau;
                std::map<std::string, std::string> mime_map = mimeTypes();
                map_iterator it = mime_map.find(extension(_path));
                if (it != mime_map.end() && fileExists(_path))
                {
                    req.get_path() = _path;
                    if (access(req.get_path().c_str(), R_OK) != 0)
                        throw (forbidden(server.confCherch("403"),req));
                    serv_file(it, fd, req, server);
                }
                else if (server.locat.find(req.locationPath)->second.cgi && (extension(_path) == "php" || extension(_path) == "py"))
                {
                    req.get_path() = _path;
                    if (access(req.get_path().c_str(), X_OK) != 0)
                        throw (forbidden(server.confCherch("403"),req));
                    if(!serveCgi(req,fd))
                    {
                        if (cgirespons)
                        {
                            fclose(output_file);
                            std::ifstream ff(temp_file.c_str());
                            std::stringstream response;
                            std::string res = std::string(std::istreambuf_iterator<char>(ff), std::istreambuf_iterator<char>()); 
                            response << "HTTP/1.1 200 OK\r\n";
                            if(!req.get_header("Cookie").empty())
                                   response << "Cookie: "<< req.get_header("Cookie") <<"\r\n";
                            response << res;
                            printf("%s\n", response.str().c_str());
                            if (send(fd, response.str().c_str() , response.str().size(), 0) == -1)
                                throw true;
                            req.connexion = true;
                            ff.close();
                        }
                    }
                    else
                    {
                        throw (serverError(server.confCherch("500"), req));
                    }
                    req.firstTime = false;
                }
                else
                {
                    throw(forbidden(server.confCherch("403"), req));
                }
            }
        }
        else
        {
            std::string location = (_path.substr(req.root_end)) + "/";
            Redirect(location, req, fd, server);
        }
    }
}

void Response::checkHeaders(Request &req, Conf &server)
{
    std::map<std::string, std::string> head;
    head = req.get_headers();
    if (head.count("Transfer-Encoding:") && req.get_header("Transfer-Encoding:") != "chunked")
    {
        throw(notImplement(server.confCherch("501"), req));
    }
    if (req.get_method() == "POST" && !head.count("Content-Type:"))
    {
        throw(notImplement(server.confCherch("501"), req));
    }
    if((req.get_method() != "GET") && (req.get_method() != "POST") && (req.get_method() != "DELETE"))
    {
        throw (badRequest(server.confCherch("400"), req));
    }
    if(req.get_method().empty() || req.get_path().empty() || !req.startLineForma)
    {
        throw(badRequest(server.confCherch("400"), req));
    }
    if(head.count("Content-Length:") && std::atol(req.get_header("Content-Length:").c_str()) <= 0)
    if(req.get_path().empty() || req.get_path()[0] != '/')
    {
        throw(badRequest(server.confCherch("400"), req));
    }
    if (req.get_method() == "POST" && !head.count("Transfer-Encoding:") && !head.count("Content-Length:"))
    {
        throw(lengthRequired(server.confCherch("411"), req));
    }
    if ((req.get_method() == "GET") && (!req.get_body().empty() || head.count("Content-Length:")))
    {
        throw(badRequest(server.confCherch("400"), req));
    }
    if (head.count("Transfer-Encoding:") && head.count("Content-Length:"))
    {
        throw(badRequest(server.confCherch("400"), req));
    }
    if (req.get_protocol().empty() || req.get_protocol() != "HTTP/1.1")
    {
        throw(httpVersion(server.confCherch("505"), req));
    }
    if (req.body_limit < std::atol(req.get_header("Content-Length:").c_str()))
    {
        throw(EntityTooLarge(server.confCherch("413"), req));
    }
    if (req.get_path().size() > 2048)
    {
        throw(longRequest(server.confCherch("414"), req));
    }
}

Response::Response()
{
}

Response::~Response()
{
}

std::string Response::getResource(int &file, Request &req, Conf &server)
{
    (void)server;
    std::stringstream content;
    char buffer[1024];
    size_t a = read(file, buffer, 1023);
    if (a == (size_t)-1)
    {
        throw true;
    }
    if (a < 1023)
        req.connexion = true;
    content.write(buffer, a);
    return content.str();
}

std::string Response::getRedirctionS(std::string &location)
{
    std::stringstream response;
    response << "HTTP/1.1 301 Moved Permanently\r\n"
             << "Location: " << location << "\r\n"
             << "Date: " << getCurrentDateTime() << "\r\n"
             << "\r\n";
    return response.str();
}


std::string Response::extension(const std::string &path)
{
    size_t dot = path.rfind('.');
    if (dot == std::string::npos)
    {
        std::cerr << RED << "failure to find extension !" << DEF << std::endl;
        return "null";
    }
    return path.substr(dot + 1);    
}


void Response::clear()
{
    out.close();
    close(file);
    chunked.clear();
    decimal = 0;
    str.clear();
    tmp.clear();
    path.clear();
    file = -1;
    pid = -1;
    firstcgi = false;
    cgirespons = false;
    timeout = false;
    output_file = NULL;
    temp_file.clear();
    firstExcep = false;
    end = 0;
    tmp_path.clear();
}

void Response::Redirect(std::string &location, Request &req, int &fd, Conf &server)
{
    (void)server;
    std::string content = getRedirctionS(location);
    if(send(fd, content.c_str(), content.size(), 0) == -1)
        throw true;
    req.connexion = true;
    req.firstTime = false;
}

#include <dirent.h>
std::string listDirectory(const char *path)
{
    std::stringstream response;
    response << "<html><head><title>Directory Listing</title>"
             << "<link rel=\"stylesheet\" href=\"https://fonts.googleapis.com/icon?family=Material+Icons\">"
             << "<link rel=\"stylesheet\" href=\"https://fonts.googleapis.com/css2?family=Material+Symbols+Outlined:opsz,wght,FILL,GRAD@24,400,0,0\" />"
             << "<style>"
             << ".li , a{ display : flex; gap : 4px;align-items : center; width: 30%;} "
             << "a:hover {background-color: #00000059; border-radius : 2px; }"
             << "a {color: black; font-size: 14px;}"
             << "li, a { "
             << "text-decoration : none;"
             << "list-style-type: none;"
             << "} *{box-sizing : border-box;}"
             << "    body {"
             << "        display: flex;"
             << "        font-family: Arial, sans-serif;"
             << "        justify-content: center;"
             << "        align-items: center;"
             << "        height: 100vh;"
             << "        background-color: #f5f5f5;"
             << "    }"
             << "    .main-content {"
             << "        border-radius: 50px;"
             << "        display: block;"
             << "        width: 80%;"
             << "        height: 80%;"
             << "        justify-content: center;"
             << "        align-items: center;"
             << "        text-align: center;"
             << "        padding: 20px;"
             << "    }"
             << "    .main-content .list {"
             << "        list-style-type: none;"
             << "        display : flex; align-items : center; gap : 30px; width : 100%; margin: 30px ; flex-wrap : wrap"
             << "    }"
             << "    .main-content h1 {"
             << "        width: 40%;"
             << "        height: auto;"
             << "        display: block;"
             << "        margin: 0 auto;"
             << "        font-size: 40px;"
             << "        color: black;"
             << "    }"
             << "</style>"
             << "</head>"
             << "<body>"
             << "    <div class=\"main-content\">"
             << "    <h1>Directory Listing</h1>"
             << "    <hr size=\"2\" color=\"black\" width=\"90%\">"
             << "    <div class=\"list\">";

    DIR *dir;
    struct dirent *entry;

    if ((dir = opendir(path)) != NULL)
    {
        while ((entry = readdir(dir)) != NULL)
        {

            if ((int)entry->d_type == 4)
            {
                response << "<a href=\"" << entry->d_name << "\"> <li class='li'><i  style='color: #edbe00;' class=\"material-icons\">folder</i>";
            }
            else if ((int)entry->d_type == 8)
            {
                response << "<a href=\"" << entry->d_name << "\"> <li class='li'><i  style='color: #edbe00;'class='material-symbols-outlined'>draft</i>";
            }
            else
            {
                response << "<a href=\"" << entry->d_name << "\"> <li class='li'><i  style='color: #edbe00;'class='material-symbols-outlined'>unknown_document</i>";
            }
            response << entry->d_name << " </li></a>";
        }
        closedir(dir);
    }
    else
        perror("opendir");
    response << "</div></div>"
                << "</body></html>";
    return response.str();
}

std::string getCurrentDateTime()
{
    char buffer[80];
    std::time_t rawtime;
    std::tm *timeinfo;

    std::time(&rawtime);
    timeinfo = std::gmtime(&rawtime);

    std::strftime(buffer, 80, "%a, %d %b %Y %H:%M:%S GMT", timeinfo);
    return std::string(buffer);
}

bool directoryExists(std::string path)
{
    struct stat info;

    if (stat(path.c_str(), &info) != 0)
    {
        return false;
    }

    return (info.st_mode & S_IFDIR) != 0;
}

bool fileExists(std::string path)
{
    struct stat info;

    if (stat(path.c_str(), &info) != 0)
    {
        return false;
    }

    return (info.st_mode & S_IFREG) != 0;
}

size_t hexadecimal(const std::string &chunkHeader)
{
    std::istringstream iss(chunkHeader);
    size_t chunkSize;
    iss >> std::hex >> chunkSize;
    return chunkSize;
}

std::string random_name()
{
    char letters[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    int len = 5;
    srand(time(0));
    std::string name;
    name.reserve(len);
    for (int i = 0; i < len; i++)
        name += letters[rand() % (sizeof(letters) - 1)];
    return (name + ".txt");
}

int Response::serveCgi(Request &req, int &fd)
{
    std::string php_path = "/usr/bin/php-cgi";
    std::string py_path = "/usr/bin/python3";
    (void)fd;

    const char* args[3];
    char** env = new char*[8];
    if (!firstcgi)
    {
        firstcgi = true;
        start = clock();
        temp_file = "/tmp/cgi_" + random_name();
        const char* path = req.get_path().c_str();
        char resolved_path[PATH_MAX];
        std::string pp = realpath(path, resolved_path);
        output_file = fopen(temp_file.c_str(), "w");
        if (!output_file) {
            std::cerr << "Failed to open temporary file for writing." << std::endl;
            return 1;
        }
                    env[0] = new char [("QUERY_STRING=" + req.get_query()).size() + 1];
                    strcpy((char *)env[0], ("QUERY_STRING=" + req.get_query()).c_str());
                    env[1] = new char [("REQUEST_METHOD=" + req.get_method()).size() + 1];
                    strcpy((char *)env[1], ("REQUEST_METHOD=" + req.get_method()).c_str());
                    env[2] = new char [("SCRIPT_FILENAME=" + pp).size() + 1];
                    strcpy((char *)env[2], ("SCRIPT_FILENAME=" + pp).c_str());
                    env[3] = new char [21];
                    strcpy(env[3], "REDIRECT_STATUS=200");
                    if (req.get_method() != "POST"){
                        env[4] = new char [27];
                        strcpy((char *)env[4],("CONTENT_TYPE=\"text/html\""));
                    }
                    else{
                        env[4] = new char [("CONTENT_TYPE=" + req.get_header("Content-Type:")).size() + 1];
                        strcpy((char *)env[4],("CONTENT_TYPE="+ req.get_header("Content-Type:")).c_str());
                    };
                    env[6] = new char [("HTTP_COOKIE=" + req.get_header("Cookie:")).size() + 1];
                    strcpy(env[6],("HTTP_COOKIE=" + req.get_header("Cookie:")).c_str());
        if (extension(req.get_path()) == "php"){
            args[0] = php_path.c_str();
            args[1] = req.get_path().c_str();
            env[5] = new char [("SCRIPT_NAME=" + php_path).size() + 1];
            strcpy((char *)env[5],("SCRIPT_NAME=" + php_path).c_str());
        }
        else if (extension(req.get_path()) == "py"){
            env[5] = new char [("SCRIPT_NAME=" + py_path).size() + 1];
            strcpy((char *)env[5],("SCRIPT_NAME=" + py_path).c_str());
            args[0] = py_path.c_str();
            args[1] = req.get_path().c_str();
        }
        args[2]  = NULL;
        env[7] = NULL;
        pid = fork();
        if (pid == 0)
        {
        
            if (freopen(temp_file.c_str(), "w", stdout) == NULL) 
            {
                std::cerr << "Failed to freopen stdout." << std::endl;
                fclose(output_file);
                
                return 1 ;
            }
            if (execve(args[0], (char* const*)args, env) == -1) 
            {
                std::cerr << "Failed to execute CGI script." << std::endl;
                fclose(output_file);
            
                return 1;
            }
            int i = 0;
            while (env[i]){
                delete[] env[i];
                i++;
            }
            delete[] env;
        } 
    }
    else if (pid > 0)
    {
        int status;
        int WAIT_PID = waitpid(pid, &status, WNOHANG);
        if (WAIT_PID == -1){
            perror("waitpid");
            kill(pid, SIGKILL);
            waitpid(pid, &status, 0);
            req.connexion = true;
            return 1;
        }
        else if (WAIT_PID == 0)
        {
            end = (double)(clock() - start) / CLOCKS_PER_SEC;
            if (end >= 5.00){
                timeout = true;
                kill(pid, SIGKILL);
                waitpid(pid, &status, 0);
                firstcgi = false;
                fclose(output_file);
            }
        }
        else
            cgirespons = true;
    }
    else
    {
        std::cerr << "Fork failed." << std::endl;
        fclose(output_file);
        return 1;
    }

    return 0;
}
