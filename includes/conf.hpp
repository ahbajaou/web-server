

#ifndef CONF_HPP
#define CONF_HPP

#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>
#include <sstream>
#include <map>
#include <filesystem>
#include <vector>




bool directoryExists(std::string path);
bool fileExists(std::string path);

class loca{
	public :
		bool get;
		bool post;
		bool delet;
		bool cgi;
		bool  autoindex;
		std::string root;
		std::string location;
		std::string defau;
		std::string upload;
		std::string redirect;
};

// class Response;
class Conf {
	private :
		std::string name;
		std::map<std::string, loca>::iterator it;
		std::map<std::string,std::string> loc1;
		std::map<std::string,std::string>::iterator ito;
		int serv_n ;
		int serv_v ;
	public :
		typedef std::map<std::string , std::string>::iterator map_iterator;
		std::map<std::string, std::string> map;
		std::map<std::string, loca> locat;
		int numOfserver;
		Conf(){
			numOfserver = 0;
		}
		Conf(std::ifstream & fg);
		void parseFrom(std::string name);
		std::string isspaceRemove(std::string tmp);
		void    parseAndCheckLocation();
		void defaultConfic();
		void    parsAndCheckServer();
		std::string confCherch(std::string name);
		void parsLocation(std::ifstream & fg);
		void displayLocation();
		std::map<std::string, loca> getLocal();
};

#endif

