#include "Request.hpp"

int main(void)
{
	std::string httpRequest =
    "POST /submit-form HTTP/1.1\r\n"
    "Host: www.example.com\r\n "
    "User-Agent: curl/7.68.0\r\n"
    "Content-Type: application/x-www-form-urlencoded\r\n"
    "Content-Length: 27\r\n"
    "Connection: keep-alive\r\n"
    "\r\n"
    "name=John+Doe&age=30&city=NY";

	Request request(httpRequest);
	request.parseRequest(httpRequest);
    std::cout << request.getErrorCode() << std::endl;
}