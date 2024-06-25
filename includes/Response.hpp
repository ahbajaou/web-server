#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include <fstream>
#include <sys/epoll.h>
#include <map>
#include <stdint.h>
#include "conf.hpp"
class Conf;
class Request;

class Response
{
	private:
		std::ofstream out;
		std::string chunked;
		size_t decimal;
		std::string str;
        std::string tmp;
		std::string path;
	public:
		typedef std::map<std::string , std::string>::iterator map_iterator;
		bool firstcgi;
		bool postToGet;
		bool firstExcep;
		std::string tmp_path;
	    pid_t pid;
	    // pid_t WAIT_PID;
		bool cgirespons;
		bool timeout;
		int file;
		// int status;
		FILE* output_file;
    	clock_t start;
		double end;
		std::string temp_file;
		Response();
		~Response();

		/*             METHODs              */
		void	GET(int &fd, Request &req, Conf &server);
		void	POST(int &fd, Request &req, Conf &server, epoll_event &event);
		int		DELETE(int &fd, Request &req, Conf &server, std::string dpath);
		/**************************************/

		void	clear();
		void 	serv_file(map_iterator &type, int &fd, Request &req, Conf &server);
		void 	serv_dir(int &fd, Request &req, Conf &server);
		int 	serveCgi(Request &req,int &fd);

		void	generateResponse(int &fd, Request &req, Conf &server, epoll_event &event);
		void	checkHeaders(Request &req, Conf &server);
		std::string extension(const std::string &path);
		
		
		/*   -       4XX        -      */
		
			std::string badRequest(std::string path, Request &req);//      				400
			std::string forbidden(std::string path, Request &req);//       				403
			std::string notFound(std::string path, Request &req); //      	 			404
			std::string notAllow(std::string method, std::string path, Request &req);// 405
			std::string timeOut(std::string path, Request &req);//						408
			std::string conflict(std::string path, Request &req);//						409
			std::string lengthRequired(std::string path, Request &req);// 				411	
			std::string EntityTooLarge(std::string path, Request &req); // 				413
			std::string longRequest(std::string path, Request &req);//     				414
			std::string mediaType(std::string path, Request &req);//					415
			std::string headerTooLarge(std::string path, Request &req); // 				431

		
		
		/*            5XX               */
			std::string serverError(std::string path, Request &req);// 					500
			std::string notImplement(std::string path, Request &req); //  				501
			std::string httpVersion(std::string path, Request &req);//    	 			505
			std::string CGtimeOut();//													504

			// std::string homepage(); // 200 home

			std::string Created();
			std::string noContent();
			std::string DeleteSuc();


		/***********************/
		
		void Redirect(std::string &location, Request &req, int &fd, Conf &server);
		
		std::string getResource(int &file, Request &req, Conf &server);
		std::string getRedirctionS(std::string &location);
};
std::string listDirectory(const char* path);
bool directoryExists(std::string path);
bool fileExists(std::string path);
std::string getCurrentDateTime();

std::map<std::string, std::string> mimeTypes();
std::map<std::string, std::string> post_type();


#endif
