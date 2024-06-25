/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ahbajaou <ahbajaou@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/17 11:26:36 by arahmoun          #+#    #+#             */
/*   Updated: 2024/03/28 01:11:09 by ahbajaou         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Request.hpp"

Request::Request()
{
}
void Request::pars(std::stringstream &buf, size_t &endOf)
{
	std::string key;
	std::string dst;
	std::string value;

	std::getline(buf, dst);
	size_t i = dst.size() + 1;

	if(std::count(dst.begin(), dst.end(), ' ') == 2)
	{
		startLineForma = true;
	}
	startline << dst;
	startline >> method;
	startline >> path;
	startline >> protocol;
	
	while(i < endOf && buf >> key && std::getline(buf, value))
	{
		headers[key] = value;
		i += key.length() + value.length() + 1;
	}

	if(buf && (endOf + 4) < buf.str().size())
	{
		buf >> key;
		body << key << buf.rdbuf();
	}

	chun = 0;
	ra = 0;
	firstTime = true;
	replacePercent20(path);
	removeDuplicateSlashes(path);
	std::string tmp(path);
	tmp.rfind('?') != std::string::npos ? path = tmp.substr(0, tmp.rfind('?')) : path = tmp;
	tmp.rfind('?') != std::string::npos ? query = tmp.substr(tmp.rfind('?') + 1) : query = "";
}

size_t findEndOfHeaders(char* buffer, ssize_t bufferSize)
{
	const char *end = "\r\n\r\n";
	size_t size = strlen(end);
	for (ssize_t i = 0; i < bufferSize; i++)
	{
		if (strncmp(buffer + i, end, size) == 0)
		{
			return i;
		}
	}
	return -1;
}

Request::Request(const Request &other)
{
	*this = other;
}


Request &Request::operator=(const Request &other)
{
	if(this != &other)
	{
		startline << other.startline.str();
		method = other.method;
		path = other.path;
		protocol = other.protocol;
		firstTime = other.firstTime;
		headers = other.headers;
		body << other.body.str();
	}
	return *this;
}

std::ostream &operator<<(std::ostream &os, Request &other)
{
	os << other.get_method() << " ";
	os << other.get_path() << " ";
	os << other.get_protocol() << "\n";
	std::map<std::string, std::string>::const_iterator it = other.get_headers().begin();
	std::map<std::string, std::string>::const_iterator ite = other.get_headers().end();
	for (; it != ite; ++it)
		os << it->first <<BLUE<< "|" <<YOLLOW << it->second << '\n';
	os << "body :\n" << other.get_body() << RED <<"." << DEF << '\n';
	return os;
}

void Request::clear()
{
    startline.str("");
	startline.clear();
	method.clear();
	path.clear();
	protocol.clear();
	headers.clear();
	body.str("");
	body.clear();
	query.clear();
	root_end = 0;
	startLineForma = false;
	body_limit = 0;
	firstTime = false;
	connexion = false;
	CGI = false;
	ra = 0;
	chun = 0;
	red_path.clear();
	locationPath.clear();
}

const std::map<std::string, std::string> &Request::get_headers() const
{
	return headers;
}

const std::string Request::get_header(const char *key)
{
	if(headers.find(key) != headers.end())
	{
		std::stringstream ss(headers.find(key)->second);
		std::string ret;
		ss >> ret;
		return ret;
	}
    return ("");
}



const std::string Request::get_body() const
{
	return body.str();
}


std::string &Request::get_path()
{
	return path;
}

std::string &Request::get_query()
{
	return query;
}

const std::string Request::get_method() const
{
	return method;
}

const std::string Request::get_protocol() const
{
	return protocol;
}

Request::~Request()
{
}

void replacePercent20(std::string& str) {
    std::string::size_type pos = 0;
    const std::string pattern = "%20";
    const std::string space = " ";

    while ((pos = str.find(pattern, pos)) != std::string::npos) {
        str.replace(pos, pattern.length(), space);
        pos += space.length();
    }
}

void removeDuplicateSlashes(std::string& str) {
    std::string::iterator it = str.begin();
    char prevChar = '\0';
    
    while (it != str.end()) {
        if (*it == '/' && prevChar == '/') {
            it = str.erase(it);
        } else {
            prevChar = *it;
            ++it;
        }
    }
}
