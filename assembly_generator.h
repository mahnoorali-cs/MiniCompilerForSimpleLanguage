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

using namespace std;

//  ASSEMBLY GENERATOR  
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

class AssemblyGenerator {
public:
    string generate(const vector<TACInstruction>& tac) {
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

    void generateToFile(const vector<TACInstruction>& tac,
                        const string& filename) {
        string code = generate(tac);
        ofstream out(filename);
        if (!out.is_open())
            throw runtime_error("[AssemblyGenerator] Cannot open: " + filename);
        out << code;
        cout << "[AssemblyGenerator] Written to " << filename << "\n";
    }

private:
    ostringstream output_;
    vector<string> variables_;
    unordered_map<string, bool> seen_;

    void emitLine(const string& line) { output_ << line << "\n"; }
    void emitComment(const string& text) {
        output_ << "    ; TAC: " << text << "\n";
    }

    bool isNumericLiteral(const string& s) {
        if (s.empty()) return false;
        size_t start = (s[0] == '-') ? 1 : 0;
        for (size_t i = start; i < s.size(); i++)
            if (!isdigit((unsigned char)s[i])) return false;
        return start < s.size();
    }

    bool isValidVarName(const string& s) {
        if (s.empty()) return false;
        for (char c : s)
            if (!isalnum((unsigned char)c) && c != '_') return false;
        return isalpha((unsigned char)s[0]) || s[0] == '_';
    }

    void trackVar(const string& name) {
        if (name.empty() || isNumericLiteral(name) || !isValidVarName(name)) return;
        if (seen_.find(name) == seen_.end()) {
            seen_[name] = true;
            variables_.push_back(name);
        }
    }

    void collectVariables(const vector<TACInstruction>& tac) {
        for (const auto& i : tac) {
            if (i.op == TACOp::LABEL || i.op == TACOp::GOTO) continue;
            trackVar(i.result);
            trackVar(i.arg1);
            trackVar(i.arg2);
        }
    }

    string tacToString(const TACInstruction& i) {
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

    void loadIntoAX(const string& operand) {
        if (isNumericLiteral(operand))
            emitLine("    MOV  AX, " + operand);
        else
            emitLine("    LOAD AX, " + operand);
    }

    void handleAssign(const TACInstruction& i) {
        loadIntoAX(i.arg1);
        emitLine("    STORE " + i.result + ", AX");
    }

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

    void handleLabel(const TACInstruction& i) {
        emitLine(i.result + ":");
    }

    void handleGoto(const TACInstruction& i) {
        emitLine("    JMP " + i.result);
    }

    void handleIfFalseGoto(const TACInstruction& i) {
        string cond = i.arg1;
        istringstream ss(cond);
        string lhs, op, rhs;
        ss >> lhs >> op >> rhs;

        if (!lhs.empty() && !op.empty() && !rhs.empty()) {
            loadIntoAX(lhs);
            if (isNumericLiteral(rhs))
                emitLine("    CMP  AX, " + rhs);
            else {
                emitLine("    LOAD BX, " + rhs);
                emitLine("    CMP  AX, BX");
            }
            string jmp;
            if      (op == ">")  jmp = "JLE";
            else if (op == "<")  jmp = "JGE";
            else if (op == ">=") jmp = "JL";
            else if (op == "<=") jmp = "JG";
            else if (op == "==") jmp = "JNE";
            else if (op == "!=") jmp = "JE";
            else                 jmp = "JMP_IF_FALSE";
            emitLine("    " + jmp + " " + i.result);
        } else {
            loadIntoAX(i.arg1);
            emitLine("    CMP  AX, 0");
            emitLine("    JE   " + i.result);
        }
    }

    void handlePrint(const TACInstruction& i) {
        loadIntoAX(i.arg1);
        emitLine("    PRINT AX");
    }
};

// ── Public entry point 
inline string runAssemblyGenerator(const vector<TACInstruction>& tac,
                                   const string& outputFile = "output.asm") {
    AssemblyGenerator gen;
    gen.generateToFile(tac, outputFile);
    return gen.generate(tac);
}
