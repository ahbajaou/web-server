
#include "../includes/conf.hpp"


Conf::Conf(std::ifstream & fg)
{
    try{
		serv_n = 0;
		serv_v = 0;
		numOfserver = 0;
		getline(fg,name);
		size_t tab = static_cast<size_t>(std::count(name.begin(),name.end(),'\t'));
		size_t spac = static_cast<size_t>(std::count(name.begin(),name.end(),' '));
		if ((name.empty()) || (spac == name.size())
			|| (tab == name.size()) || (spac + tab == name.size()))
			return;
		if (name.find("server") != std::string::npos)
		{
			numOfserver++;
			// number++;
			getline(fg,name);

			map_iterator tmp_tmp;
			if (name.find("{") != std::string::npos)
			{	
				serv_n++;
				while (getline(fg,name))
				{

					size_t tab = static_cast<size_t>(std::count(name.begin(),name.end(),'\t'));
					size_t spac = static_cast<size_t>(std::count(name.begin(),name.end(),' '));
					if ((name.empty()) || (spac == name.size())
						|| (tab == name.size()) || (spac + tab == name.size()))
						continue;
					if (name.find("=") != std::string::npos)
					{
						parseFrom(name);
						parsLocation(fg);
						if (name.find("]") != std::string::npos){
							displayLocation();
							continue;
						}							
						if (isspaceRemove(name.substr(0,name.find("="))) == "error_page"){
							std::stringstream ss(name.substr(name.find("=") + 1));
							std::string key;
							std::string value;
							std::string err;
							ss >> key;
							ss >> value;
							if (ss >> err || !fileExists(value.c_str()))
									throw "ERORR: Erorr Configfile";
							map[key] = value;
							}
						else
						{
							name.erase(std::remove_if(name.begin(),name.end(),isspace),name.end());
							if (map.count(name.substr(0,name.find("="))) != 0)
								throw "ERROR DUPLICATE!";
							map[name.substr(0,name.find("="))] = name.substr(name.find("=") + 1);
					
						}
						if (name.find("}") != std::string::npos){
							serv_v++;
							break; 
						}
						if (name.find("server") != std::string::npos && name.size() == 6)
						{
							throw "ERORR: Erorr Configfile";
						}
					}
					else if (name.find("}") != std::string::npos)
					{
							serv_v++;
						parsAndCheckServer();
						if (loc1.empty())
							throw "LOCATION NOT FOUND!";
						break;
					}
					else
					{
						if (!name.empty())
							throw "ERORR: Erorr Configfile";
						
					}
				}
				if (serv_n != serv_v){
						throw "ERORR: Erorr Configfile";
				}

			}
			else
			{
				throw "ERORR: Erorr Configfile";
			}
		}
		else
		{
			throw "ERORR: Erorr Configfile";
		}

    }
    catch (const char * err){
        std::cout << err << std::endl;
        exit(0);
    }

}

void Conf::parseFrom(std::string name)
{
    try
    {
        std::string form;
        std::vector<std::string> str;
        std::stringstream ss(name);
        while (ss >> form){
            str.push_back(form);
        }
        if (str[0] == "]")
		{
			if (str[0][1])
            	throw "FORM NOT VALID!";
            return ;
		}
        if (str[1] != "=")
            throw "FORM NOT VALID!";
    }
    catch(const char *e)
    {
        std::cerr << e  << '\n';
        exit(0);
    }
}

std::map<std::string, loca> Conf::getLocal(){
    return locat;
}

void Conf::displayLocation()
{
			std::map<std::string,std::string>::iterator it = loc1.begin();
			loca loc = loca();
			try{
				for(;it != loc1.end(); it++){
					if (it->first.find("location") != std::string::npos){
						loc.location = it->second;
						if (locat.count(it->second))
							throw "Error duplicate";
					}
		
					else if (it->first == "method"){
							loc.get = false;
							loc.post = false;
							loc.delet = false;
							std::string met;
							std::stringstream spl(it->second);
							while (spl >> met){
								if (met == "POST")
									loc.post = true;
								else if (met == "GET")
									loc.get = true;
								else if (met == "DELETE")
									loc.delet = true;
								else
									throw "METHODE NOT FOUND!";
							}
					}
					else if (it->first.find("root") != std::string::npos){
						loc.root = isspaceRemove(it->second);
					}
					else if (it->first == "autoindex"){
						if (isspaceRemove(it->second).empty())
							throw "AUTOINDEX NOT FOUND!";
						if (isspaceRemove(it->second) == "on")
							loc.autoindex = true;
						else if (isspaceRemove(it->second) == "off")
							loc.autoindex = false;
						else
							throw "AUTOINDEX NOT FOUND!";
					}
					else if (it->first == "cgi"){
						if (isspaceRemove(it->second).empty())
							throw "CGI NOT FOUND!";
						if (isspaceRemove(it->second) == "on")
							loc.cgi = true;
						else if (isspaceRemove(it->second) == "off")
							loc.cgi = false;
						else
							throw "CGI NOT FOUND!";
					}
					else if (it->first.find("default") != std::string::npos){
						if (isspaceRemove(it->second).empty())
							throw "DEFAULT NOT FOUND!";
						loc.defau = isspaceRemove(it->second);

					}
					else if (it->first.find("upload") != std::string::npos){
						if (isspaceRemove(it->second).empty())
							throw "UPLOAD NOT FOUND!";
						loc.upload = isspaceRemove(it->second);
					}
					else if (it->first.find("redirect") != std::string::npos){
						if (isspaceRemove(it->second).empty())
							throw "REDIRECT NOT FOUND!";
						loc.redirect = isspaceRemove(it->second);
					}
				}
				locat[loc.location] = loc;
			}
			catch(const char *err){
				std::cout << err << std::endl;
				exit(0);
			}
}

std::string Conf::isspaceRemove(std::string tmp)
{
    std::string separate = tmp;
    separate.erase(std::remove_if(separate.begin(),separate.end(),isspace),separate.end());
    return  separate;
}

void    Conf::parseAndCheckLocation()
{		
        std::map<std::string,std::string>::iterator lc;
        try{
            if (loc1.find("method") == loc1.end() || (map.find("method") != loc1.end() && isspaceRemove(loc1.find("method")->second).empty())){
                throw "METHOD NOT FOUND!";
            }
            if ((lc = loc1.find("root")) == loc1.end() || (map.find("root") != loc1.end() && isspaceRemove(loc1.find("root")->second).empty())){
                throw "ROOT NOT FOUND!";
            }
            else{
                    if (directoryExists(isspaceRemove(lc->second)) != true){
                        throw "ROOT NOT FOUND!";
                }
            }
            if ((lc = loc1.find("upload")) != loc1.end()){
                    if (directoryExists(isspaceRemove(lc->second)) != true || (loc1.find("upload") != loc1.end() && isspaceRemove(loc1.find("upload")->second).empty())){
                        throw "UPLOAD NOT FOUND!";
                    }
            }
			if ((lc = loc1.find("default")) != loc1.end()){
				if (!fileExists(isspaceRemove(loc1.find("root")->second + "/" + lc->second)) || (loc1.find("default") != loc1.end() && isspaceRemove(loc1.find("default")->second).empty())){
					throw "DEFAULT NOT FOUND!";
				}
            }
        lc = loc1.begin();
        for(; lc != loc1.end(); lc++){
            if (lc->first.compare("cgi") != 0 && lc->first.compare("autoindex") != 0 && lc->first.compare("default") != 0\
            && lc->first.compare("location") != 0 && lc->first.compare("root") != 0 && lc->first.compare("method") != 0\
            && lc->first.compare("redirect") != 0 && lc->first.compare("upload") != 0)
            {
                throw "SYNTAX ERROR!";
            }
        }
        }
        catch(const char * err){
            std::cout << err << std::endl;
            exit(0);
        }
}

void Conf::defaultConfic()
{
    map["port"] = "8080";
    map["host"] = "127.0.0.1";
    map["server_name"] = "weldjed";
    map["body_size_limit"] = "10000000";
    loc1["location"] = "/";
    loc1["method"] = "GET POST";
    loc1["root"] = "www/server1";
    loc1["autoindex"] = "on";
    loc1["cgi"] = "off";

}

void    Conf::parsAndCheckServer()
{
			std::map<std::string, std::string>::iterator mp;
			try{
				if ((mp = map.find("port")) == map.end() || (map.find("port") != map.end() && map.find("port")->second.empty())){
						throw "PORT NOT FOUND!";
				}
				else{
					if ((atof(mp->second.c_str()) < 0) || (atof(mp->second.c_str()) > 65535))
						throw "PORT OUT OF RANGE!";
				}
				if (map.find("host") == map.end() || (map.find("host") != map.end() && map.find("host")->second.empty())){
						throw "HOST NOT FOUND!";
				}
				if (map.find("body_size_limit") == map.end()|| (map.find("body_size_limit") != map.end() && map.find("body_size_limit")->second.empty())){
						throw "BODY SIZE LIMIT NOT FOUND!";
				}
				if (map.find("root") != map.end()){
						throw "YOU SHOULD GIVES ROOT IN LOCATION!";
				}
				if (map.count(name.substr(0,name.find("="))))
					throw "Error duplicate";

	}
	catch (const char * err){
		std::cout << err << std::endl;
		exit(0);
	}
}

std::string Conf::confCherch(std::string name)
{
    if ((ito = map.find(name)) != map.end()){
        return ito->second;
    }
    return "not found";
}


void Conf::parsLocation(std::ifstream & fg)
{
		int roott = 0;
		int cggi = 0;
		int autoi = 0;
		int uplo = 0;
		int def = 0;
		int meth = 0;
		
			(void)fg;
			if (isspaceRemove(name.substr(0,name.find("="))) == "port" || isspaceRemove(name.substr(0,name.find("="))) == "host"
				|| isspaceRemove(name.substr(0,name.find("="))) == "server_name" || isspaceRemove(name.substr(0,name.find("="))) == "body_size_limit"
				|| isspaceRemove(name.substr(0,name.find("="))) == "error_page")
				return ;
			if (isspaceRemove(name.substr(0,name.find("="))) == "location")
				{
					loc1.clear();
					if (name.find("=") != std::string::npos)
					{
						name.erase(std::remove_if(name.begin(),name.end(),isspace),name.end());
						loc1[name.substr(0,name.find("="))] = (name.substr(name.find("=") + 1));
						getline(fg,name);
						if (name.find("[") != std::string::npos)
						{
							std::string form;
							std::vector<std::string> str;
							std::stringstream ss(name);
							ss >> form;
							if (form != "[")
								throw "FORM NOT VALID!";
							while (getline(fg,name)){
								size_t tab = static_cast<size_t>(std::count(name.begin(),name.end(),'\t'));
								size_t spac = static_cast<size_t>(std::count(name.begin(),name.end(),' '));
								if ((name.empty()) || (spac == name.size())
									|| (tab == name.size()) || (spac + tab == name.size()))
								{
									continue;
								}
								parseFrom(name);
								if (name.find("=") != std::string::npos){
									if (name.find("root") != std::string::npos)
									{
										roott++;
										if (roott == 2)
											throw "YOU SHOULD ENTER ONE ROOT!";
									}
									if (name.find("cgi") != std::string::npos)
									{
										cggi++;
										if (cggi == 2)
											throw "YOU SHOULD ENTER ONE cgi!";
									}
									if (name.find("method") != std::string::npos)
									{
										meth++;
										if (meth == 2)
											throw "YOU SHOULD ENTER ONE method!";
									}
									if (name.find("autoindex") != std::string::npos)
									{
										autoi++;
										if (autoi == 2)
											throw "YOU SHOULD ENTER ONE autoindex!";
									}
									if (name.find("upload") != std::string::npos)
									{
										uplo++;
										if (uplo == 2)
											throw "YOU SHOULD ENTER ONE upload!";
									}
									if (name.find("default") != std::string::npos)
									{
										def++;
										if (def == 2)
											throw "YOU SHOULD ENTER ONE default!";
									}
									loc1[isspaceRemove(name.substr(0,name.find("=")))] = (name.substr(name.find("=") + 1));
									if (name.find("]") != std::string::npos){
										break;
									}
									if (name.find("location") != std::string::npos){
											throw "ERORR: Erorr Configfile";
									}
								}
								else if (name.find("]") != std::string::npos){
									parseAndCheckLocation();
									break;
								}
								else{
									throw "ERORR: Erorr Configfile";
								}

							}
						}
						else
							throw "ERORR: Erorr Configfile";
					}
					else
						throw "ERORR: Erorr Configfile";
				}
				else
				{
					if (name.find("#") != std::string::npos)
						throw "CONFIG FILE NOT SEPORT COMMENT!";
					else
						throw "LOCATION NOT FOUND!";
				}

}
