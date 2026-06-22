#ifndef COMMON_H
#define COMMON_H

#include <string>
#include <vector>

using namespace std;

/*
TOKEN STRUCTURE
*/

struct Token
{
    string type;
    string value;
};

/*
AST NODE STRUCTURE
*/

struct ASTNode
{
    string nodeType;

    string value;

    ASTNode* left;
    ASTNode* right;

    ASTNode(string type = "", string val = "")
    {
        nodeType = type;
        value = val;
        left = nullptr;
        right = nullptr;
    }
};

#endif
