#include <iostream>
#include <fstream>
#include <string>
#include <cctype>
#include <unordered_map>
#include <stdexcept>
#include <vector>

using namespace std;

enum lex_type 
{
    LEX_NULL, LEX_INTEGER, LEX_REAL, LEX_BOOLEAN,
    LEX_IF, LEX_ELSE, LEX_FOR, LEX_TO, LEX_STEP, LEX_NEXT,
    LEX_WHILE, LEX_READLN, LEX_WRITELN, LEX_TRUE, LEX_FALSE,
    LEX_ID, LEX_NUM, LEX_ASSIGN, LEX_EQ, LEX_NEQ, LEX_LT,
    LEX_LEQ, LEX_GT, LEX_GEQ, LEX_PLUS, LEX_MINUS, LEX_OR,
    LEX_TIMES, LEX_DIV, LEX_AND, LEX_NOT, LEX_BEGIN, LEX_END,
    LEX_FIN, LEX_RBRACE, LEX_LBRACE, LEX_COMMA, LEX_SEMICOLON,
    LEX_LPAREN, LEX_RPAREN, LEX_RNUM
};

struct Lex 
{
    lex_type type;
    string value;

    Lex(lex_type t = LEX_NULL, const string& v = "") : type(t), value(v) {}
};

bool isNum(const string& input) 
{
    size_t i = 0;
    size_t len = input.length();

    if (input[len - 1] == 'B' || input[len - 1] == 'b') 
    {
        for (i = 0; i < len - 1; i++)
        {
            if (input[i] != '0' && input[i] != '1') 
            {
                return false;
            }
        }
        return true;
    }

    if (input[len - 1] == 'O' || input[len - 1] == 'o') 
    {
        for (i = 0; i < len - 1; i++) 
        {
            if (input[i] < '0' || input[i] > '7') 
            {
                return false;
            }
        }
        return true;
    }

    if (input[len - 1] == 'D' || input[len - 1] == 'd')
    {
        for (i = 0; i < len - 1; i++) 
        {
            if (!isdigit(input[i])) 
            {
                return false;
            }
        }
        return true;
    }

    if (input[len - 1] == 'H' || input[len - 1] == 'h') 
    {
        for (i = 0; i < len - 1; i++)
        {
            if (!isdigit(input[i]) && (input[i] < 'A' || input[i] > 'F') &&
                (input[i] < 'a' || input[i] > 'f')) 
            {
                return false;
            }
        }
        return true;
    }

    i = 0;
    while (i < len && isdigit(input[i])) 
    {
        i++;
    }

    if (len == i) return true;

    if (i < len && input[i] == '.') 
    {
        bool hasDigits = false;
        i++;
        while (i < len && isdigit(input[i])) 
        {
            i++;
            hasDigits = true;
        }
        if (!hasDigits) return false;
    }

    if (i < len && (input[i] == 'E' || input[i] == 'e'))
    {
        i++;
        if (i < len && (input[i] == '+' || input[i] == '-')) 
        {
            i++;
        }
        bool hasExpDigits = false;
        while (i < len && isdigit(input[i])) 
        {
            i++;
            hasExpDigits = true;
        }
        if (!hasExpDigits) return false;
    }

    return i == len;
}

class Lexer
{
private:
    ifstream file;
    char current_char;
    unordered_map<string, lex_type> keywords = 
    {
        {"if", LEX_IF}, {"else", LEX_ELSE}, {"for", LEX_FOR}, {"to", LEX_TO}, {"step", LEX_STEP},
        {"next", LEX_NEXT}, {"while", LEX_WHILE}, {"readln", LEX_READLN}, {"writeln", LEX_WRITELN},
        {"true", LEX_TRUE}, {"false", LEX_FALSE}, {"begin", LEX_BEGIN}, {"end", LEX_END}
    };

    void nextSim() { current_char = file.get(); }
    void skipSpace() { while (isspace(current_char)) nextSim(); }
    void skipComment() 
    {
        nextSim(); nextSim();
        while (current_char != EOF)
        {
            if (current_char == '*' && file.peek() == '/') 
            {
                nextSim(); nextSim();
                return;
            }
            nextSim();
        }
    }

    Lex getIdentifier() 
    {
        string result;
        while (isalnum(current_char)) 
        {
            result += current_char;
            nextSim();
        }
        return keywords.count(result) ? Lex(keywords[result], result) : Lex(LEX_ID, result);
    }

    Lex getNum()
    {
        string result;

        while (isalnum(current_char) || current_char == '.' || current_char == '+' || current_char == '-') 
        {
            result += current_char;
            nextSim();
        }

        if (isNum(result)) 
        {
            if (result.find(".") != -1) 
            {
                return Lex(LEX_RNUM, result);
            } 
            else
            {
                return Lex(LEX_NUM, result);
            }
        }
        else
        {
            throw runtime_error("Лексическая ошибка - Неверное число '" + result + "'.");
        }
    }

    Lex getOper() 
    {
        string result(1, current_char);
        nextSim();

        if (result == ":")
        {
            if (current_char == '=')
            {
                result += current_char;
                nextSim();
                return Lex(LEX_ASSIGN, result);
            }
            else 
            {
                throw runtime_error("Лексическая ошибка - ':' используется неправильно.");
            }
        }

        if (result == "=") 
        {
            if (current_char == '=')
            {
                result += current_char;
                nextSim();
                return Lex(LEX_EQ, result);
            }
            else 
            {
                throw runtime_error("Лексическая ошибка - неправильно используется '='.");
            }
        }

        if (result == "!") {
            if (current_char == '=') {
                result += current_char;
                nextSim();
                return Lex(LEX_NEQ, result);
            }
            else if (current_char == ' ')
            {
                result += current_char;
                nextSim();
                return Lex(LEX_REAL, result);
            }
            return Lex(LEX_NOT, result);
        }

        if (result == "<") 
        {
            if (current_char == '=') 
            {
                result += current_char;
                nextSim();
                return Lex(LEX_LEQ, result);
            }
            return Lex(LEX_LT, result);
        }

        if (result == ">") 
        {
            if (current_char == '=')
            {
                result += current_char;
                nextSim();
                return Lex(LEX_GEQ, result);
            }
            return Lex(LEX_GT, result);
        }

        if (result == "|")
        {
            if (current_char == '|') 
            {
                result += current_char;
                nextSim();
                return Lex(LEX_OR, result);
            }
        }

        if (result == "&") 
        {
            if (current_char == '&')
            {
                result += current_char;
                nextSim();
                return Lex(LEX_AND, result);
            }
        }

        if (result == "+") return Lex(LEX_PLUS, result);
        if (result == "-") return Lex(LEX_MINUS, result);
        if (result == "*") return Lex(LEX_TIMES, result);
        if (result == "/") return Lex(LEX_DIV, result);

        throw runtime_error("Лексическая ошибка - Неизвестный оператор: '" + result + "'.");
    }

    Lex getDelimiter()
    {
        string result(1, current_char);
        nextSim();

        if (result == "%") return Lex(LEX_INTEGER, result);
        if (result == "!") return Lex(LEX_REAL, result);
        if (result == "$") return Lex(LEX_BOOLEAN, result);

        // Обработка других разделителей
        if (result == "(" || result == ")" || result == ";" || result == "{" || result == "}" || result == ",") 
        {
            if (result == "(") return Lex(LEX_LPAREN, result);
            if (result == ")") return Lex(LEX_RPAREN, result);
            if (result == ";") return Lex(LEX_SEMICOLON, result);
            if (result == ",") return Lex(LEX_COMMA, result);
            if (result == "{") return Lex(LEX_LBRACE, result);
            if (result == "}") return Lex(LEX_RBRACE, result);
        }
        else 
        {
            throw runtime_error("Лексическая ошибка - Неизвестный символ: '" + result + "'.");
        }
    }

public:
    explicit Lexer(const string& filename)
    {
        file.open(filename);
        if (!file.is_open())
        {
            throw runtime_error("Файл не найден или не может быть открыт.");
        }
        nextSim();
    }

    vector<Lex> getLexTable()
    {
        vector<Lex> lex_table;
        Lex token;
        while ((token = getNextLex()).type != LEX_FIN) 
        {
            lex_table.push_back(token);
        }
        return lex_table;
    }

    Lex getNextLex()
    {
        while (current_char != EOF) {
            if (current_char == '/' && file.peek() == '*') 
            {
                skipComment();
                continue;
            }
            if (isspace(current_char)) 
            {
                skipSpace();
                continue;
            }
            if (isalpha(current_char)) return getIdentifier();
            if (isdigit(current_char) || current_char == '.') return getNum();
            if (current_char == '=' || current_char == '!' || current_char == '<' || current_char == '>' ||
                current_char == '+' || current_char == '-' || current_char == '*' || current_char == '/' ||
                current_char == '&' || current_char == '|' || current_char == ':')
            {
                return getOper();
            }
            return getDelimiter();
        }
        return Lex(LEX_FIN, "");
    }
};

const char* lexType(lex_type type)
{
    switch (type) 
    {
    case LEX_NULL: return "NULL";
    case LEX_INTEGER: return "INTEGER";
    case LEX_REAL: return "REAL";
    case LEX_BOOLEAN: return "BOOLEAN";
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
    case LEX_RNUM: return "REAL NUMBER";
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
    case LEX_RBRACE: return "RIGHT BRACE";
    case LEX_LBRACE: return "LEFT BRACE";
    case LEX_COMMA: return "COMMA";
    case LEX_SEMICOLON: return "SEMICOLON";
    case LEX_LPAREN: return "LEFT PARENTHESIS";
    case LEX_RPAREN: return "RIGHT PARENTHESIS";
    default: return "UNKNOWN";
    }

}

struct Identifier 
{
    string name;
    lex_type type;
};

class Parser
{
private:
    vector<Lex> lex_table;
    unordered_map<string, lex_type> symbol_table; // Таблица идентификаторов
    size_t pos;

    Lex current() 
    {
        if (pos < lex_table.size()) 
        {
            return lex_table[pos];
        }
        return Lex(LEX_FIN, "");
    }

    void nextSim() 
    {
        if (pos < lex_table.size())
        {
            pos++;
        }
    }

    void expect(lex_type type) 
    {
        if (current().type != type) 
        {
            throw runtime_error("Синтаксическая ошибка: Ожидается " + string(lexType(type)) + ", но найден " + lexType(current().type) + ".");
        }
        nextSim();
    }

    void errDeclaredIdentifier(const string& identifier)
    {
        if (symbol_table.find(identifier) == symbol_table.end())
        {
            throw runtime_error("Семантическая ошибка - идентификатор '" + identifier + "' не объявлен.");
        }
    }

    void errIdentifier(const string& identifier)
    {
        if (identifier.empty())
        {
            throw runtime_error("Семантическая ошибка - идентификатор не может быть пустым.");
        }
        if (!isalpha(identifier[0]))
        {
            throw runtime_error("Семантическая ошибка - Идентификатор должен начинаться с буквы.");
        }
        for (char c : identifier)
        {
            if (!isalnum(c))
            {
                throw runtime_error("Семантическая ошибка - Идентификатор может содержать только буквенно-цифровые символы.");
            }
        }
    }

    void errDeclareIdentifier(const string& identifier, lex_type type) 
    {
        if (symbol_table.find(identifier) != symbol_table.end())
        {
            throw runtime_error("Семантическая ошибка - идентификатор ' " + identifier + "' уже объявлен.");
        }
        symbol_table[identifier] = type;
    }

    lex_type getIdentifierType(const string& identifier) 
    {
        auto it = symbol_table.find(identifier);
        if (it == symbol_table.end())
        {
            throw runtime_error("Семантическая ошибка - идентификатор ' " + identifier + "'не объявлен.");
        }
        return it->second;
    }

    void errBoolean() 
    {
        lex_type expr_type = expression();
        if (expr_type != LEX_BOOLEAN) 
        {
            throw runtime_error("Семантическая ошибка - Условие должно быть логическим выражением.");
        }
    }

    void errRelationOper(lex_type left, lex_type right) 
    {
        if (left != LEX_INTEGER || right != LEX_INTEGER)
        {
            throw runtime_error("Семантическая ошибка - для реляционных операторов требуются целочисленные операнды.");
        }
    }

public:
    Parser(const vector<Lex>& lex_table) : lex_table(lex_table), pos(0) {}

    void parse()
    {
        program();
        if (current().type != LEX_FIN) {
            throw runtime_error("Синтаксическая ошибка - Неожиданный токен после завершения программы.");
        }
    }

    void program()
    {
        expect(LEX_LBRACE);
        while (current().type != LEX_RBRACE) {
            if (current().type == LEX_INTEGER || current().type == LEX_REAL || current().type == LEX_BOOLEAN)
            {
                description();
            }
            else 
            {
                statement();
            }
        }
        expect(LEX_RBRACE);
    }

    void description() 
    {
        vector<string> temp;
        lex_type type = current().type;
        nextSim();
        errIdentifier(current().value);
        temp.push_back(current().value);
        expect(LEX_ID);
        while (current().type == LEX_COMMA)
        {
            nextSim();
            errIdentifier(current().value);
            temp.push_back(current().value);
            expect(LEX_ID);
        }
        for (auto identifier : temp) 
        {
            errDeclareIdentifier(identifier, type);
        }
    }

    void statement() 
    {
        if (current().type == LEX_ID) 
        {
            assignment();
        }
        else if (current().type == LEX_IF) 
        {
            conditional();
        }
        else if (current().type == LEX_FOR)
        {
            forLoop();
        }
        else if (current().type == LEX_WHILE)
        {
            whileLoop();
        }
        else if (current().type == LEX_READLN) 
        {
            inputStatement();
        }
        else if (current().type == LEX_WRITELN)
        {
            outputStatement();
        }
        else if (current().type == LEX_BEGIN) 
        {
            compoundStatement();
        }
        else 
        {
            throw runtime_error("Синтаксическая ошибка - Неожиданный токен " + string(lexType(current().type)));
        }
    }

    void assignment()
    {
        string identifier = current().value;
        errDeclaredIdentifier(identifier);
        expect(LEX_ID);
        expect(LEX_ASSIGN);
        lex_type left_type = getIdentifierType(identifier);
        lex_type right_type = expression();
        if (left_type != right_type) 
        {
            throw runtime_error("Семантическая ошибка - несоответствие типа в присвоении " + identifier + "'.");
        }
    }

    void conditional()
    {
        expect(LEX_IF);
        expect(LEX_LPAREN);
        errBoolean();
        expect(LEX_RPAREN);
        statement();
        if (current().type == LEX_ELSE) 
        {
            nextSim();
            statement();
        }
    }

    void forLoop() 
    {
        expect(LEX_FOR);
        assignment();
        expect(LEX_TO);
        errBoolean();
        if (current().type == LEX_STEP)
        {
            nextSim();
            lex_type expr_type = expression();
            if (expr_type != LEX_INTEGER) 
            {
                throw runtime_error("Семантическая ошибка - шаг цикла должен быть целым числом.");
            }
        }
        statement();
        expect(LEX_NEXT);
    }

    void whileLoop() 
    {
        expect(LEX_WHILE);
        expect(LEX_LPAREN);
        errBoolean();
        expect(LEX_RPAREN);
        statement();
    }

    lex_type expression() 
    {
        lex_type result = operand();
        while (current().type == LEX_EQ || current().type == LEX_NEQ ||
            current().type == LEX_LT || current().type == LEX_LEQ ||
            current().type == LEX_GT || current().type == LEX_GEQ) 
        {
            lex_type left = result;
            nextSim();
            lex_type right = operand();
            errRelationOper(left, right);
            result = LEX_BOOLEAN;
        }
        return result;
    }

    lex_type operand() 
    {
        lex_type result = term();
        while (current().type == LEX_PLUS || current().type == LEX_MINUS || current().type == LEX_OR)
        {
            nextSim();
            lex_type temp_result = term();
            if (temp_result != result)
            {
                result = LEX_NULL;
            }
        }
        return result;
    }

    lex_type term() 
    {
        lex_type result = factor();
        while (current().type == LEX_TIMES || current().type == LEX_DIV || current().type == LEX_AND)
        {
            nextSim();
            lex_type temp_result = factor();
            if (temp_result != result)
            {
                result = LEX_NULL;
            }
        }
        return result;
    }

    lex_type factor() 
    {
        if (current().type == LEX_ID)
        {
            string identifier = current().value;
            errDeclaredIdentifier(identifier);
            nextSim();
            return getIdentifierType(identifier);
        }
        else if (current().type == LEX_NUM) 
        {
            nextSim();
            return LEX_INTEGER;
        }
        else if (current().type == LEX_RNUM) 
        {
            nextSim();
            return LEX_REAL;
        }
        else if (current().type == LEX_LPAREN)
        {
            nextSim();
            lex_type result = expression();
            expect(LEX_RPAREN);
            return result;
        }
        else if (current().type == LEX_TRUE || current().type == LEX_FALSE)
        {
            nextSim();
            return LEX_BOOLEAN;
        }
        else if (current().type == LEX_NOT) 
        {
            nextSim();
            lex_type result = factor();
            if (result != LEX_BOOLEAN) 
            {
                throw runtime_error("Семантическая ошибка - для оператора NOT требуется логический операнд.");
            }
            return LEX_BOOLEAN;
        }
        else 
        {
            throw runtime_error("Синтаксическая ошибка - Неожиданный токен " + string(lexType(current().type)));
        }
    }

    void compoundStatement() 
    {
        expect(LEX_BEGIN);
        statement();
        while (current().type == LEX_SEMICOLON) 
        {
            nextSim();
            statement();
        }
        expect(LEX_END);
    }

    void inputStatement() {

        expect(LEX_READLN);
        errDeclaredIdentifier(current().value);
        expect(LEX_ID);
        while (current().type == LEX_COMMA) 
        {
            nextSim();
            errDeclaredIdentifier(current().value);
            expect(LEX_ID);
        }
    }

    void outputStatement() 
    {
        expect(LEX_WRITELN);
        expression();
        while (current().type == LEX_COMMA) 
        {
            nextSim();
            expression();
        }
    }
};


int main() {
    setlocale(0, "");
    try 
    {
        Lexer lexer("input.txt");
        vector<Lex> lex_table = lexer.getLexTable();

        Parser parser(lex_table);
        parser.parse();

        cout << "Парсинг прошел успешно." << endl;
    }
    catch (const exception& e) {
        cerr << "[Ошибка] " << e.what() << endl;
    }
    return 0;
}


/*
int main() {
    setlocale(0, "");
    try
    {
        Lexer lexer("input.txt");
        vector<Lex> lex_table = lexer.getLexTable();

        for (const auto& token : lex_table) 
        {
            cout << "Token: " << lex_type_description(token.type)
                << ", value: " << token.value << endl;
        }
    }
    catch (const exception& e) 
    {
        cerr << "[Ошибка] " << e.what() << endl;
    }
    return 0;
}
*/
