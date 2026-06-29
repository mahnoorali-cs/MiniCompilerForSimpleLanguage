#pragma once
#include <string>
#include <vector>
using namespace std;


//  1. Lexer Output 
struct Token {
    string type;   
    string value;  
};

// 2. Parser / Semantic Analyzer Output 
struct Node {
    string type;      
    string datatype;  
    string left;     
    string op;        
    string right;     
    string condition; 
};

// 3. TAC / Assembly Output
enum class TACOp {
    ASSIGN,        // result = arg1
    ADD,           // result = arg1 + arg2
    SUB,           // result = arg1 - arg2
    MUL,           // result = arg1 * arg2
    DIV,           // result = arg1 / arg2
    LABEL,         // LABEL name:
    GOTO,          // GOTO label
    IF_FALSE_GOTO, // IF_FALSE arg1 GOTO label
    PRINT          
};

struct TACInstruction {
    TACOp       op;
    std::string result; // destination / label name
    std::string arg1;   // first operand
    std::string arg2;   // second operand (may be empty)
};
