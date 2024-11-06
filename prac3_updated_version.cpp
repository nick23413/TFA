#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <unordered_set>
#include <cctype>

using namespace std;

const unordered_set<string> KEYWORDS = {
    "if", "else", "for", "to", "step", "next", "while",
    "readln", "writeln", "true", "false", "%", "!", "$"
};

enum class State { H, ID, NM, ASGN, DLM, ERR, COMM };
enum class TokenType { KWORD, IDENT, NUM, OPER, DELIM };

struct Token {
    TokenType type;
    string value;
};

class LexemeTable {
    vector<Token> tokens;

public:
    void add(const Token& token) {
        tokens.push_back(token);
    }

    void display() const {
        for (const auto& token : tokens) {
            cout << "Token type: ";
            switch (token.type) {
                case TokenType::KWORD: cout << "KWORD"; break;
                case TokenType::IDENT: cout << "IDENT"; break;
                case TokenType::NUM: cout << "NUM"; break;
                case TokenType::OPER: cout << "OPER"; break;
                case TokenType::DELIM: cout << "DELIM"; break;
            }
            cout << ", value: " << token.value << '\n';
        }
    }
};

class Lexer {
    string filename;
    LexemeTable table;

    bool isKeyword(const string& id) const {
        return KEYWORDS.find(id) != KEYWORDS.end();
    }

    bool isNum(const string& input) const {
        size_t i = 0, len = input.length();
        if (input.back() == 'B' || input.back() == 'b') {
            for (i = 0; i < len - 1; i++) if (input[i] != '0' && input[i] != '1') return false;
            return true;
        } else if (input.back() == 'O' || input.back() == 'o') {
            for (i = 0; i < len - 1; i++) if (input[i] < '0' || input[i] > '7') return false;
            return true;
        } else if (input.back() == 'D' || input.back() == 'd') {
            for (i = 0; i < len - 1; i++) if (!isdigit(input[i])) return false;
            return true;
        } else if (input.back() == 'H' || input.back() == 'h') {
            for (i = 0; i < len - 1; i++) if (!isdigit(input[i]) && (input[i] < 'A' || input[i] > 'F') && (input[i] < 'a' || input[i] > 'f')) return false;
            return true;
        }
        i = 0;
        while (i < len && isdigit(input[i])) i++;
        if (len == i) return true;
        if (i < len && input[i] == '.') {
            bool hasDigits = false; i++;
            while (i < len && isdigit(input[i])) { i++; hasDigits = true; }
            if (!hasDigits) return false;
        }
        if (i < len && (input[i] == 'E' || input[i] == 'e')) {
            i++;
            if (i < len && (input[i] == '+' || input[i] == '-')) i++;
            bool hasExpDigits = false;
            while (i < len && isdigit(input[i])) { i++; hasExpDigits = true; }
            if (!hasExpDigits) return false;
        }
        return i == len;
    }

public:
    Lexer(const string& name) : filename(name) {}

    void process() {
        ifstream file(filename);
        if (!file.is_open()) {
            cout << "Cannot open file " << filename << ".\n";
            return;
        }

        char c;
        State state = State::H;
        Token token;

        while (file.get(c)) {
            switch (state) {
                case State::H:
                    if (isspace(c)) continue;
                    else if (isalpha(c)) { state = State::ID; token.value += c; }
                    else if (isdigit(c) || c == '.') { state = State::NM; token.value += c; }
                    else if (c == ':') state = State::ASGN;
                    else if (c == '%' || c == '!' || c == '$') { token.type = TokenType::KWORD; token.value = c; table.add(token); token.value.clear(); }
                    else { state = State::DLM; file.putback(c); }
                    break;

                case State::ID:
                    if (isalnum(c)) token.value += c;
                    else { token.type = isKeyword(token.value) ? TokenType::KWORD : TokenType::IDENT; table.add(token); token.value.clear(); state = State::H; file.putback(c); }
                    break;

                case State::NM:
                    if (isalnum(c) || c == '.' || c == '+' || c == '-') token.value += c;
                    else if (isNum(token.value)) { token.type = TokenType::NUM; table.add(token); token.value.clear(); state = State::H; file.putback(c); }
                    else { cout << "Wrong num: " << token.value << '\n'; token.value.clear(); state = State::ERR; }
                    break;

                case State::ASGN:
                    if (c == '=') { token.type = TokenType::OPER; token.value = ":="; table.add(token); token.value.clear(); state = State::H; }
                    else { cout << "Unknown character: " << c << '\n'; state = State::ERR; }
                    break;

                case State::DLM:
                    if (c == '(' || c == ')' || c == ';' || c == '{' || c == '}' || c == ',') { token.type = TokenType::DELIM; token.value = c; table.add(token); token.value.clear(); state = State::H; }
                    else if (c == '<' || c == '>' || c == '=' || c == '!' || c == '*' || c == '/' || c == '&' || c == '+' || c == '-' || c == '|') token.value += c;
                    else { if (token.value == "/*") state = State::COMM; else { token.type = TokenType::OPER; table.add(token); token.value.clear(); state = State::H; } }
                    break;

                case State::COMM:
                    token.value += c;
                    if (token.value.length() > 2 && token.value[token.value.length() - 3] == '*' && token.value[token.value.length() - 2] == '/') { token.value.clear(); state = State::H; }
                    break;

                case State::ERR:
                    state = State::H; file.putback(c);
                    break;
            }
        }
    }

    void show() const {
        table.display();
    }
};

int main() {
    Lexer lexer("input.txt");
    lexer.process();
    lexer.show();
    return 0;
}
