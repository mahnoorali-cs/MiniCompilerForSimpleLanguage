#ifndef LEXER_H
#define LEXER_H

#include "common.h"

class Lexer
{
private:

    string sourceCode;

    int position;

    bool isKeyword(string word);

public:

    Lexer(string source);

    vector<Token> tokenize();
};

#endif
