/*
_CMD_DEF_(name, opcode, size, exec, read)
*/

#define _INT_SIZE_ (sizeof(int32_t))
#define _PUSH_VAR_ code.push_back(cmd.args[iarg++]);
#define _PUT_INT_(value) {code.resize(code.size()+_INT_SIZE_); \
    *((int32_t*)&(code.data()[code.size()-_INT_SIZE_])) = value;}
#define _PUSH_INT_ _PUT_INT_(cmd.args[iarg++])
#define _POP_VAR_ARG_ (func.code[++pc])
#define _POP_VAR_VAL_ (vars[func.code[++pc]])
#define _POP_INT_ARG_ (*(uint32_t*)&(func.code.data()[(pc+=4)-3]))

#define _DO_JMP_ { \
    int tmp = _POP_INT_ARG_; DEB("dojmp" << tmp) \
    new_pc = old_pc + tmp; }
#define _PUSH_LABEL_OFFSET_ { \
    int offset = _GET_LABEL_(cmd.args[iarg++]) - pc; DEB("dooffset" << offset)\
    _PUT_INT_(offset) \
}

_CMD_DEF_(NOP, 0, 1, {}, {})
_CMD_DEF_(INC, 1, 2, {vars[_POP_VAR_ARG_]++;}, {_PUSH_VAR_})
_CMD_DEF_(STORE, 2, 6, {auto varind = _POP_VAR_ARG_; vars[varind] = _POP_INT_ARG_;}, {_PUSH_VAR_ _PUSH_INT_})

#define _BIN_OP(name, ind, op) _CMD_DEF_(name, ind, 4, {int vard = _POP_VAR_ARG_; int lhs=_POP_VAR_VAL_; int rhs=_POP_VAR_VAL_; vars[vard] = lhs op rhs;}, {_PUSH_VAR_ _PUSH_VAR_ _PUSH_VAR_})

_BIN_OP(ADD, 20, +)
_BIN_OP(SUB, 21, -)
_BIN_OP(MUL, 22, *)
_BIN_OP(DIV, 23, /)
_BIN_OP(MOD, 24, %)

#undef _BIN_OP

_CMD_DEF_(EXIT, 4, 1, {return 0;}, {})
_CMD_DEF_(PRINT, 5, 2, {std::cout << "out: " << vars[_POP_VAR_ARG_] << '\n';}, {_PUSH_VAR_})

_CMD_DEF_(JMP, 6, 5, _DO_JMP_, _PUSH_LABEL_OFFSET_)

#define _BIN_COND_JMP_(name, ind, op) _CMD_DEF_(name, ind, 7, {int lhs=_POP_VAR_VAL_; int rhs=_POP_VAR_VAL_; if (lhs op rhs) _DO_JMP_}, {_PUSH_VAR_ _PUSH_VAR_ _PUSH_LABEL_OFFSET_})

_BIN_COND_JMP_(JEQ, 30, ==)
_BIN_COND_JMP_(JNE, 31, !=)
_BIN_COND_JMP_(JLT, 32, <)
_BIN_COND_JMP_(JLE, 33, <=)
_BIN_COND_JMP_(JGE, 34, >=)
_BIN_COND_JMP_(JGT, 35, >)

_CMD_DEF_(INPUT, 9, 2, {auto varind = _POP_VAR_ARG_; std::cout << "input: "; std::cin >> vars[varind];}, {_PUSH_VAR_})
_CMD_DEF_(MOV, 10, 3, {int vard = _POP_VAR_ARG_; int src=_POP_VAR_VAL_; vars[vard] = src;}, {_PUSH_VAR_ _PUSH_VAR_})

#define _GET_ARG_I_(i) int arg##i = _POP_VAR_VAL_;
#define _CALL_N_(n, ind, args_read, args_push, ...) _CMD_DEF_(CALL##n, ind, 6+n, {int vard = _POP_VAR_ARG_; int funcind = _POP_INT_ARG_; args_read; Frame fr(interp.functions[funcind], {__VA_ARGS__}); vars[vard] = fr.run(interp);}, {_PUSH_VAR_ _PUSH_INT_ args_push})

_CALL_N_(1, 41, _GET_ARG_I_(0), _PUSH_VAR_, arg0)
_CALL_N_(2, 42, _GET_ARG_I_(0) _GET_ARG_I_(1), _PUSH_VAR_ _PUSH_VAR_, arg0, arg1)
_CALL_N_(3, 43, _GET_ARG_I_(0) _GET_ARG_I_(1) _GET_ARG_I_(2), _PUSH_VAR_ _PUSH_VAR_ _PUSH_VAR_, arg0, arg1, arg2)

_CMD_DEF_(RET, 13, 2, { return vars[_POP_VAR_ARG_];}, {_PUSH_VAR_})
