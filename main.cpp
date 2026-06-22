// ================================================================
//  MINI COMPILER — Main Driver
//  Integrates all 5 phases:
//    Lexer → Parser → Semantic Analyzer → TAC Generator → Assembly Generator
// ================================================================
#include "compiler.h"
#include "lexer.h"
#include "parser.h"
#include "semantic.h"
#include "tac_generator.h"
#include "assembly_generator.h"
#include <iostream>
#include <string>
#include <vector>

// ── Helper: print token list ──────────────────────────────────────
void printTokens(const std::vector<Token>& tokens) {
    std::cout << "\n===== TOKENS =====\n";
    for (const auto& t : tokens)
        std::cout << "  " << t.type << "(" << t.value << ")\n";
    std::cout << "==================\n";
}

// ── Helper: print node list ───────────────────────────────────────
void printNodes(const std::vector<Node>& nodes) {
    std::cout << "\n===== AST NODES =====\n";
    for (const auto& n : nodes) {
        std::cout << "  [" << n.type << "]";
        if (!n.datatype.empty()) std::cout << " type=" << n.datatype;
        if (!n.left.empty())     std::cout << " left=" << n.left;
        if (!n.op.empty())       std::cout << " op=" << n.op;
        if (!n.right.empty())    std::cout << " right=" << n.right;
        if (!n.condition.empty())std::cout << " cond=" << n.condition;
        std::cout << "\n";
    }
    std::cout << "=====================\n";
}

int main() {
    // ── Test source code ──────────────────────────────────────────
    std::string sourceCode = R"(
        int x;
        float y;
        bool flag;
        x = 10;
        y = 3;
        x = x + 5;
        if(x > 5) {
            cout << x;
        }
        while(y > 0) {
            y = y - 1;
        }
    )";

    std::cout << "======================================\n";
    std::cout << "   MINI COMPILER — START\n";
    std::cout << "======================================\n";
    std::cout << "\nSource Code:\n" << sourceCode << "\n";

    // ── Phase 1: Lexer ────────────────────────────────────────────
    std::cout << "\n[Phase 1] Running Lexer...\n";
    std::vector<Token> tokens = runLexer(sourceCode);
    printTokens(tokens);

    // ── Phase 2: Parser ───────────────────────────────────────────
    std::cout << "\n[Phase 2] Running Parser...\n";
    std::vector<Node> nodes = runParser(tokens);
    printNodes(nodes);

    if (nodes.empty()) {
        std::cerr << "\n[Compiler] Parser returned no nodes. Stopping.\n";
        return 1;
    }

    // ── Phase 3: Semantic Analyzer ────────────────────────────────
    std::cout << "\n[Phase 3] Running Semantic Analyzer...\n";
    std::vector<Node> checkedNodes = runSemanticAnalyzer(nodes);

    if (checkedNodes.empty()) {
        std::cerr << "\n[Compiler] Semantic analysis produced no valid nodes. Stopping.\n";
        return 1;
    }

    // ── Phase 4: TAC Generator ────────────────────────────────────
    std::cout << "\n[Phase 4] Running TAC Generator...\n";
    std::vector<TACInstruction> tac = runTACGenerator(checkedNodes);

    if (tac.empty()) {
        std::cerr << "\n[Compiler] TAC Generator produced no instructions. Stopping.\n";
        return 1;
    }

    // ── Phase 5: Assembly Generator ───────────────────────────────
    std::cout << "\n[Phase 5] Running Assembly Generator...\n";
    AssemblyGenerator asmGen;
    std::string assembly = asmGen.generate(tac);

    std::cout << "\n===== ASSEMBLY OUTPUT =====\n";
    std::cout << assembly;

    // Write to file
    asmGen.generateToFile(tac, "output.asm");

    std::cout << "\n======================================\n";
    std::cout << "   MINI COMPILER — DONE\n";
    std::cout << "======================================\n";

    return 0;
}
