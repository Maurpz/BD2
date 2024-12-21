#include <iostream>
#include <vector>
#include <string>
#include <regex>

enum TokenType {
    KEYWORD,     // SELECT, FROM, CREATE, DELETE, UPDATE, INSERT, etc.
    IDENTIFIER,  // Nombres de tablas o columnas
    NUMBER,      // Números (enteros, flotantes)
    SYMBOL,      // Paréntesis, comas, punto y coma, etc.
    OPERATOR,    // Operadores como '=', '<', '>'
    STRING,      // Cadenas de texto entre comillas
    UNKNOWN      // Cualquier otro token no identificado
};

struct Token {
    TokenType type;
    std::string value;
};

class Lexer {
public:
    Lexer(const std::string& input) : input_(input), pos_(0) {}

    std::vector<Token> tokenize() {
	std::vector<Token> tokens;

	while (pos_ < input_.length()) {
	    skipWhitespace();

	    if (std::isalpha(peek())) {
		// Leer palabras clave o identificadores
		tokens.push_back(readKeywordOrIdentifier());
	    } else if (std::isdigit(peek())) {
		// Leer números
		tokens.push_back(readNumber());
	    } else if (peek() == '\'' || peek() == '\"') {
		// Leer cadenas
		tokens.push_back(readString());
	    } else if (isOperator(peek())) {
		// Leer operadores
		tokens.push_back(readOperator());
	    } else if (std::ispunct(peek())) {
		// Leer otros símbolos
		tokens.push_back(readSymbol());
	    } else {
		// Leer caracteres desconocidos
		tokens.push_back({UNKNOWN, std::string(1, nextChar())});
	    }
	}

	return tokens;
    }


private:
    std::string input_;
    size_t pos_;

    char peek() const {
        return pos_ < input_.length() ? input_[pos_] : '\0';
    }

    char nextChar() {
        return input_[pos_++];
    }

    void skipWhitespace() {
        while (std::isspace(peek())) {
            nextChar();
        }
    }
    bool isOperator(char c) {
        return c == '=' || c == '<' || c == '>' || c == '!';
    }

    Token readOperator() {
	char first = nextChar();

	// Verificar operadores de dos caracteres como `<=`, `>=`, `!=`
	if ((first == '<' || first == '>' || first == '!') && peek() == '=') {
	    char second = nextChar();
	    return {OPERATOR, std::string(1, first) + second};
	}

	// Operadores de un solo carácter como `=`, `<`, `>`
	return {OPERATOR, std::string(1, first)};
    }


    Token readKeywordOrIdentifier() {
        std::string word;
        while (std::isalnum(peek()) || peek() == '_') {
            word += nextChar();
        }

        static const std::vector<std::string> keywords = {
            "SELECT", "FROM", "WHERE", "CREATE", "TABLE", "DELETE", "INSERT", "UPDATE", "VALUES", "SET", "AND", "OR" , "INTO"
        };

        if (std::find(keywords.begin(), keywords.end(), word) != keywords.end()) {
            return {KEYWORD, word};
        }

        return {IDENTIFIER, word};
    }

    Token readNumber() {
        std::string number;
        while (std::isdigit(peek())) {
            number += nextChar();
        }

        if (peek() == '.') {
            number += nextChar();
            while (std::isdigit(peek())) {
                number += nextChar();
            }
        }

        return {NUMBER, number};
    }

    Token readString() {
        char quote = nextChar(); // Read opening quote
        std::string str;

        while (peek() != quote && peek() != '\0') {
            str += nextChar();
        }

        if (peek() == quote) {
            nextChar(); // Consume closing quote
        }

        return {STRING, str};
    }

    Token readSymbol() {
        return {SYMBOL, std::string(1, nextChar())};
    }
};







class Parser {
public:
    Parser(const std::vector<Token>& tokens) : tokens_(tokens), pos_(0) {}

    void parse() {
        while (pos_ < tokens_.size()) {
            if (match("SELECT")) {
                parseSelect();
            } else if (match("CREATE")) {
                parseCreate();
            } else if (match("DELETE")) {
                parseDelete();
            } else if (match("INSERT")) {
	        parseInsert();
	    } else {
                throw std::runtime_error("Error de sintaxis: Consulta desconocida.");
            }
        }
    }

private:
    const std::vector<Token>& tokens_;
    size_t pos_;

    const Token& currentToken() const {
        return tokens_[pos_];
    }

    const Token& nextToken() {
        return tokens_[pos_++];
    }

    bool match(const std::string& keyword) {
        if (pos_ < tokens_.size() && tokens_[pos_].type == KEYWORD && tokens_[pos_].value == keyword) {
            pos_++;
            return true;
        }
        return false;
    }

    void expect(TokenType type, const std::string& errorMessage) {
        if (pos_ >= tokens_.size() || tokens_[pos_].type != type) {
            throw std::runtime_error(errorMessage);
        }
        pos_++;
    }

    void parseSelect() {
	// Verificar columnas: * o una lista de columnas separadas por comas
	if (pos_ < tokens_.size() && tokens_[pos_].type == SYMBOL && tokens_[pos_].value == "*") {
	    pos_++; // Consumir *
	} else {
	    // Lista de columnas
	    if (pos_ >= tokens_.size() || tokens_[pos_].type != IDENTIFIER) {
		throw std::runtime_error("Error: Se esperaba una columna o '*' después de SELECT.");
	    }
	    pos_++; // Consumir la primera columna

	    // Verificar columnas adicionales separadas por comas
	    while (pos_ < tokens_.size() && tokens_[pos_].type == SYMBOL && tokens_[pos_].value == ",") {
		pos_++; // Consumir la coma
		if (pos_ >= tokens_.size() || tokens_[pos_].type != IDENTIFIER) {
		    throw std::runtime_error("Error: Se esperaba una columna después de ','.");
		}
		pos_++; // Consumir la siguiente columna
	    }
	}

	// Verificar FROM
	if (pos_ >= tokens_.size() || tokens_[pos_].type != KEYWORD || tokens_[pos_].value != "FROM") {
	    throw std::runtime_error("Error: Se esperaba ',' o FROM después de las columnas.");
	}
	pos_++; // Consumir FROM

	// Verificar el nombre de la tabla
	if (pos_ >= tokens_.size() || tokens_[pos_].type != IDENTIFIER) {
	    throw std::runtime_error("Error: Se esperaba un nombre de tabla después de FROM.");
	}
	pos_++; // Consumir el nombre de la tabla

	// Verificar WHERE opcional
	if (pos_ < tokens_.size() && tokens_[pos_].type == KEYWORD && tokens_[pos_].value == "WHERE") {
	    pos_++; // Consumir WHERE

	    // Verificar condición: columna operador valor
	    if (pos_ >= tokens_.size() || tokens_[pos_].type != IDENTIFIER) {
		throw std::runtime_error("Error: Se esperaba una columna en la cláusula WHERE.");
	    }
	    pos_++; // Consumir columna

	    if (pos_ >= tokens_.size() || tokens_[pos_].type != OPERATOR) {
		throw std::runtime_error("Error: Se esperaba un operador en la cláusula WHERE.");
	    }
	    pos_++; // Consumir operador

	    if (pos_ >= tokens_.size() || (tokens_[pos_].type != NUMBER && tokens_[pos_].type != IDENTIFIER)) {
		throw std::runtime_error("Error: Se esperaba un valor en la cláusula WHERE.");
	    }
	    pos_++; // Consumir valor
	}

	// Verificar el punto y coma al final
	if (pos_ >= tokens_.size() || tokens_[pos_].type != SYMBOL || tokens_[pos_].value != ";") {
	    throw std::runtime_error("Error: Se esperaba ';' al final de la consulta.");
	}
	pos_++; // Consumir punto y coma

	// Confirmar que no haya tokens adicionales
	if (pos_ < tokens_.size()) {
	    throw std::runtime_error("Error: Tokens inesperados después del final de la consulta.");
	}
    }


    void parseCreate() {
        // CREATE TABLE <table> (<columns>)
        if (tokens_[pos_].value != "TABLE") {
            throw std::runtime_error("Error de sintaxis: Se esperaba TABLE.");
        }
	pos_++;
	
	if (tokens_[pos_].type != IDENTIFIER){
	    throw std::runtime_error("Error: Se esperaba nombre de la tabla.");
	}
	pos_++;

	if (tokens_[pos_].type != SYMBOL && tokens_[pos_].value != "("){
	    throw std::runtime_error("Error de sintaxis: Se esperaba '(' para definir columnas.");
	}
        pos_++;

	if (tokens_[pos_].type != IDENTIFIER){
	    throw std::runtime_error("Error de sintaxis: Se esperaba una columna.");
	}
	pos_++;

	while (pos_ < tokens_.size() && tokens_[pos_].value != ")") {

	    if (pos_ >= tokens_.size() || tokens_[pos_].type != SYMBOL || tokens_[pos_].value != ",") {
		throw std::runtime_error("Error: Se esperaba una ',' o ')'.");
	    }
	    pos_++;

	    if (tokens_[pos_].type != IDENTIFIER){
		throw std::runtime_error("Error: Se esperaba una columna.");
	    }
	    pos_++;
	}

	if (tokens_[pos_].type != SYMBOL && tokens_[pos_].value != ")"){
	    throw std::runtime_error("Error de sintaxis: Se esperaba ')' para definir columnas.");
	}

        pos_++;
        // Verificar el punto y coma al final
	if (pos_ >= tokens_.size() || tokens_[pos_].type != SYMBOL || tokens_[pos_].value != ";") {
	    throw std::runtime_error("Error: Se esperaba ';' al final de la consulta.");
	}
	pos_++; // Consumir punto y coma

	// Confirmar que no haya tokens adicionales
	if (pos_ < tokens_.size()) {
	    throw std::runtime_error("Error: Tokens inesperados después del final de la consulta.");
	}
    }

    void parseInsert() {
	// Verificar que la consulta comience con "INSERT INTO"
	if (tokens_[pos_].value != "INTO") {
	    throw std::runtime_error("Error de sintaxis: Se esperaba 'INTO' después de 'INSERT'.");
	}
	pos_++;

	// Verificar que el siguiente token sea el nombre de la tabla
	if (tokens_[pos_].type != IDENTIFIER) {
	    throw std::runtime_error("Error: Se esperaba el nombre de la tabla.");
	}
	pos_++;

	// Verificar apertura de paréntesis para las columnas
	if (tokens_[pos_].type != SYMBOL || tokens_[pos_].value != "(") {
	    throw std::runtime_error("Error de sintaxis: Se esperaba '(' después del nombre de la tabla.");
	}
	pos_++;

	// Leer los nombres de las columnas
	while (pos_ < tokens_.size() && tokens_[pos_].value != ")") {
	    if (tokens_[pos_].type != IDENTIFIER) {
		throw std::runtime_error("Error de sintaxis: Se esperaba un nombre de columna.");
	    }
	    pos_++;

	    if (tokens_[pos_].value == ")") break;

	    if (tokens_[pos_].type != SYMBOL || tokens_[pos_].value != ",") {
		throw std::runtime_error("Error de sintaxis: Se esperaba ',' entre nombres de columnas.");
	    }
	    pos_++;
	}

	// Verificar cierre de paréntesis para las columnas
	if (tokens_[pos_].type != SYMBOL || tokens_[pos_].value != ")") {
	    throw std::runtime_error("Error de sintaxis: Se esperaba ')' después de los nombres de las columnas.");
	}
	pos_++;

	// Verificar la palabra clave "VALUES"
	if (tokens_[pos_].value != "VALUES") {
	    throw std::runtime_error("Error de sintaxis: Se esperaba 'VALUES' después de las columnas.");
	}
	pos_++;

	// Verificar apertura de paréntesis para los valores
	if (tokens_[pos_].type != SYMBOL || tokens_[pos_].value != "(") {
	    throw std::runtime_error("Error de sintaxis: Se esperaba '(' antes de los valores.");
	}
	pos_++;

	// Leer los valores
	while (pos_ < tokens_.size() && tokens_[pos_].value != ")") {
	    if (tokens_[pos_].type != STRING && tokens_[pos_].type != NUMBER) {
		throw std::runtime_error("Error de sintaxis: Se esperaba un valor o tipo desconocido.");
	    }
	    pos_++;

	    if (tokens_[pos_].value == ")") break;

	    if (tokens_[pos_].type != SYMBOL || tokens_[pos_].value != ",") {
		throw std::runtime_error("Error de sintaxis: Se esperaba ',' entre los valores.");
	    }
	    pos_++;
	}

	// Verificar cierre de paréntesis para los valores
	if (tokens_[pos_].type != SYMBOL || tokens_[pos_].value != ")") {
	    throw std::runtime_error("Error de sintaxis: Se esperaba ')' después de los valores.");
	}
	pos_++;

	// Verificar el punto y coma al final
	if (pos_ >= tokens_.size() || tokens_[pos_].type != SYMBOL || tokens_[pos_].value != ";") {
	    throw std::runtime_error("Error: Se esperaba ';' al final de la consulta.");
	}
	pos_++; // Consumir punto y coma

	// Confirmar que no haya tokens adicionales
	if (pos_ < tokens_.size()) {
	    throw std::runtime_error("Error: Tokens inesperados después del final de la consulta.");
	}
    }


    void parseDelete() {
        // DELETE FROM <table> [WHERE <condition>]
        if (tokens_[pos_].value != "FROM"){
	    throw std::runtime_error("Error: Se esperaba FROM.");
	}
	pos_++;

	if (tokens_[pos_].type != IDENTIFIER){
	    throw std::runtime_error("Error: Se esperaba columna.");
	}
	pos_++;

	// Verificar WHERE opcional
	if (pos_ < tokens_.size() && tokens_[pos_].type == KEYWORD && tokens_[pos_].value == "WHERE") {
	    pos_++; // Consumir WHERE

	    // Verificar condición: columna operador valor
	    if (pos_ >= tokens_.size() || tokens_[pos_].type != IDENTIFIER) {
		throw std::runtime_error("Error: Se esperaba una columna en la cláusula WHERE.");
	    }
	    pos_++; // Consumir columna

	    if (pos_ >= tokens_.size() || tokens_[pos_].type != OPERATOR) {
		throw std::runtime_error("Error: Se esperaba un operador en la cláusula WHERE.");
	    }
	    pos_++; // Consumir operador

	    if (pos_ >= tokens_.size() || (tokens_[pos_].type != NUMBER && tokens_[pos_].type != IDENTIFIER)) {
		throw std::runtime_error("Error: Se esperaba un valor en la cláusula WHERE.");
	    }
	    pos_++; // Consumir valor
	}
	// Verificar el punto y coma al final
	if (pos_ >= tokens_.size() || tokens_[pos_].type != SYMBOL || tokens_[pos_].value != ";") {
	    throw std::runtime_error("Error: Se esperaba ';' al final de la consulta.");
	}
	pos_++; // Consumir punto y coma

	// Confirmar que no haya tokens adicionales
	if (pos_ < tokens_.size()) {
	    throw std::runtime_error("Error: Tokens inesperados después del final de la consulta.");
	}

    }

};


int main() {
    std::string sql1 = "SELECT id name FROM alumno WHERE id < 10;";
    std::string sql11 = "CREATE TABLE user (id, name);";
    std::string sql2 = "INSERT INTO users (name,id) VALUES ('John Doe');";
    std::string sql = "DELETE FROM user WHERE id = 10;";
    Lexer lexer(sql);
    std::vector<Token> tokens = lexer.tokenize();

    for(auto& token : tokens){
        std::cout << token.value << token.type << std::endl;
    }
    try {
        Parser parser(tokens);
        parser.parse();
        std::cout << "Consulta SQL válida.\n";
    } catch (const std::exception& e) {
        std::cerr << e.what() << "\n";
    }

    return 0;
}

