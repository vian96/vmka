#include <alloca.h>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <sys/types.h>
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

typedef int32_t RegType;

struct CmdEntry {
    InstrOpcodeNS::InstrOpcode opc;
    uint8_t rd, rs1, rs2;
    RegType im;
};

struct CallArgs {
    RegType arg0, arg1;
};

std::vector<uint8_t> build_bytecode(std::vector<CmdEntry> cmds) {
    std::vector<uint8_t> code;
    std::unordered_map<int, size_t> labels_map;
    #define _CMD_DEF_(name, opcode, argsnum, exec, read) {\
    case InstrOpcodeNS::name: \
        code.push_back(opcode); \
        size_t iarg = 0; \
        read; \
        continue; }

    #define _GET_LABEL_(ind) (0)
    for (const auto &cmd:cmds) {
        int pc = code.size();
        switch (cmd.opc) {
            case InstrOpcodeNS::LABEL:
                labels_map[cmd.im] = code.size();
                DEB((char) cmd.im << code.size())
                continue;
            #include "cmds.hpp"
            #undef _GET_LABEL_
        }
    }
    code.resize(0);
    #define _GET_LABEL_(ind) (labels_map[ind])
    for (const auto &cmd:cmds) {
        int pc = code.size();
        switch (cmd.opc) {
            case InstrOpcodeNS::LABEL:
                continue;
            #include "cmds.hpp"
            #undef _CMD_DEF_
            #undef _GET_LABEL_
        }
    }
    DEB("code size: " << code.size());
    return code;
}

class Function {
public:
    uint8_t *code;
    size_t num_vars;
    size_t num_params;
    Function(size_t nvars, size_t nparams, uint8_t *code_)
        : num_vars(nvars), num_params(nparams), code(code_) {}
};

class Frame {
public:
    const Function &func;
    int pc;
    int new_pc;
    Frame *prev;
    RegType vars[];

    Frame(Frame *prev_, const Function &func_) : prev(prev_), func(func_), pc(0) {}
};

class Runtime {
public:
    std::vector<Function> functions;
    Frame *frame;
    RegType run_func(size_t func_ind, CallArgs args) {
        auto &func = functions[func_ind];
        frame = new(alloca(sizeof(Frame) + func.num_vars*sizeof(RegType))) Frame(frame, func);
        if (frame->func.num_params)
            frame->vars[0] = args.arg0;
        if (frame->func.num_params > 1)
            frame->vars[1] = args.arg1;
        //printf("arg0=%d arg1=%d num=%d\n", args.arg0, args.arg1, func.num_params);
        //printf("arg0=%d arg1=%d\n", frame->vars[0], frame->vars[1]);

        frame->new_pc = 0;
        while (true) {
            frame->pc = frame->new_pc;
            switch (func.code[frame->pc]) {
                #define _CMD_DEF_(name, opcode, size, exec, read) {\
                case InstrOpcodeNS::name: \
                    auto old_pc = frame->pc; \
                    frame->new_pc = frame->pc + size; \
                    DEB(#name << frame->pc << ' ' << frame->new_pc); \
                    exec; \
                    DEB(#name << frame->pc << ' ' << frame->new_pc); \
                    continue; }
                #include "cmds.hpp"
            }
        }
    }
};

Runtime runtime;

int main() {
    int status=0;
    using namespace InstrOpcodeNS;
    std::cout << "parse\n";
    
    auto main_code = build_bytecode({
        {STORE, 0, .im=10},
//        {STORE, {1, 17}},
        {INPUT, 1},
        {LABEL, .im='s'},
        {INC, 0},
        {PRINT, .rs1=0},
        {JEQ, .rs1=0, .rs2=1, .im='e'},
        {JMP, .im='s'},
        {LABEL, .im='e'},
        {EXIT}
    });
    Function main_func(3, 0, main_code.data());

    auto sum_code = build_bytecode({
        {STORE, 0, .im=10},
        {INPUT, 1},
        {ADD, 1, 1, 0},
        {PRINT, .rs1=1},
        {RET, .rs1=1}
    });
    Function sum_func(3, 0, sum_code.data());

    auto fib_iter_code = build_bytecode({
        // v0 i v1 n v2 f0 v3 f1 v4 t
        {INPUT, 1},
        {STORE, 0, .im=2},
        {STORE, 2, .im=0},
        {STORE, 3, .im=1},
        {LABEL, .im='l'},
        {ADD, 4, 2, 3},
//        {PRINT, {4}},
        {MOV, 2, 3},
        {MOV, 3, 4},
        {INC, 0},
//        {PRINT, {3}},
        {JLE, .rs1=0, .rs2=1, .im='l'},
        {PRINT, .rs1=3},
        {RET, .rs1=3}
    });
    Function fib_iter_func(3, 0, fib_iter_code.data());
 
    auto fib_rec_code = build_bytecode({
        // v0 n v1 n-1(-2) v2 fib(n-1) v3 fib(n-2) v4 -1holder
        {STORE, 4, .im=1},
        {JLE, .rs1=0, .rs2=4, .im='s'},
        {STORE, 4, .im=-1},
        {ADD, 0, 0, 4},
      //  {PRINT, {0}},
        {CALL1, 2, .rs1=0, .im=2}, // fib(n-1)
       //  {PRINT, {0}},
       // {PRINT, {2}},
        {ADD, 0, 0, 4},
        {CALL1, 3, .rs1=0, .im=2}, // fib(n-2)
       // {PRINT, {0}},
       // {PRINT, {3}},
        {ADD, 2, 2, 3},
        {RET, .rs1=2},
        {LABEL, .im='s'},
        {RET, .rs1=0}
    });
    Function fib_rec_func(5, 1, fib_rec_code.data());
 
    runtime.functions = {main_func, fib_iter_func, fib_rec_func};

    std::cout << "count from 10 to input:\n";
    runtime.run_func(0, {});
    std::cout << "iter fib(input): ";
    runtime.run_func(1, {});
    std::cout << "recursive fib(11): " << runtime.run_func(2, {11}) << '\n';
    std::cout << "finishing..\n";
    return status;
}
