#include <iostream>
#include <string>
#include <vector>
#include <set>
#include <cctype>
#include <stdexcept>

// Definición de tipos de tokens
enum TokenType {
    KEYWORD,        // Palabras clave como CREATE, INSERT, etc.
    IDENTIFIER,     // Nombres de tablas o columnas
    SYMBOL,         // Símbolos como '(', ')', ',', ';'
    OPERATOR,       // Operadores como '=', '>', '<'
    LITERAL,        // Literales como cadenas o números
    END_OF_FILE     // Final de la entrada
};

// Representación de un token
struct Token {
    TokenType type;
    std::string value;

    Token(TokenType t, const std::string& v) : type(t), value(v) {}
};

// Palabras clave de SQL
const std::set<std::string> keywords = {
    "CREATE", "TABLE", "INSERT", "INTO", "SELECT", "UPDATE", "DELETE", "FROM", "WHERE", "VALUES", "SET"
};

// Símbolos válidos en SQL
const std::set<char> symbols = {
    '(', ')', ',', ';', '*', '='
};

// Clase Lexer para la tokenización
class Lexer {
private:
    std::string input;
    size_t pos;

    // Avanza hasta el siguiente carácter no blanco
    void skipWhitespace() {
        while (pos < input.size() && std::isspace(input[pos])) {
            pos++;
        }
    }

    // Extrae un identificador o palabra clave
    std::string extractWord() {
        size_t start = pos;
        while (pos < input.size() && (std::isalnum(input[pos]) || input[pos] == '_')) {
            pos++;
        }
        return input.substr(start, pos - start);
    }

    // Extrae un literal numérico o de cadena
    std::string extractLiteral() {
        if (input[pos] == '\'') { // Cadenas delimitadas por comillas simples
            size_t start = ++pos;
            while (pos < input.size() && input[pos] != '\'') {
                pos++;
            }
            if (pos >= input.size() || input[pos] != '\'') {
                throw std::runtime_error("Error de sintaxis: literal no cerrado con comillas simples");
            }
            pos++; // Saltar la comilla final
            return input.substr(start, pos - start - 1);
        } else { // Números
            size_t start = pos;
            while (pos < input.size() && std::isdigit(input[pos])) {
                pos++;
            }
            return input.substr(start, pos - start);
        }
    }

public:
    Lexer(const std::string& sql) : input(sql), pos(0) {}

    std::vector<Token> tokenize() {
        std::vector<Token> tokens;
        while (pos < input.size()) {
            skipWhitespace();

            if (pos >= input.size()) break;

            if (std::isalpha(input[pos])) { // Identificadores o palabras clave
                std::string word = extractWord();
                if (keywords.count(word)) {
                    tokens.emplace_back(KEYWORD, word);
                } else {
                    tokens.emplace_back(IDENTIFIER, word);
                }
            } else if (std::isdigit(input[pos]) || input[pos] == '\'') { // Literales
                tokens.emplace_back(LITERAL, extractLiteral());
            } else if (symbols.count(input[pos])) { // Símbolos
                tokens.emplace_back(SYMBOL, std::string(1, input[pos++]));
            } else { // Operadores
                tokens.emplace_back(OPERATOR, std::string(1, input[pos++]));
            }
        }
        tokens.emplace_back(END_OF_FILE, "");
        return tokens;
    }
};

// Nodo base para el AST
struct ASTNode {
    virtual ~ASTNode() = default;
};

// Nodo para CREATE TABLE
struct CreateTableNode : ASTNode {
    std::string tableName;
    std::vector<std::pair<std::string, std::string>> columns;

    CreateTableNode(const std::string& tableName,
                    const std::vector<std::pair<std::string, std::string>>& columns)
        : tableName(tableName), columns(columns) {}
};

struct SelectNode : ASTNode {
    std::vector<std::string> columns;  // Columnas seleccionadas
    std::string tableName;             // Nombre de la tabla
    std::string condition;            // Condición WHERE (opcional)

    // Constructor que inicializa los campos
    SelectNode(const std::vector<std::string>& columns, const std::string& tableName, const std::string& condition = "")
        : columns(columns), tableName(tableName), condition(condition) {}
};



// Clase Parser
class Parser {
private:
    std::vector<Token> tokens;
    size_t current;

    // Devuelve el token actual
    Token& currentToken() {
        return tokens[current];
    }

    // Verifica y avanza al siguiente token
    void consume(TokenType type, const std::string& expectedValue = "") {
        if (current >= tokens.size() || currentToken().type != type ||
            (!expectedValue.empty() && currentToken().value != expectedValue)) {
            throw std::runtime_error("Error de sintaxis: se esperaba " + expectedValue);
        }
        current++;
    }

public:
    Parser(const std::vector<Token>& tokens) : tokens(tokens), current(0) {}
    CreateTableNode* parseCreateTable() {
    consume(KEYWORD, "CREATE");
    consume(KEYWORD, "TABLE");
    std::string tableName = currentToken().value;
    consume(IDENTIFIER);

    consume(SYMBOL, "(");
    std::vector<std::pair<std::string, std::string>> columns;

    while (true) {
        // Procesar el nombre de la columna
        if (currentToken().type == IDENTIFIER) {
            std::string columnName = currentToken().value;
            consume(IDENTIFIER);

            // Procesar el tipo de dato
            if (currentToken().type == IDENTIFIER) {
                std::string columnType = currentToken().value;
                consume(IDENTIFIER);

                // Manejo adicional para tipos como VARCHAR(50)
                if (currentToken().value == "(") {
                    columnType += currentToken().value; // Añadir '('
                    consume(SYMBOL, "(");

                    if (currentToken().type == LITERAL) {
                        columnType += currentToken().value; // Añadir el valor literal
                        consume(LITERAL);
                    } else {
                        throw std::runtime_error(
                            "Error de sintaxis: se esperaba un valor numérico dentro de los paréntesis");
                    }

                    if (currentToken().value == ")") {
                        columnType += currentToken().value; // Añadir ')'
                        consume(SYMBOL, ")");
                    } else {
                        throw std::runtime_error(
                            "Error de sintaxis: se esperaba ')' para cerrar el tipo de dato");
                    }
                }

                columns.emplace_back(columnName, columnType);
            } else {
                throw std::runtime_error(
                    "Error de sintaxis: se esperaba un tipo de dato para la columna '" + columnName + "'");
            }
        } else {
            throw std::runtime_error("Error de sintaxis: se esperaba el nombre de una columna");
        }

        // Procesar separadores ',' o el cierre ')'
        if (currentToken().value == ",") {
            consume(SYMBOL, ",");
        } else if (currentToken().value == ")") {
            consume(SYMBOL, ")");
            break; // Fin del listado de columnas
        } else {
            throw std::runtime_error(
                "Error de sintaxis: se esperaba ',' o ')' después de la definición de la columna");
        }
    }

    return new CreateTableNode(tableName, columns);
    }

    // Analiza SELECT
    SelectNode* parseSelect() {
    consume(KEYWORD, "SELECT");

    // Analizamos las columnas (pueden ser '*' o una lista de columnas)
    std::vector<std::string> columns;
    if (currentToken().value == "*") {
        columns.push_back("*");
        consume(SYMBOL, "*");
    } else {
        while (currentToken().type == IDENTIFIER) {
            columns.push_back(currentToken().value);
            consume(IDENTIFIER);
            if (currentToken().value == ",") {
                consume(SYMBOL, ",");
            } else {
                break;
            }
        }
    }

    consume(KEYWORD, "FROM");
    std::string tableName = currentToken().value;
    consume(IDENTIFIER);

    std::string condition;  // Cadena que almacenará la condición en caso de existir
    if (currentToken().value == "WHERE") {
        consume(KEYWORD, "WHERE");
        condition = currentToken().value;
        consume(IDENTIFIER); // Captura la condición simple (en este caso, como "age")

        if (currentToken().type == OPERATOR) {
            condition += " " + currentToken().value;
            consume(OPERATOR); // Captura el operador (en este caso, ">")
            condition += " " + currentToken().value;
            consume(LITERAL);  // Captura el valor después del operador (en este caso, "30")
        }
    }

    return new SelectNode{columns, tableName, condition};
    }


   

    // Punto de entrada para parsear cualquier consulta
    ASTNode* parse() {
    if (currentToken().value == "CREATE") {
        return parseCreateTable();
    } else if (currentToken().value == "SELECT") {
        return parseSelect();
    }
    throw std::runtime_error("Error de sintaxis: consulta no reconocida.");
    }
 
};

int main() {
    std::string sql = "SELECT id, name FROM users WHERE age > 30;";
    Lexer lexer(sql);
    auto tokens = lexer.tokenize();

    try {
        Parser parser(tokens);
        ASTNode* ast = parser.parse();

        if (auto createNode = dynamic_cast<CreateTableNode*>(ast)) {
            std::cout << "CREATE TABLE: " << createNode->tableName << "\n";
            for (const auto& col : createNode->columns) {
                std::cout << "  Column: " << col.first << " " << col.second << "\n";
            }
        }
	else if (auto selectNode = dynamic_cast<SelectNode*>(ast)) {
    	    std::cout << "SELECT columns: ";
    	    for (const auto& col : selectNode->columns) {
        	std::cout << col << " ";
    	    }
    	    std::cout << "\nFROM table: " << selectNode->tableName << "\n";

    	    if (!selectNode->condition.empty()) {
        	std::cout << "WHERE condition: " << selectNode->condition << "\n";
    	    }
	}

        delete ast; // Libera la memoria
    } catch (const std::exception& e) {
        std::cerr << e.what() << "\n";
    }

    return 0;
}

