//We should change the templates so they do not take end!!!!
#pragma once

template<typename T>
std::function<void(T&, typename ConfigParser::TokenIt&, typename ConfigParser::TokenIt&)>
ConfigParser::wrapParser(
    void (ConfigParser::*fn)(
        T&,
        typename ConfigParser::TokenIt&,
        typename ConfigParser::TokenIt&))
{
    return [this, fn](T& target,
                      typename ConfigParser::TokenIt& it,
                      typename ConfigParser::TokenIt& end)
    {
        (this->*fn)(target, it, end);
    };
}

template<typename T>
void ConfigParser::parseReturn(T &t, typename ConfigParser::TokenIt& it, typename ConfigParser::TokenIt& end) {
	std::string error_code = it->value;
	if (error_code != "301" && error_code != "302") {
			error("Redirection directive: Invalid status code \"" + error_code + "\"");
		}
	++it;
	expectTokenType(KEYWORD, it, end);
	std::string uri_redirection = it->value;
	if (it->value[0] == '/') {
		t.setRedirection(std::make_pair(error_code, uri_redirection));
		++it;
		return;
	} else if (it->value == "http" || it->value == "https") {
		++it;
		expectTokenType(COLON, it, end);
		uri_redirection.append(it->value);
		++it;
		expectTokenType(KEYWORD, it, end);
		uri_redirection.append(it->value);
		t.setRedirection(std::make_pair(error_code, uri_redirection));
		++it;
		return ;
	}
	error("Redirection directive: Wrong format");
}

template<typename T>
void ConfigParser::parseAllowedMethods(T &t, typename ConfigParser::TokenIt& it, typename ConfigParser::TokenIt& end) {
	std::set<std::string> unique_methods;
	(void)end;
	for (int i = 0; i < 3; i++) {
		std::string method = it->value;
		if (method != "GET" && method != "POST" && method != "DELETE") {
			error("Invalid allowed_method method: " + method);
		}
		if (!unique_methods.insert(method).second) {
			error("Allowed Methods Directive: Invalid duplicated method \"" + method + "\"");
		}
		t.getAllowedMethods().push_back(method);
		if (it->token_type != KEYWORD) {
			break;
		}
		++it;
	}
}

template<typename T>
void ConfigParser::parseListen(T &t, typename ConfigParser::TokenIt& it, typename ConfigParser::TokenIt& end) {
	std::string port = it->value;
	(void)end;
	if (!isValidPort(port)) {
		error("Listen directive: invalid port number \"" + port + "\"");
	}
	t.setPort(std::stoi(port));
	++it;
}

template<typename T>
void ConfigParser::parseHost(T &t, typename ConfigParser::TokenIt& it, typename ConfigParser::TokenIt& end) {
	std::string host = it->value;
	(void)end;
	t.setHost_u_long(convertHost(host));
	t.setHost_string(host);
	++it;
}

template<typename T>
void ConfigParser::parseRoot(T &t, typename ConfigParser::TokenIt& it, typename ConfigParser::TokenIt& end) {
	std::string root = it->value;
	(void)end;
	if (!isValidPath(root)) {
		error("Root directive: invalid root path \"" + root + "\"");
	}
	t.setRoot(root);
	required_directives.has_root = true;
	++it;
}

template<typename T>
void ConfigParser::parseIndex(T &t, typename ConfigParser::TokenIt& it, typename ConfigParser::TokenIt& end) {
	std::string index = it->value;
	(void)end;
	if (!isValidIndex(index)) {
		error("Index directive: invalid file name \"" + index + "\"");
	}
	t.setIndex(index);
	required_directives.has_index = true; //maybe there is a better way to do this?
	++it;
}

template<typename T>
void ConfigParser::parseServerNames(T &t, typename ConfigParser::TokenIt& it, typename ConfigParser::TokenIt& end) {
	std::vector<std::string> server_names;
	while (it != end) {
		std::string name = it->value;
		if (!isValidServerName(name)) {
			error("Server Names directive: invalid server name \"" 
				+ name + "\"");
		}
		server_names.push_back(name);
		++it;
		if (it->token_type == SEMI_COLON) {
			t.setServerNames(server_names);
			return;
		}
	}
	error("Server Names directive: missing semi-colon");
	
}

template<typename T>
void ConfigParser::parseAutoIndex(T &t, typename ConfigParser::TokenIt& it, typename ConfigParser::TokenIt& end) {
	std::string value = it->value;
	(void)end;
	if (value != "on" && value != "off") {
		error("Auto Index directive: Invalid value \"" + value + "\"");
	}
	t.setAutoIndex(value == "on");
	++it;
}

template<typename T>
void ConfigParser::parseClientMaxBody(T &t, typename ConfigParser::TokenIt& it, typename ConfigParser::TokenIt& end) {
	auto bytes = isValidClientBodySize(it->value);
	(void)end;
	if (!bytes) {
		error("Client Max Body directive: Invalid value \"" + it->value + "\"");
	}
	t.setClientMaxBody(bytes);
	++it;
}

template<typename T>
void ConfigParser::parseErrorPage(T &t, typename ConfigParser::TokenIt& it, typename ConfigParser::TokenIt& end) {
	std::string error_code = it->value;
	if (!isValidErrorCode(error_code)){
		error("Error Page: Invalid code \"" + error_code + "\"");
	}
	++it;
	expectTokenType(KEYWORD, it, end);
	std::string error_page_path = it->value;
	if (!isValidPath(error_page_path)) {
		error("Error Page: Invalid error page path \"" + error_page_path + "\"");
	}
	t.setErrorPage(error_code, error_page_path);
	++it;
}