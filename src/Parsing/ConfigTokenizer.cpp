#include "./ConfigTokenizer.hpp"

ConfigTokenizer::ConfigTokenizer(std::string file): index(0) {
	while (index < file.length()) {
		while (index < file.length() && std::isspace(file[index])) {
			index++;
		}
		char c = file[index];
		if (std::isalnum(c) || c == '/') {
			handleKeys(file);
		} else if (isSymbol(c)) {
			handleSymbols(c);
		} else if (c == '#') {
			skipComments(file);
		} else if (!std::isspace(c)){
				throw std::runtime_error("Unexpected character: " + std::string(1, file[index]));
		}
	}
}

void ConfigTokenizer::handleSymbols(char c) {
	switch(c) {
		case '{':
				addToken(BRACE_OPEN, "{");
				break;
		case '}':
				addToken(BRACE_CLOSE, "}");
				break;
		case ';':
				addToken(SEMI_COLON, ";");
				break;
		case ':':
				addToken(COLON, ":");
				break;
	}
	index++;
}

void ConfigTokenizer::skipComments(std::string line) {
	while(index < line.length() && line[index] != '\n') {
		index++;
	}
}

void ConfigTokenizer::handleKeys(std::string line) {
	std::string key = "";
	while(index < line.length() && !std::isspace(line[index]) && !isSymbol(line[index])) {
		key.push_back(line[index]);
		index++;
		//validate a path happens here
	}
	addToken(KEYWORD, key);
}

bool ConfigTokenizer::isSymbol(char c) {
	return ((c == '{') || (c == '}') || (c == ';') || (c == ':'));
}

void ConfigTokenizer::addToken(TokenType type, std::string value) {
	Token token;
	token.token_type = type;
	token.value = value;
	this->tokens.push_back(token);
}

const std::vector<Token>& ConfigTokenizer::getTokens() const {
	return tokens;
}
