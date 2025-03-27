#include "./ConfigTokenizer.hpp"

ConfigTokenizer::ConfigTokenizer(std::string& file): index(0) {
	while (index < file.length()) {
		char c = file[index];
		if (std::isalnum(c) || c == '/') {
				handleKeys(&file[index]);
		} else if (isSymbol(c)) {
			handleSymbols(c);
		} else if (!std::isspace(c)){
			throw std::runtime_error("Unexpected character: " + file[index]);
		}
		index++;
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
}

void ConfigTokenizer::handleKeys(std::string line) {
	std::string key = "";
	while(index < line.length() && !std::isspace(line[index]) && isSymbol(line[index])) {
		key.push_back(line[index]);
		index++;
	}
	addToken(KEYWORD, key);
}

bool ConfigTokenizer::isSymbol(char c) {
	return (c == '{' | c == '}' | c == ';' | c == ':');
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
