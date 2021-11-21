//
// Created by Kevin Tan on 2021/11/20.
//

#ifndef CODE_INSTRUCTION_H
#define CODE_INSTRUCTION_H

#include <sstream>
#include "token.h"
#include "opcode.h"

class Instruction {
public:
    OpCode opcode;
    std::stringstream name;

    Instruction(OpCode opcode, const char *name) : opcode(opcode), name(name) {}

    friend ostream &operator<<(ostream &out, const Instruction &self) {
        out << self.name.str();
        return out;
    }

    virtual ~Instruction() = default;
};

using InstructionP = shared_ptr<Instruction>;

class LoadObject : public Instruction {
public:
    ObjectP &object;

    explicit LoadObject(ObjectP &object) : Instruction(NAME(LOAD_OBJECT)), object(object) {
        switch (object->type) {
            case INT: {
                auto o = dynamic_pointer_cast<IntObject>(object);
                if (object->ident_info == nullptr) {
                    name << '\t' << o->value;
                } else {
                    name << '\t' << o->ident_info->name << "\t(int declared in line " << o->ident_info->line << " at "
                         << o << ')';
                }
                break;
            }
            case CHAR_ARRAY:
                name << '\t' << dynamic_pointer_cast<StringObject>(object)->value;
                break;
            default:
                break;
        }
    }
};

class BuildArray : public Instruction {
public:
    explicit BuildArray() : Instruction(NAME(BUILD_ARRAY)) {}
};

class StoreObject : public Instruction {
public:
    ObjectP &object;

    explicit StoreObject(ObjectP &object) : Instruction(NAME(STORE_OBJECT)), object(object) {
        auto o = dynamic_pointer_cast<IntObject>(object);
        if (object->ident_info == nullptr) {
            name << '\t' << o->value;
        } else {
            name << '\t' << o->ident_info->name << "\t(int declared in line " << o->ident_info->line << " at " << o
                 << ')';
        }
    }
};

class PopTop : public Instruction {
public:
    explicit PopTop() : Instruction(NAME(POP_TOP)) {}
};

class PrintF : public Instruction {
public:
    int va_args_cnt;

    explicit PrintF(int va_args_cnt) : Instruction(NAME(PRINTF)), va_args_cnt(va_args_cnt) {
        name << '\t' << va_args_cnt;
    }
};

class GetInt : public Instruction {
public:
    explicit GetInt() : Instruction(NAME(GETINT)) {}
};

class JumpAbsolute : public Instruction {
public:
    int offset;

    explicit JumpAbsolute(int offset) : Instruction(NAME(JUMP_ABSOLUTE)), offset(offset) {}
};

//class JumpForward : public Instruction {
//public:
//    int offset;
//
//    explicit JumpForward(int offset) : Instruction(NAME(JUMP_FORWARD)), offset(offset) {}
//};

class JumpIfFalseOrPop : public Instruction {
public:
    explicit JumpIfFalseOrPop() : Instruction(NAME(JUMP_IF_FALSE_OR_POP)) {}
};

class JumpIfTrueOrPop : public Instruction {
public:
    explicit JumpIfTrueOrPop() : Instruction(NAME(JUMP_IF_TRUE_OR_POP)) {}
};

class CallFunction : public Instruction {
public:
    FuncObjectP &func;

    explicit CallFunction(FuncObjectP &func) : Instruction(NAME(CALL_FUNCTION)), func(func) {
        name << '\t' << func->ident_info->name << "\t(" << func->params.size() << " args, "
                                                                                  "declared in line "
             << func->ident_info->line << " at " << func << ')';
    }
};

class ReturnValue : public Instruction {
public:
    explicit ReturnValue() : Instruction(NAME(RETURN_VALUE)) {}
};

class UnaryOperation : public Instruction {
public:
    UnaryOpCode unary_opcode;

    explicit UnaryOperation(UnaryOpCode opcode) : Instruction(NAME(UNARY_OP)), unary_opcode(opcode) {}
};

class BinaryOperation : public Instruction {
public:
    BinaryOpCode binary_opcode;

    explicit BinaryOperation(BinaryOpCode opcode) : Instruction(NAME(BINARY_OP)), binary_opcode(opcode) {}
};

//class UnaryPositive : public Instruction {
//public:
//    explicit UnaryPositive() : Instruction(NAME(UNARY_POSITIVE)) {}
//};
//
//class UnaryNegative : public Instruction {
//public:
//    explicit UnaryNegative() : Instruction(NAME(UNARY_NEGATIVE)) {}
//};
//
//class UnaryNot : public Instruction {
//public:
//    explicit UnaryNot() : Instruction(NAME(UNARY_NOT)) {}
//};
//
//class BinaryAdd : public Instruction {
//public:
//    explicit BinaryAdd() : Instruction(NAME(BINARY_ADD)) {}
//};
//
//class BinarySub : public Instruction {
//public:
//    explicit BinarySub() : Instruction(NAME(BINARY_SUB)) {}
//};
//
//class BinaryMul : public Instruction {
//public:
//    explicit BinaryMul() : Instruction(NAME(BINARY_MUL)) {}
//};
//
//class BinaryDiv : public Instruction {
//public:
//    explicit BinaryDiv() : Instruction(NAME(BINARY_DIV)) {}
//};
//
//class BinaryMod : public Instruction {
//public:
//    explicit BinaryMod() : Instruction(NAME(BINARY_MOD)) {}
//};
//
//class BinaryEq : public Instruction {
//public:
//    explicit BinaryEq() : Instruction(NAME(BINARY_EQ)) {}
//};
//
//class BinaryLt : public Instruction {
//public:
//    explicit BinaryLt() : Instruction(NAME(BINARY_LT)) {}
//};
//
//class BinaryLe : public Instruction {
//public:
//    explicit BinaryLe() : Instruction(NAME(BINARY_LE)) {}
//};
//
//class BinaryGt : public Instruction {
//public:
//    explicit BinaryGt() : Instruction(NAME(BINARY_GT)) {}
//};
//
//class BinaryGe : public Instruction {
//public:
//    explicit BinaryGe() : Instruction(NAME(BINARY_GE)) {}
//};
//
//class BinaryLogicalAnd : public Instruction {
//public:
//    explicit BinaryLogicalAnd() : Instruction(NAME(BINARY_LOGICAL_AND)) {}
//};
//
//class BinaryLogicalOr : public Instruction {
//public:
//    explicit BinaryLogicalOr() : Instruction(NAME(BINARY_LOGICAL_OR)) {}
//};

#endif //CODE_INSTRUCTION_H
