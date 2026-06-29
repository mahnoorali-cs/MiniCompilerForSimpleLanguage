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

using namespace std;

// ── Helper: print token list ──
void printTokens(const vector<Token>& tokens) {
    cout << "\n===== TOKENS =====\n";
    for (const auto& t : tokens)
        cout << "  " << t.type << "(" << t.value << ")\n";
    cout << "==================\n";
}

// ── Helper: print node list ──
void printNodes(const vector<Node>& nodes) {
    cout << "\n===== AST NODES =====\n";
    for (const auto& n : nodes) {
        cout << "  [" << n.type << "]";
        if (!n.datatype.empty()) cout << " type=" << n.datatype;
        if (!n.left.empty())     cout << " left=" << n.left;
        if (!n.op.empty())       cout << " op=" << n.op;
        if (!n.right.empty())    cout << " right=" << n.right;
        if (!n.condition.empty())cout << " cond=" << n.condition;
        cout << "\n";
    }
    cout << "=====================\n";
}

int main() {
    // ── Test source code ──
    string sourceCode = R"(
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

    cout << "======================================\n";
    cout << "   MINI COMPILER — START\n";
    cout << "======================================\n";
    cout << "\nSource Code:\n" << sourceCode << "\n";

    // ── Phase 1: Lexer ──
    cout << "\n[Phase 1] Running Lexer...\n";
    vector<Token> tokens = runLexer(sourceCode);
    printTokens(tokens);

    // ── Phase 2: Parser ──
    cout << "\n[Phase 2] Running Parser...\n";
    vector<Node> nodes = runParser(tokens);
    printNodes(nodes);

    if (nodes.empty()) {
        cerr << "\n[Compiler] Parser returned no nodes. Stopping.\n";
        return 1;
    }

    // ── Phase 3: Semantic Analyzer ──
    cout << "\n[Phase 3] Running Semantic Analyzer...\n";
    vector<Node> checkedNodes = runSemanticAnalyzer(nodes);

    if (checkedNodes.empty()) {
        cerr << "\n[Compiler] Semantic analysis produced no valid nodes. Stopping.\n";
        return 1;
    }

    // ── Phase 4: TAC Generator ──
    cout << "\n[Phase 4] Running TAC Generator...\n";
    vector<TACInstruction> tac = runTACGenerator(checkedNodes);

    if (tac.empty()) {
        cerr << "\n[Compiler] TAC Generator produced no instructions. Stopping.\n";
        return 1;
    }

    // ── Phase 5: Assembly Generator ──
    cout << "\n[Phase 5] Running Assembly Generator...\n";
    AssemblyGenerator asmGen;
    string assembly = asmGen.generate(tac);

    cout << "\n===== ASSEMBLY OUTPUT =====\n";
    cout << assembly;

    // Write to file
    asmGen.generateToFile(tac, "output.asm");

    cout << "\n======================================\n";
    cout << "   MINI COMPILER — DONE\n";
    cout << "======================================\n";

    return 0;
}
