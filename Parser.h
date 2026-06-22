#ifndef PARSER_H
#define PARSER_H

#include "Common.h"

class Parser
{
private:

    vector<Token> tokens;
    int current;

    Token peek();
    Token advance();
    bool match(string type);

    ASTNode* parseDeclaration();
    ASTNode* parseAssignment();
    ASTNode* parseExpression();

public:

    Parser(vector<Token> tokenList);

    ASTNode* parse();
};

#endif
