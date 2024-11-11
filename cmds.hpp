/*
_CMD_DEF_(name, opcode, size, exec, read)
*/

#define _INT_SIZE_ (sizeof(int32_t))
#define _PUSH_VAR_ code.push_back(cmd.args[iarg++]);
#define _PUT_INT_(value) {code.resize(code.size()+_INT_SIZE_); \
    *((int32_t*)&(code.data()[code.size()-_INT_SIZE_])) = value;}
#define _PUSH_INT_ _PUT_INT_(cmd.args[iarg++])
#define _POP_VAR_ARG_ (func->code[++pc])
#define _POP_VAR_VAL_ (vars[func->code[++pc]])
#define _POP_INT_ARG_ (*(uint32_t*)&(func->code.data()[(pc+=4)-3]))

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
_CMD_DEF_(ADD, 3, 4, {int vard = _POP_VAR_ARG_; int lhs=_POP_VAR_VAL_; int rhs=_POP_VAR_VAL_; vars[vard] = lhs+rhs;}, {_PUSH_VAR_ _PUSH_VAR_ _PUSH_VAR_})
_CMD_DEF_(EXIT, 4, 1, {return;}, {})
_CMD_DEF_(PRINT, 5, 2, {std::cout << "out: " << vars[_POP_VAR_ARG_] << '\n';}, {_PUSH_VAR_})
_CMD_DEF_(JMP, 6, 5, _DO_JMP_, _PUSH_LABEL_OFFSET_)
_CMD_DEF_(JEQ, 7, 7, {int lhs=_POP_VAR_VAL_; int rhs=_POP_VAR_VAL_; if (lhs==rhs) _DO_JMP_}, {_PUSH_VAR_ _PUSH_VAR_ _PUSH_LABEL_OFFSET_})
_CMD_DEF_(JNE, 8, 7, {int lhs=_POP_VAR_VAL_; int rhs=_POP_VAR_VAL_; if (lhs!=rhs) _DO_JMP_}, {_PUSH_VAR_ _PUSH_VAR_ _PUSH_LABEL_OFFSET_})
_CMD_DEF_(INPUT, 9, 2, {auto varind = _POP_VAR_ARG_; std::cout << "input: "; std::cin >> vars[varind];}, {_PUSH_VAR_})
_CMD_DEF_(MOV, 10, 3, {int vard = _POP_VAR_ARG_; int src=_POP_VAR_VAL_; vars[vard] = src;}, {_PUSH_VAR_ _PUSH_VAR_})
_CMD_DEF_(JLE, 11, 7, {int lhs=_POP_VAR_VAL_; int rhs=_POP_VAR_VAL_; if (lhs<=rhs) _DO_JMP_}, {_PUSH_VAR_ _PUSH_VAR_ _PUSH_LABEL_OFFSET_})
 
