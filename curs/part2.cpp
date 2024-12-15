#include <iostream>
#include <fstream>
#include <string>
#include <cctype>
#include <unordered_map>
#include <stdexcept>
#include <vector>

using namespace std;

enum lex_type {
    LEX_NULL, LEX_REAL, LEX_DELIM,
    LEX_IF, LEX_ELSE, LEX_FOR, LEX_TO, LEX_STEP, LEX_NEXT,
    LEX_WHILE, LEX_READLN, LEX_WRITELN, LEX_TRUE, LEX_FALSE,
    LEX_ID, LEX_NUM, LEX_ASSIGN, LEX_EQ, LEX_NEQ, LEX_LT,
    LEX_LEQ, LEX_GT, LEX_GEQ, LEX_PLUS, LEX_MINUS, LEX_OR,
    LEX_TIMES, LEX_DIV, LEX_AND, LEX_NOT, LEX_BEGIN, LEX_END,
    LEX_FIN, LEX_RBRACE, LEX_LBRACE, LEX_COMMA, LEX_SEMICOLON,
    LEX_LPAREN, LEX_RPAREN, LEX_PERCENT, LEX_EXCLAMATION, LEX_DOLLAR // Новые типы
};

struct Lex {
    lex_type type;
    string value;

    Lex(lex_type t = LEX_NULL, const string& v = "") : type(t), value(v) {}
};

bool isNum(const string& input) {
    size_t i = 0;
    size_t len = input.length();

    if (input[len - 1] == 'B' || input[len - 1] == 'b') {
        for (i = 0; i < len - 1; i++) {
            if (input[i] != '0' && input[i] != '1') {
                return false;
            }
        }
        return true;
    }

    if (input[len - 1] == 'O' || input[len - 1] == 'o') {
        for (i = 0; i < len - 1; i++) {
            if (input[i] < '0' || input[i] > '7') {
                return false;
            }
        }
        return true;
    }

    if (input[len - 1] == 'D' || input[len - 1] == 'd') {
        for (i = 0; i < len - 1; i++) {
            if (!isdigit(input[i])) {
                return false;
            }
        }
        return true;
    }

    if (input[len - 1] == 'H' || input[len - 1] == 'h') {
        for (i = 0; i < len - 1; i++) {
            if (!isdigit(input[i]) && (input[i] < 'A' || input[i] > 'F') &&
                (input[i] < 'a' || input[i] > 'f')) {
                return false;
            }
        }
        return true;
    }

    i = 0;
    while (i < len && isdigit(input[i])) {
        i++;
    }

    if (len == i) return true;

    if (i < len && input[i] == '.') {
        bool hasDigits = false;
        i++;
        while (i < len && isdigit(input[i])) {
            i++;
            hasDigits = true;
        }
        if (!hasDigits) return false;
    }

    if (i < len && (input[i] == 'E' || input[i] == 'e')) {
        i++;
        if (i < len && (input[i] == '+' || input[i] == '-')) {
            i++;
        }
        bool hasExpDigits = false;
        while (i < len && isdigit(input[i])) {
            i++;
            hasExpDigits = true;
        }
        if (!hasExpDigits) return false;
    }

    return i == len;
}

class Lexer {
private:
    ifstream file;
    char current_char;
    unordered_map<string, lex_type> keywords = {
    {"if", LEX_IF}, {"else", LEX_ELSE}, {"for", LEX_FOR}, {"to", LEX_TO}, {"step", LEX_STEP},
    {"next", LEX_NEXT}, {"while", LEX_WHILE}, {"readln", LEX_READLN}, {"writeln", LEX_WRITELN},
    {"true", LEX_TRUE}, {"false", LEX_FALSE}, {"begin", LEX_BEGIN}, {"end", LEX_END}, {"(", LEX_LPAREN},
    {")", LEX_RPAREN}
    };

    void advance() { current_char = file.get(); }
    void skip_whitespace() { while (isspace(current_char)) advance(); }
    void skip_comment() {
        advance(); // Пропускаем '/'
        advance(); // Пропускаем '*'
        while (current_char != EOF) {
            if (current_char == '*' && file.peek() == '/') {
                advance(); // Пропускаем '*'
                advance(); // Пропускаем '/'
                return;
            }
            advance();
        }
        throw runtime_error("Unterminated comment.");
    }

    Lex parse_identifier() {
        string result;
        while (isalnum(current_char)) {
            result += current_char;
            advance();
        }
        return keywords.count(result) ? Lex(keywords[result], result) : Lex(LEX_ID, result);
    }

    Lex parse_number() {
        string result;

        while (isalnum(current_char) || current_char == '.' || current_char == '+' || current_char == '-') {
            result += current_char;
            advance();
        }

        return isNum(result) ? Lex(LEX_NUM, result) : throw runtime_error("Некорректное число: " + result);
    }

    Lex parse_operator() {
        string result(1, current_char);
        advance();

        if (result == ":") {
            if (current_char == '=') {
                result += current_char;
                advance();
                return Lex(LEX_ASSIGN, result);
            }
            else {
                throw runtime_error("':' is used incorrectly.");
            }
        }

        if (result == "=") {
            if (current_char == '=') {
                result += current_char;
                advance();
                return Lex(LEX_EQ, result);
            }
            else {
                throw runtime_error("'=' is used incorrectly.");
            }
        }

        if (result == "!") {
            // Проверяем, является ли следующий символ идентификатором
            if (isalpha(current_char)) {
                // Если да, то это тип данных
                return Lex(LEX_NOT, result);
            }
            else {
                // Иначе это оператор отрицания
                return Lex(LEX_EXCLAMATION, result);
            }
        }

        if (result == "<") {
            if (current_char == '=') {
                result += current_char;
                advance();
                return Lex(LEX_LEQ, result);
            }
            return Lex(LEX_LT, result);
        }

        if (result == ">") {
            if (current_char == '=') {
                result += current_char;
                advance();
                return Lex(LEX_GEQ, result);
            }
            return Lex(LEX_GT, result);
        }

        if (result == "|") {
            if (current_char == '|') {
                result += current_char;
                advance();
                return Lex(LEX_OR, result);
            }
        }

        if (result == "&") {
            if (current_char == '&') {
                result += current_char;
                advance();
                return Lex(LEX_AND, result);
            }
        }

        if (result == "+") return Lex(LEX_PLUS, result);
        if (result == "-") return Lex(LEX_MINUS, result);
        if (result == "*") return Lex(LEX_TIMES, result);
        if (result == "/") return Lex(LEX_DIV, result);
        if (result == "%") return Lex(LEX_PERCENT, result); // Новый тип
        if (result == "$") return Lex(LEX_DOLLAR, result);  // Новый тип

        throw runtime_error("Unknown operator: " + result);
    }

    Lex parse_delimiter() {
        string result(1, current_char);
        advance();

        if (result == "(") return Lex(LEX_LPAREN, result); // Левая скобка
        if (result == ")") return Lex(LEX_RPAREN, result); // Правая скобка
        if (result == ";") return Lex(LEX_SEMICOLON, result);
        if (result == ",") return Lex(LEX_COMMA, result);
        if (result == "{") return Lex(LEX_LBRACE, result);
        if (result == "}") return Lex(LEX_RBRACE, result);

        throw runtime_error("Unknown symbol: " + result);
    }

    Lex parse_type() {
        string result(1, current_char);
        advance();

        if (result == "%") return Lex(LEX_PERCENT, result);
        if (result == "!") return Lex(LEX_EXCLAMATION, result);
        if (result == "$") return Lex(LEX_DOLLAR, result);

        throw runtime_error("Неизвестный символ типа: " + result);
    }

public:
    explicit Lexer(const string& filename) {
        file.open(filename);
        if (!file.is_open()) {
            throw runtime_error("Файл не найден или не может быть открыт");
        }
        advance();
    }

    vector<Lex> get_lex_table() {
        vector<Lex> lex_table;
        Lex token;
        while ((token = get_next_lex()).type != LEX_FIN) {
            lex_table.push_back(token);
        }
        return lex_table;
    }

    Lex get_next_lex() {
        while (current_char != EOF) {
            if (current_char == '/' && file.peek() == '*') {
                skip_comment();
                continue;
            }
            if (isspace(current_char)) {
                skip_whitespace();
                continue;
            }
            if (isalpha(current_char)) return parse_identifier();
            if (isdigit(current_char) || current_char == '.') return parse_number();
            if (current_char == '=' || current_char == '!' || current_char == '<' || current_char == '>' ||
                current_char == '+' || current_char == '-' || current_char == '*' || current_char == '/' ||
                current_char == '&' || current_char == '|' || current_char == ':' || current_char == '$' ||
                current_char == '%') {
                return parse_operator();
            }
            if (current_char == '(' || current_char == ')' || current_char == ';' || current_char == ',' ||
                current_char == '{' || current_char == '}') {
                return parse_delimiter(); // Обрабатываем скобки как разделители
            }
        }
        return Lex(LEX_FIN, "");
    }
};

const char* lex_type_description(lex_type type) {
    switch (type) {
    case LEX_NULL: return "NULL";
    case LEX_REAL: return "REAL";
    case LEX_IF: return "IF";
    case LEX_ELSE: return "ELSE";
    case LEX_FOR: return "FOR";
    case LEX_TO: return "TO";
    case LEX_STEP: return "STEP";
    case LEX_NEXT: return "NEXT";
    case LEX_WHILE: return "WHILE";
    case LEX_READLN: return "READLN";
    case LEX_WRITELN: return "WRITELN";
    case LEX_TRUE: return "TRUE";
    case LEX_FALSE: return "FALSE";
    case LEX_ID: return "IDENTIFIER";
    case LEX_NUM: return "NUMBER";
    case LEX_ASSIGN: return "ASSIGN";
    case LEX_EQ: return "EQUAL";
    case LEX_NEQ: return "NOT EQUAL";
    case LEX_LT: return "LESS THAN";
    case LEX_LEQ: return "LESS OR EQUAL";
    case LEX_GT: return "GREATER THAN";
    case LEX_GEQ: return "GREATER OR EQUAL";
    case LEX_PLUS: return "PLUS";
    case LEX_MINUS: return "MINUS";
    case LEX_OR: return "OR";
    case LEX_TIMES: return "TIMES";
    case LEX_DIV: return "DIVIDE";
    case LEX_AND: return "AND";
    case LEX_NOT: return "NOT";
    case LEX_BEGIN: return "BEGIN";
    case LEX_END: return "END";
    case LEX_FIN: return "FIN";
    case LEX_PERCENT: return "TYPE_PERCENT (Integer)"; // Описание для %
    case LEX_EXCLAMATION: return "TYPE_EXCLAMATION (Boolean)"; // Описание для !
    case LEX_DOLLAR: return "TYPE_DOLLAR (float)"; // Описание для $
    case LEX_LPAREN: return "LEFT PARENTHESIS"; // Добавлено
    case LEX_RPAREN: return "RIGHT PARENTHESIS"; // Добавлено
    case LEX_RBRACE: return "RIGHT BRACE";
    case LEX_LBRACE: return "LEFT BRACE";
    default: return "UNKNOWN";
    }
}


class Parser {
private:
    vector<Lex> lex_table;
    size_t pos;

    Lex current() {
        if (pos < lex_table.size()) {
            return lex_table[pos];
        }
        return Lex(LEX_FIN, "");
    }

    void advance() {
        if (pos < lex_table.size()) {
            pos++;
        }
    }

    void expect(lex_type type) {
        if (current().type != type) {
            throw runtime_error("Syntax error: Expected " + string(lex_type_description(type)) + ", but found " + lex_type_description(current().type));
        }
        advance();
    }

    void validate_identifier(const string& identifier) {
        if (identifier.empty()) {
            throw runtime_error("Semantic error: Identifier cannot be empty.");
        }
        if (!isalpha(identifier[0])) {
            throw runtime_error("Semantic error: Identifier must start with a letter.");
        }
        for (char c : identifier) {
            if (!isalnum(c)) {
                throw runtime_error("Semantic error: Identifier can only contain alphanumeric characters.");
            }
        }
    }

public:
    Parser(const vector<Lex>& lex_table) : lex_table(lex_table), pos(0) {}

    void parse() {
        program();
        if (current().type != LEX_FIN) {
            throw runtime_error("Syntax error: Unexpected token after program end.");
        }
    }

    void program() {
        while (current().type != LEX_FIN) {
            if (current().type == LEX_PERCENT || current().type == LEX_EXCLAMATION || current().type == LEX_DOLLAR) {
                description(); // Обрабатываем описание переменных
            }
            else {
                statement(); // Обрабатываем оператор
            }
        }
    }

    void description() {
        // Проверяем тип данных (% | ! | $)
        if (current().type != LEX_PERCENT && current().type != LEX_EXCLAMATION && current().type != LEX_DOLLAR) {
            throw runtime_error("Syntax error: Expected type (% | ! | $).");
        }
        advance(); // Переходим к типу

        // Проверяем идентификатор
        validate_identifier(current().value);
        expect(LEX_ID);

        // Обрабатываем возможные дополнительные идентификаторы через запятую
        while (current().type == LEX_COMMA) {
            advance();
            validate_identifier(current().value);
            expect(LEX_ID);
        }
    }

    void statement() {
        if (current().type == LEX_ID) {
            assignment();
        }
        else if (current().type == LEX_IF) {
            conditional();
        }
        else if (current().type == LEX_FOR) {
            for_loop();
        }
        else if (current().type == LEX_WHILE) {
            while_loop();
        }
        else if (current().type == LEX_READLN) {
            input_statement();
        }
        else if (current().type == LEX_WRITELN) {
            output_statement();
        }
        else if (current().type == LEX_BEGIN) {
            compound_statement();
        }
        else {
            throw runtime_error("Syntax error: Unexpected token " + string(lex_type_description(current().type)));
        }
    }

    void compound_statement() {
        expect(LEX_BEGIN);
        statement();
        while (current().type == LEX_SEMICOLON) {
            advance();
            statement();
        }
        expect(LEX_END);
    }

    void assignment() {
        validate_identifier(current().value);
        expect(LEX_ID);
        expect(LEX_ASSIGN);
        expression();
    }

    void conditional() {
        expect(LEX_IF);
        expect(LEX_LPAREN);
        expression();
        expect(LEX_RPAREN);
        statement();
        if (current().type == LEX_ELSE) {
            advance();
            statement();
        }
    }

    void for_loop() {
        expect(LEX_FOR);
        assignment();
        expect(LEX_TO);
        expression();
        if (current().type == LEX_STEP) {
            advance();
            expression();
        }
        statement();
        expect(LEX_NEXT);
    }

    void while_loop() {
        expect(LEX_WHILE);
        expect(LEX_LPAREN);
        expression();
        expect(LEX_RPAREN);
        statement();
    }

    void input_statement() {
        expect(LEX_READLN);
        validate_identifier(current().value);
        expect(LEX_ID);
        while (current().type == LEX_COMMA) {
            advance();
            validate_identifier(current().value);
            expect(LEX_ID);
        }
    }

    void output_statement() {
        expect(LEX_WRITELN); // Ожидаем ключевое слово writeln
        expect(LEX_LPAREN); // Ожидаем открывающую скобку
        expression(); // Обрабатываем выражение внутри скобок
        expect(LEX_RPAREN); // Ожидаем закрывающую скобку
        while (current().type == LEX_COMMA) {
            advance();
            expression();
        }
    }

    void expression() {
        operand();
        while (current().type == LEX_PLUS || current().type == LEX_MINUS || current().type == LEX_EQ || current().type == LEX_NEQ || current().type == LEX_LT || current().type == LEX_LEQ || current().type == LEX_GT || current().type == LEX_GEQ) {
            advance();
            operand();
        }
    }

    void operand() {
        if (current().type == LEX_ID) {
            validate_identifier(current().value); // Проверяем идентификатор
            advance(); // Переходим к следующей лексеме
        }
        else if (current().type == LEX_NUM) {
            advance(); // Переходим к следующей лексеме
        }
        else if (current().type == LEX_LPAREN) {
            advance(); // Переходим к следующей лексеме
            expression(); // Обрабатываем выражение внутри скобок
            expect(LEX_RPAREN); // Ожидаем закрывающую скобку
        }
        else if (current().type == LEX_TRUE || current().type == LEX_FALSE) {
            advance(); // Переходим к следующей лексеме
        }
        else if (current().type == LEX_NOT) {
            advance(); // Переходим к следующей лексеме
            operand(); // Обрабатываем следующий операнд
        }
        else {
            throw runtime_error("Syntax error: Unexpected token " + string(lex_type_description(current().type)));
        }
    }

    void term() {
        factor();
        while (current().type == LEX_TIMES || current().type == LEX_DIV || current().type == LEX_AND) {
            advance();
            factor();
        }
    }

    void factor() {
        if (current().type == LEX_ID) {
            validate_identifier(current().value);
            advance();
        }
        else if (current().type == LEX_NUM) {
            advance();
        }
        else if (current().type == LEX_LPAREN) {
            advance();
            expression();
            expect(LEX_RPAREN);
        }
        else if (current().type == LEX_TRUE || current().type == LEX_FALSE) {
            advance();
        }
        else if (current().type == LEX_NOT) {
            advance();
            factor();
        }
        else {
            throw runtime_error("Syntax error: Unexpected token " + string(lex_type_description(current().type)));
        }
    }
};

int main() {
    setlocale(0, "");
    try {
        Lexer lexer("input.txt");
        vector<Lex> lex_table = lexer.get_lex_table();

        Parser parser(lex_table);
        parser.parse();

        cout << "Parsing successful!" << endl;
    }
    catch (const exception& e) {
        cerr << "Error: " << e.what() << endl;
    }
    return 0;
}


/*
int main() {
    setlocale(0, "");
    try {
        Lexer lexer("input.txt");
        vector<Lex> lex_table = lexer.get_lex_table();

        for (const auto& token : lex_table) {
            cout << "Token: " << lex_type_description(token.type)
                << ", value: " << token.value << endl;
        }
    }
    catch (const exception& e) {
        cerr << "Ошибка: " << e.what() << endl;
    }
    return 0;
}
*/
