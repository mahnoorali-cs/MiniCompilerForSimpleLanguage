#pragma once
#include "compiler.h"
#include <vector>
#include <string>
#include <iostream>
#include <stdexcept>
using namespace std;

class Parser {
private:
   vector<Token> tokens;
    int pos;

    Token& current() { return tokens[pos]; }

    Token consume() {
        Token t = tokens[pos];
        if (t.type != "EOF_TOKEN") pos++;
        return t;
    }

    Token expect(const string& type) {
        if (current().type != type) {
            throw runtime_error(
                "[Parser] Syntax Error: expected '" + type +
                "' but got '" + current().type +
                "' (\"" + current().value + "\") at token " + std::to_string(pos)
            );
        }
        return consume();
    }

    Token expectValue(const string& value) {
        if (current().value != value) {
            throw runtime_error(
                "[Parser] Syntax Error: expected \"" + value +
                "\" but got \"" + current().value + "\""
            );
        }
        return consume();
    }

    bool isType(const string& val) {
        return (val == "int"   || val == "float"  || val == "double" ||
                val == "char"  || val == "bool"   || val == "string" ||
                val == "void"  || val == "long"   || val == "short"  ||
                val == "unsigned");
    }

    bool isRelOp(const string& val) {
        return (val == "==" || val == "!=" ||
                val == "<"  || val == ">"  ||
                val == "<=" || val == ">=");
    }

    bool isArithOp(const string& val) {
        return (val == "+" || val == "-" || val == "*" || val == "/");
    }

   string parseExpr(vector<Node>& nodes) {
       string left;
        if (current().type == "IDENTIFIER" || current().type == "NUMBER" ||
            current().type == "FLOAT_LITERAL") {
            left = consume().value;
        } else {
            throw runtime_error(
                "[Parser] Syntax Error: expected expression, got \"" +
                current().value + "\""
            );
        }

       
        if (isArithOp(current().value)) {
           string op = consume().value;
            string right;
            if (current().type == "IDENTIFIER" || current().type == "NUMBER" ||
                current().type == "FLOAT_LITERAL") {
                right = consume().value;
            } else {
                throw runtime_error("[Parser] Syntax Error: expected RHS of expression");
            }
            Node n;
            n.type  = "arith";
            n.left  = left;
            n.op    = op;
            n.right = right;
            nodes.push_back(n);
            return left + op + right; 
        }

        return left;
    }

   string parseCondition() {
     string left;
        if (current().type == "IDENTIFIER" || current().type == "NUMBER")
            left = consume().value;
        else
            throw runtime_error("[Parser] Syntax Error: expected condition LHS");

        if (!isRelOp(current().value) && !isArithOp(current().value))
            throw runtime_error("[Parser] Syntax Error: expected relational operator");

      string op = consume().value;

    string right;
        if (current().type == "IDENTIFIER" || current().type == "NUMBER")
            right = consume().value;
        else
            throw runtime_error("[Parser] Syntax Error: expected condition RHS");

        return left + " " + op + " " + right;
    }

    Node parseDeclStmt(std::vector<Node>& nodes) {
     string dtype = consume().value;         
      string varName = expect("IDENTIFIER").value;

        Node decl;
        decl.type     = "decl";
        decl.datatype = dtype;
        decl.left     = varName;

        if (current().type == "ASSIGN") {
            consume(); 
            string val = parseExpr(nodes);
            decl.right = val;
        }

        expect("SEMICOLON");
        return decl;
    }

    Node parseAssignStmt(std::vector<Node>& nodes) {
        string varName = consume().value; 
        expect("ASSIGN");

        if ((current().type == "IDENTIFIER" || current().type == "NUMBER" ||
             current().type == "FLOAT_LITERAL") &&
            pos + 1 < (int)tokens.size() && isArithOp(tokens[pos + 1].value)) {

            string lhs = consume().value;
            string op  = consume().value;
            if (current().type != "IDENTIFIER" && current().type != "NUMBER" &&
                current().type != "FLOAT_LITERAL")
                throw runtime_error("[Parser] Expected RHS operand in expression");
           string rhs = consume().value;
            expect("SEMICOLON");
            Node arith;
            arith.type     = "arith";
            arith.left     = lhs;
            arith.op       = op;
            arith.right    = rhs;
            arith.datatype = varName; 
            nodes.push_back(arith);

            Node assign;
            assign.type  = "assign";
            assign.left  = varName;
            assign.right = "__arith__"; 
            return assign;
        }


        string val = parseExpr(nodes);
        expect("SEMICOLON");

        Node n;
        n.type  = "assign";
        n.left  = varName;
        n.right = val;
        return n;
    }

    Node parseIfStmt(vector<Node>& nodes) {
        consume(); 
        expect("LPAREN");
        string cond = parseCondition();
        expect("RPAREN");
        expect("LBRACE");

        while (current().type != "RBRACE" && current().type != "EOF_TOKEN")
            parseStatement(nodes);
        expect("RBRACE");

       
        if (current().value == "else") {
            consume(); 
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

    Node parseWhileStmt(std::vector<Node>& nodes) {
        consume();
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

    Node parsePrintStmt(vector<Node>& nodes) {
        consume(); 
        if (current().type != "LSHIFT")
            throw runtime_error("[Parser] Syntax Error: expected '<<' after cout");
        consume();

        string val = parseExpr(nodes);
        expect("SEMICOLON");

        Node n;
        n.type  = "print";
        n.left  = val;
        return n;
    }

    void parseStatement(vector<Node>& nodes) {
        Token& tok = current();

        
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
            parseExpr(nodes); 
            expect("SEMICOLON");

        } else if (tok.type == "IDENTIFIER") {
            nodes.push_back(parseAssignStmt(nodes));

        } else {
           
            cout << "[Parser] Warning: skipping unexpected token \""
                      << tok.value << "\"\n";
            consume();
        }
    }

public:
    Parser(const vector<Token>& toks) : tokens(toks), pos(0) {}

    vector<Node> parse() {
   vector<Node> nodes;
        while (current().type != "EOF_TOKEN") {
            parseStatement(nodes);
        }
        return nodes;
    }
};

inline vector<Node> runParser(const vector<Token>& tokens) {
    Parser parser(tokens);
    try {
        return parser.parse();
    } catch (const exception& e) {
        cerr << e.what() << "\n";
        return {};
    }
}
