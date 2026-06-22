#pragma once
#include "compiler.h"
#include <vector>
#include <string>
#include <sstream>
#include <fstream>
#include <iostream>
#include <unordered_map>
#include <stdexcept>
#include <cctype>

// ================================================================
//  ASSEMBLY GENERATOR  —  Member 5
//  Input : vector<TACInstruction>  from TAC Generator
//  Output: string (assembly code) + optional file write
//
//  Target: simple hypothetical assembly
//    LOAD  reg, mem   — load variable into register
//    MOV   reg, imm   — load immediate value into register
//    STORE mem, reg   — store register to variable
//    ADD / SUB / MUL / DIV  reg, reg
//    PUSH / POP  reg
//    CMP   reg, imm
//    JMP   label
//    JMP_IF_FALSE label
//    PRINT reg
//    HALT
// ================================================================

class AssemblyGenerator {
public:
    std::string generate(const std::vector<TACInstruction>& tac) {
        output_.str("");
        output_.clear();
        variables_.clear();
        seen_.clear();

        collectVariables(tac);

        emitLine("; ===== Generated Assembly =====");
        emitLine(".data");
        for (const auto& var : variables_)
            emitLine("    " + var + " DW 0        ; 16-bit word");
        emitLine("");
        emitLine(".code");
        emitLine("START:");

        for (const auto& instr : tac) {
            emitComment(tacToString(instr));
            switch (instr.op) {
                case TACOp::ASSIGN:        handleAssign(instr);       break;
                case TACOp::ADD:
                case TACOp::SUB:
                case TACOp::MUL:
                case TACOp::DIV:           handleBinaryOp(instr);     break;
                case TACOp::LABEL:         handleLabel(instr);        break;
                case TACOp::GOTO:          handleGoto(instr);         break;
                case TACOp::IF_FALSE_GOTO: handleIfFalseGoto(instr);  break;
                case TACOp::PRINT:         handlePrint(instr);        break;
                default:                   emitLine("    ; [UNKNOWN]"); break;
            }
            emitLine("");
        }

        emitLine("    HALT");
        emitLine("; ===== End of Assembly =====");
        return output_.str();
    }

    void generateToFile(const std::vector<TACInstruction>& tac,
                        const std::string& filename) {
        std::string code = generate(tac);
        std::ofstream out(filename);
        if (!out.is_open())
            throw std::runtime_error("[AssemblyGenerator] Cannot open: " + filename);
        out << code;
        std::cout << "[AssemblyGenerator] Written to " << filename << "\n";
    }

private:
    std::ostringstream output_;
    std::vector<std::string> variables_;
    std::unordered_map<std::string, bool> seen_;

    void emitLine(const std::string& line) { output_ << line << "\n"; }
    void emitComment(const std::string& text) {
        output_ << "    ; TAC: " << text << "\n";
    }

    bool isNumericLiteral(const std::string& s) {
        if (s.empty()) return false;
        size_t start = (s[0] == '-') ? 1 : 0;
        for (size_t i = start; i < s.size(); i++)
            if (!std::isdigit((unsigned char)s[i])) return false;
        return start < s.size();
    }

    // A valid variable/temp name: only letters, digits, underscores
    bool isValidVarName(const std::string& s) {
        if (s.empty()) return false;
        for (char c : s)
            if (!isalnum((unsigned char)c) && c != '_') return false;
        return isalpha((unsigned char)s[0]) || s[0] == '_';
    }

    void trackVar(const std::string& name) {
        if (name.empty() || isNumericLiteral(name) || !isValidVarName(name)) return;
        if (seen_.find(name) == seen_.end()) {
            seen_[name] = true;
            variables_.push_back(name);
        }
    }

    void collectVariables(const std::vector<TACInstruction>& tac) {
        for (const auto& i : tac) {
            if (i.op == TACOp::LABEL || i.op == TACOp::GOTO) continue;
            trackVar(i.result);
            trackVar(i.arg1);
            trackVar(i.arg2);
        }
    }

    std::string tacToString(const TACInstruction& i) {
        switch (i.op) {
            case TACOp::ASSIGN:        return i.result + " = " + i.arg1;
            case TACOp::ADD:           return i.result + " = " + i.arg1 + " + " + i.arg2;
            case TACOp::SUB:           return i.result + " = " + i.arg1 + " - " + i.arg2;
            case TACOp::MUL:           return i.result + " = " + i.arg1 + " * " + i.arg2;
            case TACOp::DIV:           return i.result + " = " + i.arg1 + " / " + i.arg2;
            case TACOp::LABEL:         return "LABEL " + i.result + ":";
            case TACOp::GOTO:          return "GOTO " + i.result;
            case TACOp::IF_FALSE_GOTO: return "IF_FALSE (" + i.arg1 + ") GOTO " + i.result;
            case TACOp::PRINT:         return "PRINT " + i.arg1;
            default:                   return "UNKNOWN";
        }
    }

    void loadIntoAX(const std::string& operand) {
        if (isNumericLiteral(operand))
            emitLine("    MOV  AX, " + operand);   // immediate
        else
            emitLine("    LOAD AX, " + operand);   // from memory
    }

    // result = arg1
    void handleAssign(const TACInstruction& i) {
        loadIntoAX(i.arg1);
        emitLine("    STORE " + i.result + ", AX");
    }

    // result = arg1 OP arg2
    void handleBinaryOp(const TACInstruction& i) {
        loadIntoAX(i.arg1);
        emitLine("    PUSH AX");
        loadIntoAX(i.arg2);
        emitLine("    MOV  BX, AX");
        emitLine("    POP  AX");
        switch (i.op) {
            case TACOp::ADD: emitLine("    ADD AX, BX"); break;
            case TACOp::SUB: emitLine("    SUB AX, BX"); break;
            case TACOp::MUL: emitLine("    MUL AX, BX"); break;
            case TACOp::DIV: emitLine("    DIV AX, BX"); break;
            default: break;
        }
        emitLine("    STORE " + i.result + ", AX");
    }

    // LABEL name:
    void handleLabel(const TACInstruction& i) {
        emitLine(i.result + ":");
    }

    // GOTO label
    void handleGoto(const TACInstruction& i) {
        emitLine("    JMP " + i.result);
    }

    // IF_FALSE "var OP val" GOTO label
    // The condition string from TAC is like "x > 5" or "y > 0"
    void handleIfFalseGoto(const TACInstruction& i) {
        // Parse condition string: split on first space
        std::string cond = i.arg1;
        std::istringstream ss(cond);
        std::string lhs, op, rhs;
        ss >> lhs >> op >> rhs;

        if (!lhs.empty() && !op.empty() && !rhs.empty()) {
            // Evaluate: load LHS, compare with RHS
            loadIntoAX(lhs);
            if (isNumericLiteral(rhs))
                emitLine("    CMP  AX, " + rhs);
            else {
                emitLine("    LOAD BX, " + rhs);
                emitLine("    CMP  AX, BX");
            }
            // Map relational operator to jump mnemonic (jump if condition is FALSE)
            std::string jmp;
            if      (op == ">")  jmp = "JLE";  // not (>) → <=
            else if (op == "<")  jmp = "JGE";  // not (<) → >=
            else if (op == ">=") jmp = "JL";
            else if (op == "<=") jmp = "JG";
            else if (op == "==") jmp = "JNE";
            else if (op == "!=") jmp = "JE";
            else                 jmp = "JMP_IF_FALSE";
            emitLine("    " + jmp + " " + i.result);
        } else {
            // Fallback: treat arg1 as a boolean variable
            loadIntoAX(i.arg1);
            emitLine("    CMP  AX, 0");
            emitLine("    JE   " + i.result);
        }
    }

    // PRINT var
    void handlePrint(const TACInstruction& i) {
        loadIntoAX(i.arg1);
        emitLine("    PRINT AX");
    }
};

// ── Public entry point ────────────────────────────────────────────
inline std::string runAssemblyGenerator(const std::vector<TACInstruction>& tac,
                                        const std::string& outputFile = "output.asm") {
    AssemblyGenerator gen;
    gen.generateToFile(tac, outputFile);
    return gen.generate(tac);
}
