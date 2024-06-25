#include "../includes/Response.hpp"
#include "../includes/Request.hpp"
#include "../includes/conf.hpp"
#include <sys/socket.h>
#include <signal.h>
#include <sys/epoll.h>
#include <limits.h>
#include <dirent.h>

void	Response::GET(int &fd, Request &req, Conf &server)
{
    if(!req.red_path.empty())
            Redirect(req.red_path, req, fd,server);
    else if(directoryExists(req.get_path()))
    {
        if (access(req.get_path().c_str(), R_OK) != 0)
            throw (forbidden(server.confCherch("403"),req));
        std::string root =  server.locat.find(req.locationPath)->second.root;
        const char* r_path = root.c_str();
        const char* path2 = req.get_path().c_str();
        char resolved_path[PATH_MAX];
        
        std::string roo_pa =  realpath(r_path, resolved_path);
        std::string rea_pa =  realpath(path2, resolved_path) ;
        
        if(strncmp(roo_pa.c_str() , rea_pa.c_str() , strlen(roo_pa.c_str())) != 0)
            throw forbidden(server.confCherch("403"), req);

        serv_dir(fd, req, server);
    }
    else if (fileExists(req.get_path()))
    {
        std::string root =  server.locat.find(req.locationPath)->second.root;
        const char* r_path = root.c_str();
        const char* path2 = req.get_path().c_str();
        char resolved_path[PATH_MAX];

        std::string roo_pa =  realpath(r_path, resolved_path);
        std::string rea_pa =  realpath(path2, resolved_path) ;
        
        if(strncmp(roo_pa.c_str() , rea_pa.c_str() , strlen(roo_pa.c_str())) != 0)
            throw forbidden(server.confCherch("403"), req);

        std::map<std::string , std::string> mime_map = mimeTypes();
        map_iterator it = mime_map.find(extension(req.get_path()));
        if(it != mime_map.end()) 
        {
            if (access(req.get_path().c_str(), R_OK) != 0)
                throw (forbidden(server.confCherch("403"),req));
            serv_file(it, fd, req, server);
        }
        else if (fileExists(req.get_path()) && server.locat.find(req.locationPath)->second.cgi && (extension(req.get_path()) == "php" || extension(req.get_path()) == "py"))
        {
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
                    send(fd, response.str().c_str() , response.str().size(), 0);
                    req.connexion = true;
                    kill(pid, SIGKILL);
                    ff.close();
                }
                else if (timeout)
                {
                    req.connexion = true;
                    throw CGtimeOut();
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
            if(!server.locat.find(req.locationPath)->second.cgi && (extension(req.get_path()) == "php" || extension(req.get_path()) == "py"))
                throw(forbidden(server.confCherch("403"), req));
            else
                throw(mediaType(server.confCherch("415"), req));
        }
    }
    else
    {
        throw(notFound(server.confCherch("404"), req));
    }
}

unsigned long convertHexToDec(std::string hex)
{
    unsigned long decimal;
    std::stringstream ss;

    ss << hex;
    ss >> std::hex >> decimal;
    return (decimal);
}


void	Response::POST(int &fd, Request &req, Conf &server, epoll_event &event)
{
    static int i;
    if(req.get_header("Transfer-Encoding:").empty())
    {
        if (req.firstTime)
        {
            std::string up_ptah = server.locat.find(req.locationPath)->second.upload;
            std::string type = static_cast<std::string>(req.get_header("Content-Type:"));
            std::map<std::string, std::string> mime = post_type();
            map_iterator mmap = mime.find(type);
            if (mmap == mime.end())
            {
                throw (mediaType(server.confCherch("415"), req));
            }
            tmp_path = up_ptah + ("upload." + mmap->second);
            tmp_path = up_ptah + ("upload." + mmap->second);
            while (fileExists(tmp_path))
            {
                i++;
                std::stringstream ss;
                ss << i;
                std::string s;
                ss >> s;
                tmp_path = up_ptah + ("upload" + s + (".") + mmap->second);
            }
            std::string str = req.get_body();
            out.open(tmp_path.c_str(), std::ios::binary);
            out.write(str.c_str(), str.size());
            out.flush();
            req.firstTime = false;            
        }
        else if(req.ra < (ssize_t )atof(req.get_header("Content-Length:").c_str()))
        {
            ssize_t a;
            char buffer[1024];
            a = recv(fd, buffer, 1023, 0);
            if(a == -1)
                throw (true);
            req.ra += a;
            buffer[a] = '\0';
            out.write(buffer, a);
            out.flush();
        }
        if(req.ra >= (ssize_t )atof(req.get_header("Content-Length:").c_str()))
        {
            if (server.locat.find(req.locationPath)->second.cgi && (extension(req.get_path()) == "php" || extension(req.get_path()) == "py"))
            {
                std::ifstream ff(tmp_path.c_str());
                req.query = std::string(std::istreambuf_iterator<char>(ff), std::istreambuf_iterator<char>()); 
                ff.close();
                GET(fd, req, server);
                event.events = EPOLLOUT; 
            }
            else{
                req.connexion = true;
                std::string response = Created();
                if ( send(fd, response.c_str(), response.size(),0) == -1)
                    throw true;
            }
        }
    }
    else if(req.get_header("Transfer-Encoding:") == "chunked")
    {
        std::string line; 
        if (req.firstTime)
        {
            std::string up_ptah = server.locat.find(req.locationPath)->second.upload;
            std::string type = static_cast<std::string>(req.get_header("Content-Type:"));
            std::map<std::string, std::string> mime = post_type();
            map_iterator mmap = mime.find(type);
            if (mmap == mime.end())
            {
                throw (mediaType(server.confCherch("415"), req));
            }
            tmp_path = up_ptah + ("upload." + mmap->second);

            while (fileExists(tmp_path))
            {
                i++;
                std::stringstream ss;
                ss << i;
                std::string s;
                ss >> s;
                tmp_path = up_ptah + ("upload" + s + (".") + mmap->second);
            }
            out.open(tmp_path.c_str(), std::ios::binary);
            str = req.get_body();
            std::istringstream f(str);
            std::getline(f, line);
            if(str.substr(str.size() - 5, str.size()) == "0\r\n\r\n")
            {
                decimal = 0;
                str.erase(0,line.size() + 1);
                out.write(str.c_str(), str.size() - 7);
                out.flush();
            }
            else
            {
                decimal = convertHexToDec(line);
                str.erase(0,line.size() + 1);
                tmp.append(str.c_str(), str.size());
            }
            req.firstTime = false;
            req.chun++;
        }
        else if (decimal != 0)
        {
            ssize_t a;
            char buffer[1024];
            a = recv(fd, buffer, 1023, 0);
            if(a == -1)
                throw (true);
            buffer[a] = '\0';
            tmp.append(buffer, a);
            if((tmp.size() > (decimal + 10))
                || ((tmp.size() >= decimal + 5 ) && (*(tmp.end() - 1) == '\n') && (*(tmp.end() - 2) == '\r')))
            {
                ssize_t distance = tmp.size() - (decimal + 2);
                out.write(tmp.c_str(), decimal);
                out.flush();
                tmp = tmp.substr(decimal + 2, distance);
                std::istringstream f(tmp);
                std::getline(f, line);
                decimal = convertHexToDec(line);
                req.chun++;
                tmp.erase(0,line.size() + 1);
            }
        }
        if (decimal == 0)
        {
            if (server.locat.find(req.locationPath)->second.cgi && (extension(req.get_path()) == "php" || extension(req.get_path()) == "py"))
            {
                std::ifstream ff(tmp_path.c_str());
                req.query = std::string(std::istreambuf_iterator<char>(ff), std::istreambuf_iterator<char>()); 
                ff.close();
                GET(fd, req, server);
                event.events = EPOLLOUT;
            }
            else
            {
                req.connexion = true;
                std::string response = Created();
                if ( send(fd, response.c_str(), response.size(),0) == -1)
                    throw true;
            }
        }
    }
}



int	Response::DELETE(int &fd, Request &req, Conf &server, std::string dpath)
{
    std::string tmp_;
    std::string root =  server.locat.find(req.locationPath)->second.root;
    const char* path = root.c_str();
    const char* path2 = dpath.c_str();
    if(directoryExists(dpath.c_str()) && path2[dpath.size() - 1 ] != '/')
        throw conflict(server.confCherch("409"),req);
    char resolved_path[PATH_MAX];
    std::stringstream ss;
    std::stringstream ss1;
    ss  << realpath(path, resolved_path);
    ss1 << realpath(path2, resolved_path);
    std::string str1;
    std::string str2;
    ss >> str1 ;
    ss1 >> str2;
    if(directoryExists(str2.c_str()))
        str2 += "/";
    str1 += "/";
    if(req.get_path() == dpath)
        tmp_ = str2;
    if(str2.find(str1) != 0)
    {
        if(str2.empty())
            throw (notFound(server.confCherch("404"),req));
        else
        throw forbidden(server.confCherch("403"),req);

    }   
    if (access(str2.c_str(), W_OK) != 0)
        return -1;
    if(directoryExists(str2.c_str()))
    {
        DIR* dir = opendir(str2.c_str());
        if (dir != NULL) 
        {
            struct dirent* entry;
            while ((entry = readdir(dir)) != NULL) 
            {
                if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) 
                {
                    std::string filePath = str2 + entry->d_name;
                    if(directoryExists(filePath.c_str()))
                        filePath += "/";
                    DELETE(fd, req, server, filePath);
                }
            }
            if(req.get_path() != str2)
            {

                if(str2 != str1 && str2 != tmp_)
                    rmdir(str2.c_str());
            }
            req.connexion = true;
            closedir(dir);
        }
    }
    else if(fileExists(str2.c_str()))
    {
        return remove(str2.c_str());
    }
    else
    {
        throw (notFound(server.confCherch("404"), req));
    }
    return 0;
}
