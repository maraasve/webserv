#include "Location.hpp"

std::string Location::getPath() {
    return _path;
}

std::string Location::getRoot() {
    return _root;
}

std::string Location::getIndex() {
    return _index;
}

bool Location::getAutoIndex() {
    return _auto_index;
}

unsigned long long Location::getClientMaxBody() {
    return _client_max_body;
}

std::vector<std::string>& Location::getAllowedMethods() {
    return _allowed_methods;
}

std::unordered_map<std::string, std::string> Location::getErrorPage() {
    return _error_page;
}

std::pair<std::string, std::string> Location::getRedirection() {
    return _redirection;
}

void Location::setPath(std::string path) {
    _path = path;
}

void Location::setRoot(std::string root) {
    _root = root;
}

void Location::setIndex(std::string index) {
    _index = index;
}

void Location::setAutoIndex(bool auto_index) {
    _auto_index = auto_index;
}

void Location::setClientMaxBody(unsigned long long client_max_body) {
    _client_max_body = client_max_body;
}

void Location::setAllowedMethods(std::vector<std::string> allowed_methods) {
    _allowed_methods = allowed_methods;
}

void Location::setErrorPage(std::string error_code, std::string error_page_path) {
	_error_page.emplace(error_code, error_page_path);
}

void Location::setRedirection(std::pair<std::string, std::string> redirection) {
    _redirection = redirection;
}
