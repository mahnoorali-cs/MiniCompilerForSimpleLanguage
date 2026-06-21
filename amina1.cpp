#include <iostream>
#include <vector>
#include <string>
#include <cctype>
using namespace std;

struct Token {
    string type;
    string value;
};

class Lexer {
private:
    string src;
    int pos;

    bool isKeyword(string word) {
        return (
            word == "int"      || word == "float"    ||
            word == "double"   || word == "char"     ||
            word == "bool"     || word == "void"     ||
            word == "string"   || word == "long"     ||
            word == "short"    || word == "unsigned" ||
            word == "if"       || word == "else"     ||
            word == "while"    || word == "for"      ||
            word == "do"       || word == "break"    ||
            word == "continue" || word == "switch"   ||
            word == "case"     || word == "default"  ||
            word == "return"   || word == "goto"     ||
            word == "cout"     || word == "cin"      ||
            word == "endl"     || word == "print"    ||
            word == "true"     || word == "false"    ||
            word == "using"    || word == "namespace"||
            word == "const"    || word == "static"   ||
            word == "sizeof"   || word == "typedef"  ||
            word == "extern"   || word == "auto"     ||
            word == "null"     || word == "nullptr"
        );
    }

public:
    Lexer(string source) {
        src = source;
        pos = 0;
    }

    vector<Token> tokenize() {
        vector<Token> tokens;

        while (pos < src.size()) {
            char ch = src[pos];

            if (isspace(ch)) { pos++; continue; }

            if (ch == '/' && pos + 1 < src.size() && src[pos+1] == '/') {
                while (pos < src.size() && src[pos] != '\n') pos++;
                continue;
            }

            if (isalpha(ch)) {
                string word = "";
                while (pos < src.size() && (isalnum(src[pos]) || src[pos] == '_'))
                    word += src[pos++];
                if (isKeyword(word))
                    tokens.push_back({"KEYWORD", word});
                else
                    tokens.push_back({"IDENTIFIER", word});
                continue;
            }

            if (isdigit(ch)) {
                string num = "";
                while (pos < src.size() && isdigit(src[pos]))
                    num += src[pos++];
                tokens.push_back({"NUMBER", num});
                continue;
            }

            switch (ch) {
                case '=':
                    if (pos + 1 < src.size() && src[pos+1] == '=') {
                        tokens.push_back({"EQ_OP", "=="}); pos += 2;
                    } else {
                        tokens.push_back({"ASSIGN", "="}); pos++;
                    }
                    break;
                case '+': tokens.push_back({"PLUS",     "+"}); pos++; break;
                case '-': tokens.push_back({"MINUS",    "-"}); pos++; break;
                case '*': tokens.push_back({"MULTIPLY", "*"}); pos++; break;
                case '/': tokens.push_back({"DIVIDE",   "/"}); pos++; break;
                case '<':
                    if (pos + 1 < src.size() && src[pos+1] == '<') {
                        tokens.push_back({"LSHIFT", "<<"}); pos += 2;
                    } else if (pos + 1 < src.size() && src[pos+1] == '=') {
                        tokens.push_back({"LTE", "<="}); pos += 2;
                    } else {
                        tokens.push_back({"LT", "<"}); pos++;
                    }
                    break;
                case '>':
                    if (pos + 1 < src.size() && src[pos+1] == '>') {
                        tokens.push_back({"RSHIFT", ">>"}); pos += 2;
                    } else if (pos + 1 < src.size() && src[pos+1] == '=') {
                        tokens.push_back({"GTE", ">="}); pos += 2;
                    } else {
                        tokens.push_back({"GT", ">"}); pos++;
                    }
                    break;
                case '!':
                    if (pos + 1 < src.size() && src[pos+1] == '=') {
                        tokens.push_back({"NEQ", "!="}); pos += 2;
                    } else {
                        tokens.push_back({"UNKNOWN", "!"}); pos++;
                    }
                    break;
                case ';': tokens.push_back({"SEMICOLON", ";"}); pos++; break;
                case '(': tokens.push_back({"LPAREN",    "("}); pos++; break;
                case ')': tokens.push_back({"RPAREN",    ")"}); pos++; break;
                case '{': tokens.push_back({"LBRACE",    "{"}); pos++; break;
                case '}': tokens.push_back({"RBRACE",    "}"}); pos++; break;
                default:
                    cout << "Unknown character: " << ch << "\n";
                    pos++;
                    break;
            }
        }

        tokens.push_back({"EOF", ""});
        return tokens;
    }
};

int main() {
    string sourceCode = R"(
        int x;
        float y;
        bool flag;
        const int z;
        x = 10;
        if(x > 5)
        {
            cout << x;
        }
        else
        {
            return 0;
        }
    )";

    Lexer lexer(sourceCode);
    vector<Token> tokens = lexer.tokenize();

    cout << "===== TOKENS =====\n";
    for (auto& tok : tokens) {
        cout << tok.type << "(" << tok.value << ")\n";
    }

    return 0;
}