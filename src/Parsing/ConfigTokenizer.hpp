#ifndef CONFIGTOKENIZER_HPP
#define CONFIGTOKENIZER_HPP

#include <iostream>
#include <vector>

enum TokenType {
	KEYWORD,
	BRACE_OPEN,
	BRACE_CLOSE,
	COLON,
	SEMI_COLON,
	COMMENT
};

struct Token {
	enum TokenType token_type;
	std::string value;
};

class ConfigTokenizer {
private:
	std::vector<Token> tokens;
	size_t index;

public:
	ConfigTokenizer(std::string& file);

	bool isSymbol(char c);
	void addToken(TokenType type, std::string value);
	void handleSymbols(char c);
	void handleKeys(std::string line);
	const std::vector<Token>& getTokens() const;
};

#endif