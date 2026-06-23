#pragma once
#include "compiler.h"
#include <vector>
#include <string>
#include <iostream>
#include <cctype>

// ================================================================
//  SEMANTIC ANALYZER  —  Member 3
// by Maryam Ameen
//  Input : vector<Node>  from Parser
//  Output: vector<Node>  (same nodes, only passes if valid)
//
//  Checks performed:
//    - Duplicate declaration in same scope
//    - Use of undeclared variable
//    - Type mismatch on assignment (variable vs variable)
// ================================================================

// ── Symbol Table ─────────────────────────────────────────────────
struct Symbol {
    std::string name;
    std::string type;
    int         scopeLevel;
};

class SymbolTable {
private:
    std::vector<Symbol> table;
    int currentScope;

public:
    SymbolTable() : currentScope(0) {}

    void enterScope() { currentScope++; }

    void exitScope() {
        for (int i = (int)table.size() - 1; i >= 0; i--) {
            if (table[i].scopeLevel == currentScope)
                table.erase(table.begin() + i);
        }
        currentScope--;
    }

    // Returns false and prints error on duplicate in same scope
    bool addSymbol(const std::string& name, const std::string& type) {
        for (auto& s : table) {
            if (s.name == name && s.scopeLevel == currentScope) {
                std::cout << "[Semantic Error] Duplicate declaration of '"
                          << name << "' in same scope\n";
                return false;
            }
        }
        table.push_back({name, type, currentScope});
        return true;
    }

    bool isDeclared(const std::string& name) const {
        for (auto it = table.rbegin(); it != table.rend(); ++it)
            if (it->name == name) return true;
        return false;
    }

    std::string getType(const std::string& name) const {
        for (auto it = table.rbegin(); it != table.rend(); ++it)
            if (it->name == name) return it->type;
        return "";
    }

    void printTable() const {
        std::cout << "\n====== SYMBOL TABLE ======\n";
        for (auto& s : table)
            std::cout << "  Name: " << s.name
                      << " | Type: "  << s.type
                      << " | Scope: " << s.scopeLevel << "\n";
        std::cout << "==========================\n";
    }
};

// ── Semantic Analyzer ─────────────────────────────────────────────
class SemanticAnalyzer {
private:
    SymbolTable sym;
    bool hasErrors;

    bool isNumericLiteral(const std::string& s) {
        if (s.empty()) return false;
        bool hasDot = false;
        size_t start = (s[0] == '-') ? 1 : 0;
        for (size_t i = start; i < s.size(); i++) {
            if (s[i] == '.' && !hasDot) { hasDot = true; continue; }
            if (!isdigit(s[i])) return false;
        }
        return start < s.size();
    }

    // ── decl node ────────────────────────────────────────────────
    bool handleDecl(const Node& n) {
        if (n.datatype.empty()) {
            std::cout << "[Semantic Error] Declaration of '" << n.left
                      << "' has no type\n";
            return false;
        }
        return sym.addSymbol(n.left, n.datatype);
    }

    // ── assign node ──────────────────────────────────────────────
    bool handleAssign(const Node& n) {
        // LHS must be declared
        if (!sym.isDeclared(n.left)) {
            std::cout << "[Semantic Error] Variable '" << n.left
                      << "' used before declaration\n";
            return false;
        }
        // "__arith__" sentinel means RHS came from an arith node — already checked there
        if (n.right == "__arith__") {
            std::cout << "[Semantic] Assignment OK: " << n.left << " = <arith result>\n";
            return true;
        }
        // If RHS is a plain variable (not a numeric literal), check it too
        if (!n.right.empty() && !isNumericLiteral(n.right) && isalpha(n.right[0])) {
            if (!sym.isDeclared(n.right)) {
                std::cout << "[Semantic Error] Variable '" << n.right
                          << "' used before declaration\n";
                return false;
            }
            // Type check — only when both sides are named variables
            if (sym.getType(n.left) != sym.getType(n.right)) {
                std::cout << "[Semantic Error] Type mismatch: cannot assign '"
                          << sym.getType(n.right) << "' to '"
                          << sym.getType(n.left) << "'\n";
                return false;
            }
        }
        std::cout << "[Semantic] Assignment OK: " << n.left << " = " << n.right << "\n";
        return true;
    }

    // ── arith node ───────────────────────────────────────────────
    bool handleArith(const Node& n) {
        bool ok = true;
        if (!n.left.empty() && !isNumericLiteral(n.left) && isalpha(n.left[0])) {
            if (!sym.isDeclared(n.left)) {
                std::cout << "[Semantic Error] Variable '" << n.left
                          << "' used before declaration\n";
                ok = false;
            }
        }
        if (!n.right.empty() && !isNumericLiteral(n.right) && isalpha(n.right[0])) {
            if (!sym.isDeclared(n.right)) {
                std::cout << "[Semantic Error] Variable '" << n.right
                          << "' used before declaration\n";
                ok = false;
            }
        }
        if (ok)
            std::cout << "[Semantic] Arithmetic OK: "
                      << n.left << " " << n.op << " " << n.right << "\n";
        return ok;
    }

    // ── if / while node ──────────────────────────────────────────
    bool handleControlFlow(const Node& n) {
        // Condition variables already checked when the body nodes are processed
        std::cout << "[Semantic] Control flow (" << n.type
                  << ") with condition: " << n.condition << "\n";
        return true;
    }

    // ── print node ───────────────────────────────────────────────
    bool handlePrint(const Node& n) {
        if (!isNumericLiteral(n.left) && isalpha(n.left[0])) {
            if (!sym.isDeclared(n.left)) {
                std::cout << "[Semantic Error] Variable '" << n.left
                          << "' used before declaration\n";
                return false;
            }
        }
        std::cout << "[Semantic] Print OK: " << n.left << "\n";
        return true;
    }

public:
    SemanticAnalyzer() : hasErrors(false) {}

    std::vector<Node> analyze(const std::vector<Node>& nodes) {
        std::vector<Node> valid;

        for (const auto& n : nodes) {
            bool ok = false;

            if      (n.type == "decl")  ok = handleDecl(n);
            else if (n.type == "assign") ok = handleAssign(n);
            else if (n.type == "arith")  ok = handleArith(n);
            else if (n.type == "if" || n.type == "while")
                                         ok = handleControlFlow(n);
            else if (n.type == "print")  ok = handlePrint(n);
            else {
                std::cout << "[Semantic] Unknown node type: " << n.type << "\n";
                ok = false;
            }

            if (!ok) hasErrors = true;
            else     valid.push_back(n);
        }

        sym.printTable();

        if (hasErrors)
            std::cout << "\n[Semantic] Completed with errors.\n";
        else
            std::cout << "\n[Semantic] All checks passed.\n";

        return valid;
    }
};

// ── Public entry point ────────────────────────────────────────────
inline std::vector<Node> runSemanticAnalyzer(const std::vector<Node>& nodes) {
    SemanticAnalyzer sa;
    return sa.analyze(nodes);
}
