#include "Lexer.h"

#include <iostream>
#include <cctype>

using namespace std;

/*
=========================================
CONSTRUCTOR
=========================================
*/

Lexer::Lexer(string source)
{
    sourceCode = source;
    position = 0;
}

/*
KEYWORD CHECK
*/

bool Lexer::isKeyword(string word)
{
    return (

        word == "int" ||
        word == "float" ||
        word == "char" ||
        word == "bool" ||

        word == "if" ||
        word == "else" ||
        word == "while" ||
        word == "for" ||

        word == "return" ||

        word == "true" ||
        word == "false"
    );
}

/*
=========================================
TOKENIZER
=========================================
*/

vector<Token> Lexer::tokenize()
{
    vector<Token> tokens;

    while(position < sourceCode.length())
    {
        char current = sourceCode[position];

        /*
        -------------------------------------
        IGNORE SPACES
        -------------------------------------
        */

        if(isspace(current))
        {
            position++;
            continue;
        }

        /*
        -------------------------------------
        SINGLE LINE COMMENTS
        -------------------------------------
        */

        if(current == '/' &&
           position + 1 < sourceCode.length() &&
           sourceCode[position + 1] == '/')
        {
            while(position < sourceCode.length() &&
                  sourceCode[position] != '\n')
            {
                position++;
            }

            continue;
        }

        /*
        IDENTIFIER / KEYWORD
        */

        if(isalpha(current) || current == '_')
        {
            string word;

            while(position < sourceCode.length() &&
                 (isalnum(sourceCode[position]) ||
                  sourceCode[position] == '_'))
            {
                word += sourceCode[position];
                position++;
            }

            if(isKeyword(word))
            {
                tokens.push_back({"KEYWORD", word});
            }
            else
            {
                tokens.push_back({"IDENTIFIER", word});
            }

            continue;
        }

        /*
        -------------------------------------
        NUMBER
        -------------------------------------
        */

        if(isdigit(current))
        {
            string number;

            while(position < sourceCode.length() &&
                  isdigit(sourceCode[position]))
            {
                number += sourceCode[position];
                position++;
            }

            tokens.push_back({"NUMBER", number});

            continue;
        }

        /*
        -------------------------------------
        OPERATORS
        -------------------------------------
        */

        switch(current)
        {
            case '=':

                if(position + 1 < sourceCode.length() &&
                   sourceCode[position + 1] == '=')
                {
                    tokens.push_back({"EQ", "=="});
                    position += 2;
                }
                else
                {
                    tokens.push_back({"ASSIGN", "="});
                    position++;
                }

                break;

            case '+':

                tokens.push_back({"PLUS", "+"});
                position++;
                break;

            case '-':

                tokens.push_back({"MINUS", "-"});
                position++;
                break;

            case '*':

                tokens.push_back({"MULTIPLY", "*"});
                position++;
                break;

            case '/':

                tokens.push_back({"DIVIDE", "/"});
                position++;
                break;

            case '<':

                if(position + 1 < sourceCode.length() &&
                   sourceCode[position + 1] == '=')
                {
                    tokens.push_back({"LTE", "<="});
                    position += 2;
                }
                else
                {
                    tokens.push_back({"LT", "<"});
                    position++;
                }

                break;

            case '>':

                if(position + 1 < sourceCode.length() &&
                   sourceCode[position + 1] == '=')
                {
                    tokens.push_back({"GTE", ">="});
                    position += 2;
                }
                else
                {
                    tokens.push_back({"GT", ">"});
                    position++;
                }

                break;

            case '!':

                if(position + 1 < sourceCode.length() &&
                   sourceCode[position + 1] == '=')
                {
                    tokens.push_back({"NEQ", "!="});
                    position += 2;
                }
                else
                {
                    cout << "Lexer Error: Unexpected '!'\n";
                    position++;
                }

                break;

            /*
            -------------------------------------
            DELIMITERS
            -------------------------------------
            */

            case ';':

                tokens.push_back({"SEMICOLON", ";"});
                position++;
                break;

            case ',':

                tokens.push_back({"COMMA", ","});
                position++;
                break;

            case '(':

                tokens.push_back({"LPAREN", "("});
                position++;
                break;

            case ')':

                tokens.push_back({"RPAREN", ")"});
                position++;
                break;

            case '{':

                tokens.push_back({"LBRACE", "{"});
                position++;
                break;

            case '}':

                tokens.push_back({"RBRACE", "}"});
                position++;
                break;

            default:

                cout << "Lexer Error: Unknown Character -> "
                     << current << endl;

                position++;
        }
    }

    tokens.push_back({"EOF", ""});

    return tokens;
}
