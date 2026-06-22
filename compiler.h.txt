#pragma once
#include <string>
#include <vector>

// ================================================================
//  SHARED DATA STRUCTURES  —  include this header in ALL files
// ================================================================

// ── 1. Lexer Output ─────────────────────────────────────────────
struct Token {
    std::string type;   // e.g. "KEYWORD", "IDENTIFIER", "NUMBER"
    std::string value;  // e.g. "int", "x", "42"
};

// ── 2. Parser / Semantic Analyzer Output ────────────────────────
struct Node {
    std::string type;      // "decl" | "assign" | "arith" | "if" | "while" | "print"
    std::string datatype;  // "int" | "float" | "bool" | etc.  (used in decl)
    std::string left;      // LHS variable name
    std::string op;        // operator: "+", "-", "*", "/", ">", "<", "=="
    std::string right;     // RHS value or variable name
    std::string condition; // used in "if" / "while" nodes
};

// ── 3. TAC / Assembly Output ─────────────────────────────────────
enum class TACOp {
    ASSIGN,        // result = arg1
    ADD,           // result = arg1 + arg2
    SUB,           // result = arg1 - arg2
    MUL,           // result = arg1 * arg2
    DIV,           // result = arg1 / arg2
    LABEL,         // LABEL name:
    GOTO,          // GOTO label
    IF_FALSE_GOTO, // IF_FALSE arg1 GOTO label
    PRINT          // PRINT arg1
};

struct TACInstruction {
    TACOp       op;
    std::string result; // destination / label name
    std::string arg1;   // first operand
    std::string arg2;   // second operand (may be empty)
};
