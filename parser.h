#pragma once
#include "compiler.h"
#include <vector>
#include <string>
#include <iostream>
#include <stdexcept>
//This is aqeela's code
// ================================================================
//  PARSER (Syntax Analyzer)  —  Member 2
//  Input : vector<Token>  from Lexer
//  Output: vector<Node>   consumed by Semantic Analyzer
//
//  Supported grammar:
//    program      → statement*
//    statement    → declStmt | assignStmt | ifStmt
//                 | whileStmt | printStmt
//    declStmt     → TYPE IDENTIFIER [ '=' expr ] ';'
//    assignStmt   → IDENTIFIER '=' expr ';'
//    ifStmt       → 'if' '(' condition ')' '{' statement* '}'
//                   [ 'else' '{' statement* '}' ]
//    whileStmt    → 'while' '(' condition ')' '{' statement* '}'
//    printStmt    → 'cout' '<<' expr ';'
//    condition    → expr ( '==' | '!=' | '<' | '>' | '<=' | '>=' ) expr
//    expr         → IDENTIFIER | NUMBER | FLOAT_LITERAL
//                 | expr ( '+' | '-' | '*' | '/' ) expr   (simplified)
// ================================================================

class Parser {
private:
    std::vector<Token> tokens;
    int pos;

    // ── Token helpers ────────────────────────────────────────────
    Token& current() { return tokens[pos]; }

    Token consume() {
        Token t = tokens[pos];
        if (t.type != "EOF_TOKEN") pos++;
        return t;
    }

    Token expect(const std::string& type) {
        if (current().type != type) {
            throw std::runtime_error(
                "[Parser] Syntax Error: expected '" + type +
                "' but got '" + current().type +
                "' (\"" + current().value + "\") at token " + std::to_string(pos)
            );
        }
        return consume();
    }

    Token expectValue(const std::string& value) {
        if (current().value != value) {
            throw std::runtime_error(
                "[Parser] Syntax Error: expected \"" + value +
                "\" but got \"" + current().value + "\""
            );
        }
        return consume();
    }

    bool isType(const std::string& val) {
        return (val == "int"   || val == "float"  || val == "double" ||
                val == "char"  || val == "bool"   || val == "string" ||
                val == "void"  || val == "long"   || val == "short"  ||
                val == "unsigned");
    }

    bool isRelOp(const std::string& val) {
        return (val == "==" || val == "!=" ||
                val == "<"  || val == ">"  ||
                val == "<=" || val == ">=");
    }

    bool isArithOp(const std::string& val) {
        return (val == "+" || val == "-" || val == "*" || val == "/");
    }

    // ── Expression parser ────────────────────────────────────────
    // Returns a single value string (var name or literal).
    // For binary expressions, emits an "arith" node and returns a temp.
    std::string parseExpr(std::vector<Node>& nodes) {
        // Left operand: IDENTIFIER or NUMBER/FLOAT
        std::string left;
        if (current().type == "IDENTIFIER" || current().type == "NUMBER" ||
            current().type == "FLOAT_LITERAL") {
            left = consume().value;
        } else {
            throw std::runtime_error(
                "[Parser] Syntax Error: expected expression, got \"" +
                current().value + "\""
            );
        }

        // Optional: binary operator
        if (isArithOp(current().value)) {
            std::string op = consume().value;
            std::string right;
            if (current().type == "IDENTIFIER" || current().type == "NUMBER" ||
                current().type == "FLOAT_LITERAL") {
                right = consume().value;
            } else {
                throw std::runtime_error("[Parser] Syntax Error: expected RHS of expression");
            }
            Node n;
            n.type  = "arith";
            n.left  = left;
            n.op    = op;
            n.right = right;
            nodes.push_back(n);
            return left + op + right; // synthetic "result" description
        }

        return left;
    }

    // ── Condition parser ─────────────────────────────────────────
    // Returns a string like "x > 5" and emits nothing itself.
    std::string parseCondition() {
        std::string left;
        if (current().type == "IDENTIFIER" || current().type == "NUMBER")
            left = consume().value;
        else
            throw std::runtime_error("[Parser] Syntax Error: expected condition LHS");

        if (!isRelOp(current().value) && !isArithOp(current().value))
            throw std::runtime_error("[Parser] Syntax Error: expected relational operator");

        std::string op = consume().value;

        std::string right;
        if (current().type == "IDENTIFIER" || current().type == "NUMBER")
            right = consume().value;
        else
            throw std::runtime_error("[Parser] Syntax Error: expected condition RHS");

        return left + " " + op + " " + right;
    }

    // ── Statement parsers ─────────────────────────────────────────

    // TYPE IDENTIFIER [ '=' expr ] ';'
    Node parseDeclStmt(std::vector<Node>& nodes) {
        std::string dtype = consume().value;          // consume type keyword
        std::string varName = expect("IDENTIFIER").value;

        Node decl;
        decl.type     = "decl";
        decl.datatype = dtype;
        decl.left     = varName;

        if (current().type == "ASSIGN") {
            consume(); // consume '='
            std::string val = parseExpr(nodes);
            decl.right = val;
        }

        expect("SEMICOLON");
        return decl;
    }

    // IDENTIFIER '=' expr ';'
    // For binary RHS (x = a + b), we emit:
    //   arith node  { left=a, op=+, right=b, result=varName }
    //   assign node { left=varName, right="__arith__" }  ← sentinel
    // The TAC generator handles "__arith__" by using the last temp.
    Node parseAssignStmt(std::vector<Node>& nodes) {
        std::string varName = consume().value; // consume IDENTIFIER
        expect("ASSIGN");

        // Peek: is this a binary expression?
        if ((current().type == "IDENTIFIER" || current().type == "NUMBER" ||
             current().type == "FLOAT_LITERAL") &&
            pos + 1 < (int)tokens.size() && isArithOp(tokens[pos + 1].value)) {

            std::string lhs = consume().value;
            std::string op  = consume().value;
            if (current().type != "IDENTIFIER" && current().type != "NUMBER" &&
                current().type != "FLOAT_LITERAL")
                throw std::runtime_error("[Parser] Expected RHS operand in expression");
            std::string rhs = consume().value;
            expect("SEMICOLON");

            // Arith node carries the destination variable so TAC can write directly
            Node arith;
            arith.type     = "arith";
            arith.left     = lhs;
            arith.op       = op;
            arith.right    = rhs;
            arith.datatype = varName; // destination for the arith result
            nodes.push_back(arith);

            // Assign node signals the variable gets the arith result
            Node assign;
            assign.type  = "assign";
            assign.left  = varName;
            assign.right = "__arith__"; // sentinel: use last temp
            return assign;
        }

        // Simple assignment: x = value;
        std::string val = parseExpr(nodes);
        expect("SEMICOLON");

        Node n;
        n.type  = "assign";
        n.left  = varName;
        n.right = val;
        return n;
    }

    // 'if' '(' condition ')' '{' stmts '}' [ 'else' '{' stmts '}' ]
    Node parseIfStmt(std::vector<Node>& nodes) {
        consume(); // consume 'if'
        expect("LPAREN");
        std::string cond = parseCondition();
        expect("RPAREN");
        expect("LBRACE");

        // Parse body — adds body nodes directly to the output list
        while (current().type != "RBRACE" && current().type != "EOF_TOKEN")
            parseStatement(nodes);
        expect("RBRACE");

        // Optional else
        if (current().value == "else") {
            consume(); // consume 'else'
            expect("LBRACE");
            while (current().type != "RBRACE" && current().type != "EOF_TOKEN")
                parseStatement(nodes);
            expect("RBRACE");
        }

        Node n;
        n.type      = "if";
        n.condition = cond;
        return n;
    }

    // 'while' '(' condition ')' '{' stmts '}'
    Node parseWhileStmt(std::vector<Node>& nodes) {
        consume(); // consume 'while'
        expect("LPAREN");
        std::string cond = parseCondition();
        expect("RPAREN");
        expect("LBRACE");

        while (current().type != "RBRACE" && current().type != "EOF_TOKEN")
            parseStatement(nodes);
        expect("RBRACE");

        Node n;
        n.type      = "while";
        n.condition = cond;
        return n;
    }

    // 'cout' '<<' expr ';'
    Node parsePrintStmt(std::vector<Node>& nodes) {
        consume(); // consume 'cout'
        // expect '<<'
        if (current().type != "LSHIFT")
            throw std::runtime_error("[Parser] Syntax Error: expected '<<' after cout");
        consume();

        std::string val = parseExpr(nodes);
        expect("SEMICOLON");

        Node n;
        n.type  = "print";
        n.left  = val;
        return n;
    }

    // ── Dispatch one statement ────────────────────────────────────
    void parseStatement(std::vector<Node>& nodes) {
        Token& tok = current();

        // Declaration: starts with a type keyword
        if (tok.type == "KEYWORD" && isType(tok.value)) {
            nodes.push_back(parseDeclStmt(nodes));

        } else if (tok.value == "if") {
            nodes.push_back(parseIfStmt(nodes));

        } else if (tok.value == "while") {
            nodes.push_back(parseWhileStmt(nodes));

        } else if (tok.value == "cout") {
            nodes.push_back(parsePrintStmt(nodes));

        } else if (tok.value == "return") {
            consume();
            parseExpr(nodes); // parse (and discard) return value
            expect("SEMICOLON");

        } else if (tok.type == "IDENTIFIER") {
            nodes.push_back(parseAssignStmt(nodes));

        } else {
            // Skip unknown token to avoid infinite loop
            std::cout << "[Parser] Warning: skipping unexpected token \""
                      << tok.value << "\"\n";
            consume();
        }
    }

public:
    Parser(const std::vector<Token>& toks) : tokens(toks), pos(0) {}

    std::vector<Node> parse() {
        std::vector<Node> nodes;
        while (current().type != "EOF_TOKEN") {
            parseStatement(nodes);
        }
        return nodes;
    }
};

// ── Public entry point ────────────────────────────────────────────
inline std::vector<Node> runParser(const std::vector<Token>& tokens) {
    Parser parser(tokens);
    try {
        return parser.parse();
    } catch (const std::exception& e) {
        std::cerr << e.what() << "\n";
        return {};
    }
}
