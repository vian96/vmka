#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <string>
#include <unordered_map>
#include <vector>
#include <iostream>

// #define DEBUG
#ifdef DEBUG 
#define DEB(out) std::cout << out << '\n';
#else
#define DEB(out) 
#endif

namespace InstrOpcodeNS { // so that i can write using
enum InstrOpcode {
LABEL = -1,
#define _CMD_DEF_(name, opcode, argsnum, exec, read) name = opcode, 
#include "cmds.hpp"
#undef _CMD_DEF_
};
}

struct CmdEntry {
    InstrOpcodeNS::InstrOpcode opc;
    std::vector<int> args;
};

class Function {
public:
    std::vector<uint8_t> code;
    size_t num_vars;
    size_t num_params;
    Function(size_t nvars, size_t nparams, std::vector<CmdEntry> cmds)
        : num_vars(nvars), num_params(nparams) {
        std::unordered_map<int, size_t> labels_map;
        for (const auto &cmd:cmds) {
            int pc = code.size();
            switch (cmd.opc) {
                case InstrOpcodeNS::LABEL:
                    labels_map[cmd.args[0]] = code.size();
                    DEB((char) cmd.args[0] << code.size())
                    continue;
                #define _GET_LABEL_(ind) (0)
                #define _CMD_DEF_(name, opcode, argsnum, exec, read) {\
                case InstrOpcodeNS::name: \
                    code.push_back(opcode); \
                    size_t iarg = 0; \
                    read \
                    continue; }
                #include "cmds.hpp"
                #undef _CMD_DEF_
                #undef _GET_LABEL_
            }
        }
        code.resize(0);
        for (const auto &cmd:cmds) {
            int pc = code.size();
            switch (cmd.opc) {
                case InstrOpcodeNS::LABEL:
                    continue;
                #define _GET_LABEL_(ind) (labels_map[ind])
                #define _CMD_DEF_(name, opcode, argsnum, exec, read) {\
                case InstrOpcodeNS::name: \
                    code.push_back(opcode); \
                    size_t iarg = 0; \
                    read \
                    continue; }
                #include "cmds.hpp"
                #undef _CMD_DEF_
                #undef _GET_LABEL_
            }
        }
    }
};

// TODO move to class Executor instead of globals
std::vector<Function*> functions = {nullptr};
std::unordered_map<std::string, int> funcmap;

class Frame {
public:
    const Function *func;
    std::vector<int32_t> vars;
    int pc;
    int new_pc;
    int run() {
        new_pc = 0;
        while (true) {
            pc = new_pc;
            switch (func->code[pc]) {
                #define _CMD_DEF_(name, opcode, size, exec, read) {\
                case InstrOpcodeNS::name: \
                    auto old_pc = pc; \
                    new_pc = pc + size; \
                    DEB(#name << pc << ' ' << new_pc); \
                    exec \
                    DEB(#name << pc << ' ' << new_pc); \
                    continue; }
                #include "cmds.hpp"
            }
        }
    }
    Frame(Function *func_) : func(func_), vars(std::vector<int32_t>(func->num_vars)), pc(0) {}
    Frame(Function *func_, std::vector<int32_t> params) : func(func_), vars(std::vector<int32_t>(func->num_vars)), pc(0) {
        std::copy(params.begin(), params.end(), vars.begin());
    }
};

int main() {
    int status=0;
    using namespace InstrOpcodeNS;
    std::cout << "parse\n";
    Function main_func(3, 0, {
        {STORE, {0, 10}},
//        {STORE, {1, 17}},
        {INPUT, {1}},
        {LABEL, {'s'}},
        {INC, {0}},
        {PRINT, {0}},
        {JEQ, {0, 1, 'e'}},
        {JMP, {'s'}},
        {LABEL, {'e'}},
        {EXIT, {}}
    });
    Function sum_func(3, 0, {
        {STORE, {0, 10}},
        {INPUT, {1}},
        {ADD, {1, 1, 0}},
        {PRINT, {1}},
        {EXIT, {}}
    });
    Function fib_iter_func(3, 0, {
        // v0 i v1 n v2 f0 v3 f1 v4 t
        {INPUT, {1}},
        {STORE, {0, 2}},
        {STORE, {2, 0}},
        {STORE, {3, 1}},
        {LABEL, {'l'}},
        {ADD, {4, 2, 3}},
//        {PRINT, {4}},
        {MOV, {2, 3}},
        {MOV, {3, 4}},
        {INC, {0}},
//        {PRINT, {3}},
        {JLE, {0, 1, 'l'}},
        {PRINT, {3}},
        {EXIT, {}}
    });

    Function fib_rec_func(5, 1, {
        // v0 n v1 n-1(-2) v2 fib(n-1) v3 fib(n-2) v4 -1holder
        {STORE, {4, 1}},
        {JLE, {0, 4, 's'}},
        {STORE, {4, -1}},
        {ADD, {0, 0, 4}},
        {CALL1, {2, 1, 0}}, // fib(n-1)
        {PRINT, {0}},
        {PRINT, {2}},
        {ADD, {0, 0, 4}},
        {CALL1, {3, 1, 0}}, // fib(n-2)
        {PRINT, {0}},
        {PRINT, {3}},
        {ADD, {2, 2, 3}},
        {RET, {2}},
        {LABEL, {'s'}},
        {RET, {0}}
    });

    Frame fr(&fib_iter_func);
    std::cout << "run\n";
 //   fr.run();
    std::cout << "recurs\n";
    functions = {nullptr, &fib_rec_func};
    Frame frec(&fib_rec_func, {11});
    std::cout << frec.run();
    std::cout << "exit\n";
    return status;
}
