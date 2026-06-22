#include <iostream>
#include <string>
#include <cctype>
#include <vector>
#include <map>
#include <stack>
using namespace std;

/*
=====================================================
      SYMBOL TABLE ENTRY STRUCTURE
=====================================================
*/
struct Symbol
{
    string name;
    string type;
    int scopeLevel;
};

/*
=====================================================
      SYMBOL TABLE CLASS
=====================================================
*/
class SymbolTable
{
private:
    vector<Symbol> table;
    int currentScope;

public:
    SymbolTable()
    {
        currentScope = 0;
    }

    // Enter new scope (for if/while blocks later)
    void enterScope()
    {
        currentScope++;
    }

    // Exit scope
    void exitScope()
    {
        for (int i = table.size() - 1; i >= 0; i--)
        {
            if (table[i].scopeLevel == currentScope)
            {
                table.erase(table.begin() + i);
            }
        }
        currentScope--;
    }

    /*
    -----------------------------------------------------
    ADD VARIABLE (DECLARATION CHECK + DUPLICATE CHECK)
    -----------------------------------------------------
    */
    bool addSymbol(string name, string type)
    {
        // Check duplicate in same scope
        for (auto &s : table)
        {
            if (s.name == name && s.scopeLevel == currentScope)
            {
                cout << "Semantic Error: Duplicate declaration of variable '"
                     << name << "' in same scope\n";
                return false;
            }
        }

        Symbol s;
        s.name = name;
        s.type = type;
        s.scopeLevel = currentScope;
        table.push_back(s);

        return true;
    }

    /*
    -----------------------------------------------------
    CHECK IF VARIABLE EXISTS (DECLARATION CHECK)
    -----------------------------------------------------
    */
    bool isDeclared(string name)
    {
        for (auto it = table.rbegin(); it != table.rend(); it++)
        {
            if (it->name == name)
                return true;
        }
        return false;
    }

    /*
    -----------------------------------------------------
    GET TYPE OF VARIABLE
    -----------------------------------------------------
    */
    string getType(string name)
    {
        for (auto it = table.rbegin(); it != table.rend(); it++)
        {
            if (it->name == name)
                return it->type;
        }
        return "";
    }

    /*
    -----------------------------------------------------
    DISPLAY SYMBOL TABLE
    -----------------------------------------------------
    */
    void printTable()
    {
        cout << "\n====== SYMBOL TABLE ======\n";
        for (auto &s : table)
        {
            cout << "Name: " << s.name
                 << " | Type: " << s.type
                 << " | Scope: " << s.scopeLevel << endl;
        }
        cout << "==========================\n";
    }
};

/*
=====================================================
      SIMPLE AST NODE STRUCTURE (SIMPLIFIED)
=====================================================
*/
struct Node
{
    string type;   // "decl", "assign", "arith"
    string left;   // variable name
    string op;     // operator
    string right;  // value or variable
};

/*
=====================================================
      SEMANTIC ANALYZER CLASS
=====================================================
*/
class SemanticAnalyzer
{
private:
    SymbolTable sym;

public:

    /*
    -----------------------------------------------------
    VARIABLE DECLARATION HANDLING
    -----------------------------------------------------
    */
    void handleDeclaration(string varName)
    {
        sym.addSymbol(varName, "int");
    }

    /*
    -----------------------------------------------------
    ASSIGNMENT CHECK
    -----------------------------------------------------
    */
    void handleAssignment(string varName, string value)
    {
        // Check if LHS variable exists
        if (!sym.isDeclared(varName))
        {
            cout << "Semantic Error: Variable '"
                 << varName << "' not declared\n";
            return;
        }

        // If RHS is variable, check it too
        if (isalpha(value[0]))
        {
            if (!sym.isDeclared(value))
            {
                cout << "Semantic Error: Variable '"
                     << value << "' not declared\n";
                return;
            }

            // Type check
            if (sym.getType(varName) != sym.getType(value))
            {
                cout << "Type Error: Cannot assign different types\n";
                return;
            }
        }

        cout << "Assignment OK: " << varName << " = " << value << endl;
    }

    /*
    -----------------------------------------------------
    ARITHMETIC CHECK
    -----------------------------------------------------
    */
    void handleArithmetic(string left, string right)
    {
        if (isalpha(left[0]) && !sym.isDeclared(left))
        {
            cout << "Semantic Error: Variable '" << left << "' not declared\n";
            return;
        }

        if (isalpha(right[0]) && !sym.isDeclared(right))
        {
            cout << "Semantic Error: Variable '" << right << "' not declared\n";
            return;
        }

        cout << "Arithmetic expression OK: " << left << " + " << right << endl;
    }

    /*
    -----------------------------------------------------
    PROCESS AST NODE
    -----------------------------------------------------
    */
    void processNode(Node node)
    {
        if (node.type == "decl")
        {
            handleDeclaration(node.left);
        }
        else if (node.type == "assign")
        {
            handleAssignment(node.left, node.right);
        }
        else if (node.type == "arith")
        {
            handleArithmetic(node.left, node.right);
        }
    }

    void printSymbolTable()
    {
        sym.printTable();
    }
};

/*
=====================================================
      MAIN DRIVER PROGRAM (TEST CASES)
=====================================================
*/
int main()
{
    SemanticAnalyzer analyzer;

    cout << "===== SEMANTIC ANALYZER TEST =====\n\n";

    /*
    TEST CASE 1: VALID DECLARATION
    */
    Node n1 = {"decl", "x", "", ""};
    analyzer.processNode(n1);

    /*
    TEST CASE 2: VALID ASSIGNMENT
    */
    Node n2 = {"assign", "x", "", "10"};
    analyzer.processNode(n2);

    /*
    TEST CASE 3: UNDECLARED VARIABLE ERROR
    */
    Node n3 = {"assign", "y", "", "5"};
    analyzer.processNode(n3);

    /*
    TEST CASE 4: DUPLICATE DECLARATION
    */
    Node n4 = {"decl", "x", "", ""};
    analyzer.processNode(n4);

    /*
    TEST CASE 5: ARITHMETIC CHECK
    */
    Node n5 = {"arith", "x", "+", "10"};
    analyzer.processNode(n5);

    /*
    TEST CASE 6: VARIABLE IN EXPRESSION
    */
    Node n6 = {"decl", "z", "", ""};
    analyzer.processNode(n6);

    Node n7 = {"arith", "x", "+", "z"};
    analyzer.processNode(n7);

    /*
    PRINT SYMBOL TABLE
    */
    analyzer.printSymbolTable();

    return 0;
}