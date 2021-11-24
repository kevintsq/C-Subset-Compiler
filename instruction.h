//
// Created by Kevin Tan on 2021/11/20.
//

#ifndef CODE_INSTRUCTION_H
#define CODE_INSTRUCTION_H

#include <sstream>
#include <utility>
#include "token.h"
#include "opcode.h"

class Instruction {
public:
    OpCode opcode;
    std::stringstream name;

    Instruction(OpCode opcode, const char *name) : opcode(opcode) {
        this->name << name;
    }

    friend ostream &operator<<(ostream &out, const Instruction &self) {
        out << self.name.str();
        return out;
    }

    virtual ~Instruction() = default;
};

using InstructionP = shared_ptr<Instruction>;

class LoadObject : public Instruction {
public:
    ObjectP object;

    explicit LoadObject(const ObjectP &object) : Instruction(NAME(LOAD_OBJECT)), object(object) {
        switch (object->type) {
            case INT: {
                auto o = cast<IntObject>(object);
                if (object->ident_info == nullptr) {
                    name << '\t' << o->value;
                } else {
                    name << '\t' << o->ident_info->name << "\t(INT, declared in line " << o->ident_info->line << " at "
                         << o << ')';
                }
                break;
            }
            case INT_ARRAY: {
                auto o = cast<ArrayObject>(object);
                if (object->ident_info != nullptr) {
                    name << '\t' << o->ident_info->name << "\t(INT_ARRAY, declared in line " << o->ident_info->line
                         << " at " << o << ')';
                }
                break;
            }
            case CHAR_ARRAY:
                name << '\t' << cast<StringObject>(object)->value;
                break;
            default:
                ERROR_LIMITED_SUPPORT(INT or INT_ARRAY or CHAR_ARRAY);
        }
    }
};

class BuildArray : public Instruction {
public:
    explicit BuildArray() : Instruction(NAME(BUILD_ARRAY)) {}
};

class InitArray : public Instruction {
public:
    ArrayObjectP array;

    explicit InitArray(const ArrayObjectP &array) : Instruction(NAME(INIT_ARRAY)), array(array) {
        if (array->ident_info != nullptr) {
            name << '\t' << array->ident_info->name << "\t(INT_ARRAY, declared in line " << array->ident_info->line
                 << " at " << array << ')';
        }
    }
};

class SubscriptArray : public Instruction {
public:
    explicit SubscriptArray() : Instruction(NAME(SUBSCR_ARRAY)) {}
};

class StoreSubscript : public Instruction {
public:
    explicit StoreSubscript() : Instruction(NAME(STORE_SUBSCR)) {}
};

class StoreObject : public Instruction {
public:
    ObjectP object;

    explicit StoreObject(const ObjectP &object) : Instruction(NAME(STORE_OBJECT)), object(object) {
        if (object->ident_info != nullptr) {
            switch (object->type) {
                case INT:
                    name << '\t' << object->ident_info->name << "\t(INT, declared in line " << object->ident_info->line
                         << " at " << object << ')';
                    break;
                case INT_ARRAY:
                    name << '\t' << object->ident_info->name << "\t(INT_ARRAY, declared in line " << object->ident_info->line
                         << " at " << object << ')';
                    break;
                default:
                    ERROR_LIMITED_SUPPORT_WITH_LINE(object->ident_info->line, INT or INT_ARRAY assignment);
            }
        }
    }
};

class PopTop : public Instruction {
public:
    explicit PopTop() : Instruction(NAME(POP_TOP)) {}
};

class Exit : public Instruction {
public:
    explicit Exit() : Instruction(NAME(EXIT_INTERP)) {}
};

class PrintF : public Instruction {
public:
    FormatStringP format_string;

    explicit PrintF(const FormatStringP &format_string) : Instruction(NAME(CALL_PRINTF)), format_string(format_string) {
        name << '\t' << format_string->value;
    }
};

class GetInt : public Instruction {
public:
    explicit GetInt() : Instruction(NAME(CALL_GETINT)) {}
};

//class JumpAbsolute : public Instruction {
//public:
//    long long offset;
//
//    explicit JumpAbsolute(long long offset) : Instruction(NAME(JUMP_ABSOLUTE)), offset(offset) {}
//};

//class JumpForward : public Instruction {
//public:
//    long long offset;
//
//    explicit JumpForward(long long offset) : Instruction(NAME(JUMP_FORWARD)), offset(offset) {}
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
    FuncObjectP func;
public:
    CallFunction() : Instruction(NAME(CALL_FUNCTION)) {}

    explicit CallFunction(const FuncObjectP &func) : Instruction(NAME(CALL_FUNCTION)) {
        set_func(func);
    }

    inline void set_func(const FuncObjectP &f) {
        func = f;
        name << '\t' << f->ident_info->name << "\t(" << f->params.size() << " args, offset " << f->code_offset
             << ", declared in line " << f->ident_info->line << " at " << f << ')';
    }

    inline FuncObjectP get_func() {
        return func;
    }
};

using CallFunctionP = shared_ptr<CallFunction>;

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

    explicit BinaryOperation(BinaryOpCode opcode) : Instruction(NAME(BINARY_OP)), binary_opcode(opcode) {
        switch (opcode) {
            case BINARY_ADD:
                name << "\t+";
                break;
            case BINARY_SUB:
                name << "\t-";
                break;
            case BINARY_MUL:
                name << "\t*";
                break;
            case BINARY_DIV:
                name << "\t/";
                break;
            case BINARY_MOD:
                name << "\t%";
                break;
            case BINARY_EQ:
                name << "\t==";
                break;
            case BINARY_NE:
                name << "\t!=";
                break;
            case BINARY_LT:
                name << "\t<";
                break;
            case BINARY_LE:
                name << "\t<=";
                break;
            case BINARY_GT:
                name << "\t>";
                break;
            case BINARY_GE:
                name << "\t>=";
                break;
            case BINARY_LOGICAL_AND:
                name << "\t&&";
                break;
            case BINARY_LOGICAL_OR:
                name << "\t||";
                break;
            default:
                cerr << "In " << __func__ << " line " << __LINE__
                     << " source code line, NOTHING binary operation shouldn't be generated." << endl;
        }
    }
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
