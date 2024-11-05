#include <iostream>
#include <string>
#include <vector>
#include <cctype>
#include <unordered_map>

enum class TokenType {
    SELECT,
    FROM,
    IDENTIFIER,
    COMMA,
    ASTERISK,
    SEMICOLON,
    END_OF_FILE,
    UNKNOWN
};

struct Token {
    TokenType type;
    std::string value;
};

class Lexer {
public:
    explicit Lexer(const std::string &input) : input(input), position(0) {}

    Token getNextToken() {
        while (position < input.size()) {
            char currentChar = input[position];
            if (std::isspace(currentChar)) {
                position++;
                continue;
            }
            if (std::isalpha(currentChar)) {
                std::string word = readWord();
                if (keywords.count(word)) {
                    return Token{keywords[word], word};
                }
                return Token{TokenType::IDENTIFIER, word};
            }
            if (currentChar == '*') {
                position++;
                return Token{TokenType::ASTERISK, "*"};
            }
            if (currentChar == ',') {
                position++;
                return Token{TokenType::COMMA, ","};
            }
            if (currentChar == ';') {
                position++;
                return Token{TokenType::SEMICOLON, ";"};
            }
            position++;
            return Token{TokenType::UNKNOWN, std::string(1, currentChar)};
        }
        return Token{TokenType::END_OF_FILE, ""};
    }

private:
    std::string input;
    size_t position;
    std::unordered_map<std::string, TokenType> keywords = {
        {"SELECT", TokenType::SELECT},
        {"FROM", TokenType::FROM}
    };

    std::string readWord() {
        size_t start = position;
        while (position < input.size() && std::isalnum(input[position])) {
            position++;
        }
        return input.substr(start, position - start);
    }
};

class Parser {
public:
    explicit Parser(Lexer &lexer) : lexer(lexer), currentToken(lexer.getNextToken()) {}

    void parse() {
        if (currentToken.type == TokenType::SELECT) {
            parseSelectStatement();
        } else {
            std::cerr << "Error: Expected SELECT statement\n";
        }
    }

private:
    Lexer &lexer;
    Token currentToken;

    void parseSelectStatement() {
        match(TokenType::SELECT);
        parseColumnList();
        match(TokenType::FROM);
        match(TokenType::IDENTIFIER);
        match(TokenType::SEMICOLON);
        std::cout << "Parsed SELECT statement successfully\n";
    }

    void parseColumnList() {
        if (currentToken.type == TokenType::ASTERISK) {
            match(TokenType::ASTERISK);
        } else {
            match(TokenType::IDENTIFIER);
            while (currentToken.type == TokenType::COMMA) {
                match(TokenType::COMMA);
                match(TokenType::IDENTIFIER);
            }
        }
    }

    void match(TokenType expectedType) {
        if (currentToken.type == expectedType) {
            currentToken = lexer.getNextToken();
        } else {
            std::cerr << "Error: Expected " << static_cast<int>(expectedType)
                      << ", but found " << static_cast<int>(currentToken.type) << "\n";
        }
    }
};

int main() {
    std::string sqlQuery = "SELECT name, age FROM users;";
    Lexer lexer(sqlQuery);
    Parser parser(lexer);

    parser.parse();
    return 0;
}

