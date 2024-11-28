/*
_CMD_DEF_(name, opcode, size, exec, read)
*/

// builder helpers
#define _REG_SIZE_ (sizeof(RegType))
#define _PUSH_VAR_(reg) do { code.push_back(cmd.reg); } while(0)
#define _PUT_INT_(value) do {code.resize(code.size()+_REG_SIZE_); \
    *((int32_t*)&(code.data()[code.size()-_REG_SIZE_])) = value;} while(0)
#define _PUSH_IMM_ _PUT_INT_(cmd.im)
#define _PUSH_LABEL_OFFSET_ do { \
    int offset = _GET_LABEL_(cmd.im) - pc; DEB("dooffset" << offset)\
    _PUT_INT_(offset); \
} while(0)

// executor helpers
#define _POP_VAR_ARG_ (func.code[++frame->pc])
#define _POP_VAR_VAL_ (frame->vars[func.code[++frame->pc]])
#define _POP_INT_ARG_ (*(uint32_t*)&(func.code[(frame->pc+=4)-3]))
#define _DO_JMP_ do { \
    int tmp = _POP_INT_ARG_; DEB("dojmp" << tmp) \
    frame->new_pc = old_pc + tmp; } while(0)

_CMD_DEF_(NOP, 0, 1, do {} while(0), do {} while(0))
_CMD_DEF_(INC, 1, 2, do {frame->vars[_POP_VAR_ARG_]++;} while(0), do {_PUSH_VAR_(rd);} while(0))
_CMD_DEF_(STORE, 2, 6, do {auto varind = _POP_VAR_ARG_; frame->vars[varind] = _POP_INT_ARG_;} while(0), do {_PUSH_VAR_(rd); _PUSH_IMM_;} while(0))

#define _BIN_OP(name, ind, op) _CMD_DEF_(name, ind, 4, do {int vard = _POP_VAR_ARG_; int lhs=_POP_VAR_VAL_; int rhs=_POP_VAR_VAL_; frame->vars[vard] = lhs op rhs;} while(0), do {_PUSH_VAR_(rd); _PUSH_VAR_(rs1); _PUSH_VAR_(rs2);} while(0))

_BIN_OP(ADD, 20, +)
_BIN_OP(SUB, 21, -)
_BIN_OP(MUL, 22, *)
_BIN_OP(DIV, 23, /)
_BIN_OP(MOD, 24, %)

#undef _BIN_OP

_CMD_DEF_(EXIT, 4, 1, do {return 0;} while(0), do {} while(0))
_CMD_DEF_(PRINT, 5, 2, do {std::cout << "out: " << frame->vars[_POP_VAR_ARG_] << '\n';} while(0), do {_PUSH_VAR_(rs1);} while(0))

_CMD_DEF_(JMP, 6, 5, _DO_JMP_;, _PUSH_LABEL_OFFSET_;)

#define _BIN_COND_JMP_(name, ind, op) _CMD_DEF_(name, ind, 7, do {int lhs=_POP_VAR_VAL_; int rhs=_POP_VAR_VAL_; if (lhs op rhs) _DO_JMP_;} while(0), do {_PUSH_VAR_(rs1); _PUSH_VAR_(rs2); _PUSH_LABEL_OFFSET_;} while(0))

_BIN_COND_JMP_(JEQ, 30, ==)
_BIN_COND_JMP_(JNE, 31, !=)
_BIN_COND_JMP_(JLT, 32, <)
_BIN_COND_JMP_(JLE, 33, <=)
_BIN_COND_JMP_(JGE, 34, >=)
_BIN_COND_JMP_(JGT, 35, >)

_CMD_DEF_(INPUT, 9, 2, do {auto varind = _POP_VAR_ARG_; std::cout << "input: "; std::cin >> frame->vars[varind];} while(0), do {_PUSH_VAR_(rd);} while(0))
_CMD_DEF_(MOV, 10, 3, do {int vard = _POP_VAR_ARG_; int src=_POP_VAR_VAL_; frame->vars[vard] = src;} while(0), do {_PUSH_VAR_(rd); _PUSH_VAR_(rs1);} while(0))

_CMD_DEF_(CALL0, 40, 6, do {int vard = _POP_VAR_ARG_; int funcind = _POP_INT_ARG_; frame->vars[vard] = run_func(funcind, {}); printf("res=%d\n", frame->vars[vard]); frame = frame->prev;} while(0), do {_PUSH_VAR_(rd); _PUSH_IMM_;} while(0))
_CMD_DEF_(CALL1, 41, 7, do {int vard = _POP_VAR_ARG_; int funcind = _POP_INT_ARG_; RegType res = run_func(funcind, {_POP_VAR_VAL_}); frame = frame->prev;frame->vars[vard] = res;} while(0), do {_PUSH_VAR_(rd); _PUSH_IMM_; _PUSH_VAR_(rs1);} while(0))

_CMD_DEF_(RET, 13, 2, do { return frame->vars[_POP_VAR_ARG_];} while(0), do {_PUSH_VAR_(rs1);} while(0))
