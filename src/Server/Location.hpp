#ifndef LOCATION_HPP
#define LOCATION_HPP

#include <string>
#include <vector>
#include <stdbool.h>
#include <unordered_map>

class Location {
private:
	std::string _path;
	std::string _root;
	std::string _index;
	bool _auto_index = false;
	unsigned long long _client_max_body = 0;
	std::vector<std::string> _allowed_methods;
	std::unordered_map<std::string, std::string> _error_page;
	std::pair<std::string, std::string> _redirection;

public:
	std::string getPath();
	std::string getRoot();
	std::string getIndex();
	bool getAutoIndex();
	unsigned long long getClientMaxBody();
	std::vector<std::string> getAllowedMethods();
	std::unordered_map<std::string, std::string> getErrorPage();
	std::pair<std::string, std::string> getRedirection();

	void setPath(std::string path);
	void setRoot(std::string root);
	void setIndex(std::string index);
	void setAutoIndex(bool auto_index);
	void setClientMaxBody(unsigned long long client_max_body);
	void setAllowedMethods(std::vector<std::string> allowed_methods);
	void setErrorPage(std::string error_code, std::string path);
	void setRedirection(std::pair<std::string, std::string> redirection);
};

#endif