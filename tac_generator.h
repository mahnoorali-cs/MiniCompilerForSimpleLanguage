#pragma once
#include "compiler.h"
#include <vector>
#include <string>
#include <iostream>
#include <cstdio>
#include <sstream>

// ================================================================
//  TAC GENERATOR  —  Member 4
//  Input : vector<Node>           from Semantic Analyzer
//  Output: vector<TACInstruction> consumed by Assembly Generator
//
//  Also prints the TAC to stdout for debugging.
// ================================================================

class TACGenerator {
private:
    int tempCount;
    int labelCount;
    std::string lastTemp;
    std::vector<TACInstruction> instructions;

    std::string newTemp() {
        char buf[16];
        snprintf(buf, sizeof(buf), "t%d", ++tempCount);
        return std::string(buf);
    }

    std::string newLabel() {
        char buf[16];
        snprintf(buf, sizeof(buf), "L%d", ++labelCount);
        return std::string(buf);
    }

    void emit(TACOp op,
              const std::string& result = "",
              const std::string& arg1   = "",
              const std::string& arg2   = "") {
        instructions.push_back({op, result, arg1, arg2});
        printInstruction(instructions.back());
    }

    // Pretty-print a single instruction
    void printInstruction(const TACInstruction& i) {
        switch (i.op) {
            case TACOp::ASSIGN:
                std::cout << "  " << i.result << " = " << i.arg1 << "\n"; break;
            case TACOp::ADD:
                std::cout << "  " << i.result << " = " << i.arg1 << " + " << i.arg2 << "\n"; break;
            case TACOp::SUB:
                std::cout << "  " << i.result << " = " << i.arg1 << " - " << i.arg2 << "\n"; break;
            case TACOp::MUL:
                std::cout << "  " << i.result << " = " << i.arg1 << " * " << i.arg2 << "\n"; break;
            case TACOp::DIV:
                std::cout << "  " << i.result << " = " << i.arg1 << " / " << i.arg2 << "\n"; break;
            case TACOp::LABEL:
                std::cout << i.result << ":\n"; break;
            case TACOp::GOTO:
                std::cout << "  GOTO " << i.result << "\n"; break;
            case TACOp::IF_FALSE_GOTO:
                std::cout << "  IF_FALSE (" << i.arg1 << ") GOTO " << i.result << "\n"; break;
            case TACOp::PRINT:
                std::cout << "  PRINT " << i.arg1 << "\n"; break;
        }
    }

    // ── Map an arith operator string → TACOp ─────────────────────
    TACOp opFromString(const std::string& op) {
        if (op == "+") return TACOp::ADD;
        if (op == "-") return TACOp::SUB;
        if (op == "*") return TACOp::MUL;
        if (op == "/") return TACOp::DIV;
        return TACOp::ADD; // default fallback
    }

    // ── Node handlers ─────────────────────────────────────────────

    // "decl" — declarations with initial value become assignments
    void genDecl(const Node& n) {
        if (!n.right.empty()) {
            emit(TACOp::ASSIGN, n.left, n.right);
        }
        // Declaration without initializer: no TAC needed
    }

    // "assign" — simple assignment
    // "__arith__" sentinel means the arith node already wrote to n.left directly
    void genAssign(const Node& n) {
        if (n.right == "__arith__") return; // arith node already emitted the correct TAC
        emit(TACOp::ASSIGN, n.left, n.right);
    }

    // "arith" — binary expression
    // If node.datatype holds a variable name (set by parser for x = a+b),
    // emit directly into that variable. Otherwise use a temp.
    std::string genArith(const Node& n) {
        std::string dest = n.datatype.empty() ? newTemp() : n.datatype;
        emit(opFromString(n.op), dest, n.left, n.right);
        lastTemp = dest;
        return dest;
    }

    // "if" — condition already parsed as a string like "x > 5"
    void genIf(const Node& n) {
        std::string endLabel = newLabel();
        emit(TACOp::IF_FALSE_GOTO, endLabel, n.condition);
        // Body nodes are emitted separately (they precede the if-node or follow)
        // We just emit the end label here so control flow is complete
        emit(TACOp::LABEL, endLabel);
    }

    // "while"
    void genWhile(const Node& n) {
        std::string startLabel = newLabel();
        std::string endLabel   = newLabel();
        emit(TACOp::LABEL, startLabel);
        emit(TACOp::IF_FALSE_GOTO, endLabel, n.condition);
        // Body nodes follow inline in the instruction stream
        emit(TACOp::GOTO, startLabel);
        emit(TACOp::LABEL, endLabel);
    }

    // "print"
    void genPrint(const Node& n) {
        emit(TACOp::PRINT, "", n.left);
    }

public:
    TACGenerator() : tempCount(0), labelCount(0) {}

    std::vector<TACInstruction> generate(const std::vector<Node>& nodes) {
        instructions.clear();
        tempCount  = 0;
        labelCount = 0;

        std::cout << "\n===== THREE-ADDRESS CODE =====\n";

        for (const auto& n : nodes) {
            if      (n.type == "decl")              genDecl(n);
            else if (n.type == "assign")            genAssign(n);
            else if (n.type == "arith")             genArith(n);
            else if (n.type == "if")                genIf(n);
            else if (n.type == "while")             genWhile(n);
            else if (n.type == "print")             genPrint(n);
            else
                std::cout << "  ; [TAC] Skipping node type: " << n.type << "\n";
        }

        std::cout << "==============================\n";
        return instructions;
    }
};

// ── Public entry point ────────────────────────────────────────────
inline std::vector<TACInstruction> runTACGenerator(const std::vector<Node>& nodes) {
    TACGenerator gen;
    return gen.generate(nodes);
}
