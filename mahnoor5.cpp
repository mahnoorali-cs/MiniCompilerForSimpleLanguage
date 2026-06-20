#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <stdexcept>

// ──────────────────────────────────────────────
//  TAC Instruction Representation
//  (Assumed to be produced by your team's earlier
//   TAC-generation phase; replicate the same struct
//   used there so the two phases connect cleanly.)
// ──────────────────────────────────────────────
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
    std::string result;  // destination / label name
    std::string arg1;    // first operand
    std::string arg2;    // second operand (may be empty)
};

// ──────────────────────────────────────────────
//  Assembly Generator
// ──────────────────────────────────────────────
class AssemblyGenerator {
public:
    // Entry point: take a list of TAC instructions,
    // return the full assembly program as a string.
    std::string generate(const std::vector<TACInstruction>& tac) {
        output_.str("");
        output_.clear();

        emitLine("; ===== Generated Assembly =====");
        emitLine(".data");
        // Reserve space for every variable/temp we encounter
        collectVariables(tac);
        for (const auto& var : variables_) {
            emitLine("    " + var + " DW 0     ; 16-bit word");
        }

        emitLine("");
        emitLine(".code");
        emitLine("START:");

        for (const auto& instr : tac) {
            emitComment(tacToString(instr));   // original TAC as a comment
            switch (instr.op) {
                case TACOp::ASSIGN:        handleAssign(instr);       break;
                case TACOp::ADD:           handleBinaryOp(instr);     break;
                case TACOp::SUB:           handleBinaryOp(instr);     break;
                case TACOp::MUL:           handleBinaryOp(instr);     break;
                case TACOp::DIV:           handleBinaryOp(instr);     break;
                case TACOp::LABEL:         handleLabel(instr);        break;
                case TACOp::GOTO:          handleGoto(instr);         break;
                case TACOp::IF_FALSE_GOTO: handleIfFalseGoto(instr);  break;
                case TACOp::PRINT:         handlePrint(instr);        break;
                default:
                    emitLine("    ; [UNKNOWN TAC OP]");
            }
            emitLine("");
        }

        emitLine("    HALT");
        emitLine("; ===== End of Assembly =====");
        return output_.str();
    }

    // Convenience: write assembly directly to a file
    void generateToFile(const std::vector<TACInstruction>& tac,
                        const std::string& filename) {
        std::string asm_code = generate(tac);
        std::ofstream out(filename);
        if (!out.is_open())
            throw std::runtime_error("Cannot open output file: " + filename);
        out << asm_code;
        std::cout << "[AssemblyGenerator] Written to " << filename << "\n";
    }

private:
    std::ostringstream            output_;
    std::vector<std::string>      variables_;   // ordered, unique
    std::unordered_map<std::string,bool> seen_;

    // ── Helpers ──────────────────────────────

    void emitLine(const std::string& line) {
        output_ << line << "\n";
    }

    void emitComment(const std::string& text) {
        output_ << "    ; TAC: " << text << "\n";
    }

    // Register a variable/temp for the .data section
    void trackVar(const std::string& name) {
        if (name.empty()) return;
        // Skip numeric literals — they go inline
        if (isNumericLiteral(name)) return;
        if (seen_.find(name) == seen_.end()) {
            seen_[name] = true;
            variables_.push_back(name);
        }
    }

    bool isNumericLiteral(const std::string& s) {
        if (s.empty()) return false;
        size_t start = (s[0] == '-') ? 1 : 0;
        for (size_t i = start; i < s.size(); ++i)
            if (!std::isdigit(static_cast<unsigned char>(s[i]))) return false;
        return start < s.size();
    }

    // Walk all TAC to find every variable/temp
    void collectVariables(const std::vector<TACInstruction>& tac) {
        for (const auto& instr : tac) {
            if (instr.op != TACOp::LABEL && instr.op != TACOp::GOTO) {
                trackVar(instr.result);
                trackVar(instr.arg1);
                trackVar(instr.arg2);
            }
        }
    }

    // Produce a readable string of a TAC instruction (for comments)
    std::string tacToString(const TACInstruction& i) {
        switch (i.op) {
            case TACOp::ASSIGN:        return i.result + " = " + i.arg1;
            case TACOp::ADD:           return i.result + " = " + i.arg1 + " + " + i.arg2;
            case TACOp::SUB:           return i.result + " = " + i.arg1 + " - " + i.arg2;
            case TACOp::MUL:           return i.result + " = " + i.arg1 + " * " + i.arg2;
            case TACOp::DIV:           return i.result + " = " + i.arg1 + " / " + i.arg2;
            case TACOp::LABEL:         return "LABEL " + i.result + ":";
            case TACOp::GOTO:          return "GOTO " + i.result;
            case TACOp::IF_FALSE_GOTO: return "IF_FALSE " + i.arg1 + " GOTO " + i.result;
            case TACOp::PRINT:         return "PRINT " + i.arg1;
            default:                   return "UNKNOWN";
        }
    }

    // Load a value (variable OR literal) into AX
    void loadIntoAX(const std::string& operand) {
        if (isNumericLiteral(operand))
            emitLine("    MOV AX, " + operand);      // immediate value
        else
            emitLine("    LOAD AX, " + operand);     // from memory
    }

    // ── Instruction Handlers ─────────────────

    // result = arg1
    void handleAssign(const TACInstruction& i) {
        loadIntoAX(i.arg1);
        emitLine("    STORE " + i.result + ", AX");
    }

    // result = arg1 OP arg2
    void handleBinaryOp(const TACInstruction& i) {
        loadIntoAX(i.arg1);           // AX = arg1
        emitLine("    PUSH AX");      // save arg1 on stack

        loadIntoAX(i.arg2);           // AX = arg2
        emitLine("    MOV BX, AX");   // BX = arg2

        emitLine("    POP AX");       // AX = arg1

        switch (i.op) {
            case TACOp::ADD: emitLine("    ADD AX, BX");  break;
            case TACOp::SUB: emitLine("    SUB AX, BX");  break;
            case TACOp::MUL: emitLine("    MUL AX, BX");  break;
            case TACOp::DIV: emitLine("    DIV AX, BX");  break;
            default: break;
        }
        // AX now holds the result
        emitLine("    STORE " + i.result + ", AX");
    }

    // LABEL name:
    void handleLabel(const TACInstruction& i) {
        emitLine(i.result + ":");     // label appears at column 0
    }

    // GOTO label
    void handleGoto(const TACInstruction& i) {
        emitLine("    JMP " + i.result);
    }

    // IF_FALSE var GOTO label
    void handleIfFalseGoto(const TACInstruction& i) {
        loadIntoAX(i.arg1);
        emitLine("    CMP AX, 0");
        emitLine("    JMP_IF_FALSE " + i.result);   // jump if AX == 0 (false)
    }

    // PRINT var
    void handlePrint(const TACInstruction& i) {
        loadIntoAX(i.arg1);
        emitLine("    PRINT AX");
    }
};

// ──────────────────────────────────────────────
//  Demo / test driver
//  Replace this main() with a call from your team's
//  main compiler driver; just pass your TAC vector.
// ──────────────────────────────────────────────
int main() {
    // Sample TAC: mirrors your example
    //   t1 = a + b
    //   t2 = t1 * c
    //   x  = t2
    //   PRINT x
    std::vector<TACInstruction> sampleTAC = {
        { TACOp::ADD,    "t1", "a",  "b"  },
        { TACOp::MUL,    "t2", "t1", "c"  },
        { TACOp::ASSIGN, "x",  "t2", ""   },
        { TACOp::PRINT,  "",   "x",  ""   },

        // Example with a conditional jump (e.g. from an if-statement)
        { TACOp::LABEL,         "L1",    "",    "" },
        { TACOp::IF_FALSE_GOTO, "L_END", "x",  "" },
        { TACOp::SUB,           "t3",    "x", "1" },
        { TACOp::ASSIGN,        "x",     "t3", "" },
        { TACOp::GOTO,          "L1",    "",   "" },
        { TACOp::LABEL,         "L_END", "",   "" },
        { TACOp::PRINT,         "",      "x",  "" },
    };

    AssemblyGenerator gen;

    // Print to stdout
    std::string assembly = gen.generate(sampleTAC);
    std::cout << assembly << std::endl;

    // Also write to file
    gen.generateToFile(sampleTAC, "output.asm");

    return 0;
}