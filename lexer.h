#pragma once
#include "compiler.h"
#include <string>
#include <vector>
#include <cctype>
#include <iostream>

// ================================================================
//  LEXER  —  Member 1
//  Input : raw source code string
//  Output: vector<Token>
// ================================================================

class Lexer {
private:
    std::string src;
    int pos;

    bool isKeyword(const std::string& word) {
        return (
            word == "int"    || word == "float"  || word == "double" ||
            word == "char"   || word == "bool"   || word == "void"   ||
            word == "string" || word == "long"   || word == "short"  ||
            word == "unsigned"|| word == "const" || word == "static" ||
            word == "if"     || word == "else"   || word == "while"  ||
            word == "for"    || word == "do"     || word == "break"  ||
            word == "continue"|| word == "switch"|| word == "case"   ||
            word == "default"|| word == "return" || word == "goto"   ||
            word == "cout"   || word == "cin"    || word == "endl"   ||
            word == "print"  || word == "true"   || word == "false"  ||
            word == "using"  || word == "namespace"                  ||
            word == "sizeof" || word == "typedef"|| word == "extern" ||
            word == "auto"   || word == "null"   || word == "nullptr"
        );
    }

public:
    Lexer(const std::string& source) : src(source), pos(0) {}

    std::vector<Token> tokenize() {
        std::vector<Token> tokens;

        while (pos < (int)src.size()) {
            char ch = src[pos];

            // ── Whitespace ───────────────────────────────────────
            if (isspace(ch)) { pos++; continue; }

            // ── Single-line comment ──────────────────────────────
            if (ch == '/' && pos + 1 < (int)src.size() && src[pos+1] == '/') {
                while (pos < (int)src.size() && src[pos] != '\n') pos++;
                continue;
            }

            // ── Identifiers / Keywords ───────────────────────────
            if (isalpha(ch) || ch == '_') {
                std::string word;
                while (pos < (int)src.size() && (isalnum(src[pos]) || src[pos] == '_'))
                    word += src[pos++];
                if (isKeyword(word))
                    tokens.push_back({"KEYWORD", word});
                else
                    tokens.push_back({"IDENTIFIER", word});
                continue;
            }

            // ── Numbers (int + float) ────────────────────────────
            if (isdigit(ch)) {
                std::string num;
                while (pos < (int)src.size() && isdigit(src[pos]))
                    num += src[pos++];
                if (pos < (int)src.size() && src[pos] == '.') {
                    num += src[pos++];
                    while (pos < (int)src.size() && isdigit(src[pos]))
                        num += src[pos++];
                    tokens.push_back({"FLOAT_LITERAL", num});
                } else {
                    tokens.push_back({"NUMBER", num});
                }
                continue;
            }

            // ── String literals ──────────────────────────────────
            if (ch == '"') {
                std::string str;
                pos++; // skip opening "
                while (pos < (int)src.size() && src[pos] != '"')
                    str += src[pos++];
                pos++; // skip closing "
                tokens.push_back({"STRING_LITERAL", str});
                continue;
            }

            // ── Char literals ────────────────────────────────────
            if (ch == '\'') {
                pos++; // skip opening '
                std::string c(1, src[pos++]);
                pos++; // skip closing '
                tokens.push_back({"CHAR_LITERAL", c});
                continue;
            }

            // ── Operators & Punctuation ──────────────────────────
            switch (ch) {
                case '=':
                    if (pos+1 < (int)src.size() && src[pos+1] == '=')
                        { tokens.push_back({"EQ_OP", "=="}); pos += 2; }
                    else
                        { tokens.push_back({"ASSIGN", "="}); pos++; }
                    break;

                case '+':
                    if (pos+1 < (int)src.size() && src[pos+1] == '+')
                        { tokens.push_back({"INC", "++"}); pos += 2; }
                    else
                        { tokens.push_back({"PLUS", "+"}); pos++; }
                    break;

                case '-':
                    if (pos+1 < (int)src.size() && src[pos+1] == '-')
                        { tokens.push_back({"DEC", "--"}); pos += 2; }
                    else
                        { tokens.push_back({"MINUS", "-"}); pos++; }
                    break;

                case '*': tokens.push_back({"MULTIPLY", "*"}); pos++; break;

                case '/': tokens.push_back({"DIVIDE",   "/"}); pos++; break;

                case '%': tokens.push_back({"MODULO",   "%"}); pos++; break;

                case '<':
                    if (pos+1 < (int)src.size() && src[pos+1] == '<')
                        { tokens.push_back({"LSHIFT", "<<"}); pos += 2; }
                    else if (pos+1 < (int)src.size() && src[pos+1] == '=')
                        { tokens.push_back({"LTE", "<="}); pos += 2; }
                    else
                        { tokens.push_back({"LT", "<"}); pos++; }
                    break;

                case '>':
                    if (pos+1 < (int)src.size() && src[pos+1] == '>')
                        { tokens.push_back({"RSHIFT", ">>"}); pos += 2; }
                    else if (pos+1 < (int)src.size() && src[pos+1] == '=')
                        { tokens.push_back({"GTE", ">="}); pos += 2; }
                    else
                        { tokens.push_back({"GT", ">"}); pos++; }
                    break;

                case '!':
                    if (pos+1 < (int)src.size() && src[pos+1] == '=')
                        { tokens.push_back({"NEQ", "!="}); pos += 2; }
                    else
                        { tokens.push_back({"NOT", "!"}); pos++; }
                    break;

                case '&':
                    if (pos+1 < (int)src.size() && src[pos+1] == '&')
                        { tokens.push_back({"AND", "&&"}); pos += 2; }
                    else
                        { tokens.push_back({"BITAND", "&"}); pos++; }
                    break;

                case '|':
                    if (pos+1 < (int)src.size() && src[pos+1] == '|')
                        { tokens.push_back({"OR", "||"}); pos += 2; }
                    else
                        { tokens.push_back({"BITOR", "|"}); pos++; }
                    break;

                case ';': tokens.push_back({"SEMICOLON", ";"}); pos++; break;
                case ',': tokens.push_back({"COMMA",     ","}); pos++; break;
                case '(': tokens.push_back({"LPAREN",    "("}); pos++; break;
                case ')': tokens.push_back({"RPAREN",    ")"}); pos++; break;
                case '{': tokens.push_back({"LBRACE",    "{"}); pos++; break;
                case '}': tokens.push_back({"RBRACE",    "}"}); pos++; break;
                case '[': tokens.push_back({"LBRACKET",  "["}); pos++; break;
                case ']': tokens.push_back({"RBRACKET",  "]"}); pos++; break;

                default:
                    std::cout << "[Lexer] Unknown character: '" << ch << "'\n";
                    pos++;
                    break;
            }
        }

        tokens.push_back({"EOF_TOKEN", ""});
        return tokens;
    }
};

// ── Public entry point ───────────────────────────────────────────
inline std::vector<Token> runLexer(const std::string& sourceCode) {
    Lexer lexer(sourceCode);
    return lexer.tokenize();
}
