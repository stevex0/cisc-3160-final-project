#include <cctype>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <vector>

class Token {
public:
	static const int
		IDENTIFIER = 0,
		INT_LITERAL = 1,
		OPERATOR = 2,
		PUNCTUATION = 3,
		LEFT_PARENTHESIS = 4,
		RIGHT_PARENTHESIS = 5,
		END = 6;

	const int type;
	const std::string lexeme;

	Token(int type, const std::string& lexeme): type(type), lexeme(lexeme) {}
};

class Tokenizer {
private:
	int index;
	std::string* str;
	std::vector<Token>* tokens;

	Tokenizer(std::string* str, std::vector<Token>* tokens): index(0), str(str), tokens(tokens) {}

	void error(const std::string& message) {
		std::cerr << "<Tokenizer error>: " << message << std::endl;
		exit(1);
	}

	void addToken(const Token& token) {
		this->tokens->push_back(token);
	}

	void addToken(int type, const std::string& lexeme) {
		addToken(Token(type, lexeme));
	}

	void readNext() {
		++this->index;
	}

	char charAt(int i) {
		return this->str->at(i);
	}

	char currentChar() {
		return charAt(this->index);
	}

	void addIntLiteral() {
		const int start = this->index, length = this->str->length();
		// Reads until it fails the definition of an integer literal
		for (;;) {
			if (this->index >= length || !isdigit(currentChar())) {
				break;
			}
			readNext();
		}
		std::string lexeme = this->str->substr(start, this->index - start);
		addToken(Token::INT_LITERAL, lexeme);
	}

	void addIdentifier() {
		const int start = this->index, length = this->str->length();
		// Reads until it fails the definition of an identifier
		for (;;) {
			if (this->index >= length || !(isalnum(currentChar()) || currentChar() == '_')) {
				break;
			}
			readNext();
		}
		std::string lexeme = this->str->substr(start, this->index - start);
		addToken(Token::IDENTIFIER, lexeme);
	}

	void tokenize() {
		const int str_length = this->str->length();

		for (;;) {
			if (this->index >= str_length) {
				addToken(Token::END, "$");
				return;
			}

			const char c = currentChar();

			if (c == '0') {
				if (this->index < str_length - 1 && isdigit(charAt(this->index + 1))) {
					error("Invalid integer literal.");
				}
				addToken(Token::INT_LITERAL, "0");
				readNext();
			} else if (isdigit(c)) {
				addIntLiteral();
			} else if (isalpha(c) || c == '_') {
				addIdentifier();
			} else if (c == '=') {
				addToken(Token::OPERATOR, "=");
				readNext();
			} else if (c == '+') {
				addToken(Token::OPERATOR, "+");
				readNext();
			} else if (c == '-') {
				addToken(Token::OPERATOR, "-");
				readNext();
			} else if (c == '*') {
				addToken(Token::OPERATOR, "*");
				readNext();
			} else if (c == '(') {
				addToken(Token::LEFT_PARENTHESIS, "(");
				readNext();
			} else if (c == ')') {
				addToken(Token::RIGHT_PARENTHESIS, ")");
				readNext();
			} else if (c == ';') {
				addToken(Token::PUNCTUATION, ";");
				readNext();
			} else if (isspace(c)) {
				readNext();
			} else {
				error("Unrecognized symbol.");
			}
		}
	}
public:
	static void tokenize(std::istream& input, std::vector<Token>& tokens) {
		std::string str, next;

		// Read in one line at a time and puts it all into one big string
		while (getline(input, next)) {
			str += (next + '\n');
		}

		Tokenizer tokenizer(&str, &tokens);
		tokenizer.tokenize();
	}
};

class Parser {
private:
	int index;
	std::vector<Token>* tokens;
	std::map<std::string, int>* symbol_table;

	Parser(std::vector<Token>* tokens, std::map<std::string, int>* symbol_table): index(0), tokens(tokens), symbol_table(symbol_table) {}

	void error(const std::string& message) {
		std::cerr << "<Parser error>: " << message << std::endl;
		exit(1);
	}

	Token& currentToken() {
		return this->tokens->at(this->index);
	}

	bool match(int type) {
		return currentToken().type == type;
	}

	bool match(const std::string& lexeme) {
		return currentToken().lexeme == lexeme;
	}

	void readNext() {
		++this->index;
	}

	void program() {
		if (match(Token::END)) {
			return;
		}

		assignment();
		program();
	}

	void assignment() {
		if (!match(Token::IDENTIFIER)) {
			error("Expected an identifier.");
		}

		Token identifier = currentToken();

		readNext(); // Token is an identifier

		if (!match("=")) {
			error("Expected the operator, \'=\'.");
		}

		readNext(); // Token is the assignment operator

		int value = exp();

		if (!match(";")) {
			error("Expected an \';\'.");
		}

		readNext(); // Token is a ';'

		(*this->symbol_table)[identifier.lexeme] = value;
		std::cout << identifier.lexeme << " = " << value << std::endl;
	}

	int exp() {
		int value = term();
		return expPrime(value);
	}

	int expPrime(int accumulated_value) {
		if (match("+")) {
			readNext(); // Token is a '+'
			int value = term();
			return expPrime(accumulated_value + value);
		}

		if (match("-")) {
			readNext(); // Token is '-'
			int value = term();
			return expPrime(accumulated_value - value);
		}

		return accumulated_value;
	}

	int term() {
		int value = fact();
		return termPrime(value);
	}

	int termPrime(int accumulated_value) {
		if (match("*")) {
			readNext(); // Token is a '*'
			int value = fact();
			return termPrime(accumulated_value * value);
		}
		return accumulated_value;
	}

	int fact() {
		if (match("(")) {
			readNext(); // Token is a left parenthesis

			int value = exp();

			if (!match(")")) {
				error("Mismatch parenthesis, expected \')\'");
			}

			readNext(); // Token is a right parenthesis
			return value;
		}

		if (match("+")) {
			readNext(); // Token is a '+'
			return +exp();
		}

		if (match("-")) {
			readNext(); // Token is a '-'
			return -exp();
		}

		if (match(Token::INT_LITERAL)) {
			Token int_literal = currentToken();
			readNext(); // Token is an integer
			return std::stoi(int_literal.lexeme);
		}

		if (match(Token::IDENTIFIER)) {
			Token identifier = currentToken();
			readNext(); // Token is an identifier
			if (this->symbol_table->find(identifier.lexeme) == this->symbol_table->end()) {
				error("Symbol \'" + identifier.lexeme + "\' not defined.");
			}
			return this->symbol_table->at(identifier.lexeme);
		}

		error("Syntax error.");
	}

public:
	static void program(std::vector<Token>& tokens, std::map<std::string, int>& symbol_table) {
		Parser parser(&tokens, &symbol_table);
		parser.program();
	}
};


int main(int argc, char** argv) {
	if (argc != 2) {
		std::cerr << "Proper usage: <source file>" << std::endl;
		exit(1);
	}

	std::fstream file(argv[1]);
	if (file.fail()) {
		std::cerr << "Bad source file. Unable to read from: " << argv[1] << std::endl;
		exit(1);
	}

	clock_t start, end;
	std::vector<Token> tokens;
	std::map<std::string, int> symbol_table;

	start = clock();

	Tokenizer::tokenize(file, tokens);
	Parser::program(tokens, symbol_table);

	end = clock();

	file.close();

	std::cout << "Successfully Executed: Took " << end - start << " ms." << std::endl;

	return 0;
}