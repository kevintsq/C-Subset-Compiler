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

class LoadName : public Instruction {
public:
    ObjectP object;

    explicit LoadName(const ObjectP &object) : Instruction(NAME(LOAD_NAME) "\t\t"), object(object) {
        IdentP i = object->ident_info;
        switch (object->type) {
            case INT: {
                name << i->name << "\t\t(INT, declared in line " << i->line << ')';
                break;
            }
            case INT_ARRAY: {
                name << i->name << "\t\t(INT_ARRAY, declared in line " << i->line << ')';
                break;
            }
            default:
//                ERROR_LIMITED_SUPPORT(INT or INT_ARRAY);
                break;
        }
    }
};

class LoadFast : public Instruction {
public:
    ObjectP object;

    explicit LoadFast(const ObjectP &object) : Instruction(NAME(LOAD_FAST) "\t\t"), object(object) {
        switch (object->type) {
            case INT: {
                auto o = cast<IntObject>(object);
                if (object->ident_info == nullptr) {
                    name << o->value;
                } else {
                    name << o->ident_info->name << "\t\t(INT, declared in line "
                         << o->ident_info->line << " at " << o << ')';
                }
                break;
            }
            case INT_ARRAY: {
                auto o = cast<ArrayObject>(object);
                if (object->ident_info != nullptr) {
                    name << o->ident_info->name << "\t\t(INT_ARRAY, declared in line "
                         << o->ident_info->line << " at " << o << ')';
                }
                break;
            }
            case CHAR_ARRAY:
                name << "\t\t" << cast<StringObject>(object)->value;
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
    explicit InitArray(const ArrayObjectP &array) : Instruction(NAME(INIT_ARRAY)) {}
};

class SubscriptArray : public Instruction {
public:
    explicit SubscriptArray() : Instruction(NAME(SUBSCR_ARRAY)) {}
};

class StoreSubscript : public Instruction {
public:
    explicit StoreSubscript() : Instruction(NAME(STORE_SUBSCR)) {}
};

class StoreName : public Instruction {
public:
    ObjectP object;

    explicit StoreName(const ObjectP &object) : Instruction(NAME(STORE_NAME) "\t\t"), object(object) {
        IdentP i = object->ident_info;
        if (object->ident_info != nullptr) {
            switch (object->type) {
                case INT:
                    name << i->name << "\t\t(INT, declared in line " << i->line << ')';
                    break;
                case INT_ARRAY:
                    name << i->name << "\t\t(INT_ARRAY, declared in line " << i->line << ')';
                    break;
                default:
                    ERROR_LIMITED_SUPPORT_WITH_LINE(object->ident_info->line, INT or INT_ARRAY assignment);
            }
        }
    }
};

class PopTop : public Instruction {  // used when value is not used
public:
    explicit PopTop() : Instruction(NAME(POP_TOP)) {}
};

class Exit : public Instruction {
public:
    explicit Exit() : Instruction(NAME(EXIT_INTERP) "\n") {}
};

class PrintF : public Instruction {
public:
    FormatStringP format_string;

    explicit PrintF(const FormatStringP &format_string) : Instruction(NAME(CALL_PRINTF) "\t\t"),
                                                          format_string(format_string) {
        name << format_string->value;
    }
};

class GetInt : public Instruction {
public:
    explicit GetInt() : Instruction(NAME(CALL_GETINT)) {}
};

class JumpInstruction : public Instruction {
    long long offset = 0;
public:
    JumpInstruction(OpCode opcode, const char *name) : Instruction(opcode, name) {}

    JumpInstruction(OpCode opcode, const char *name, long long offset) : Instruction(opcode, name) {
        set_offset(offset);
    }

    inline void set_offset(long long o) {
        offset = o;
        name << o << '\n';
    }

    inline long long get_offset() const { return offset; }
};

using JumpInstructionP = shared_ptr<JumpInstruction>;

class JumpAbsolute : public JumpInstruction {
public:
    JumpAbsolute() : JumpInstruction(NAME(JUMP_ABSOLUTE) "\t\t") {}

    explicit JumpAbsolute(long long offset) : JumpInstruction(NAME(JUMP_ABSOLUTE) "\t\t", offset) {}
};

class PopJumpIfFalse : public JumpInstruction {
public:
    PopJumpIfFalse() : JumpInstruction(NAME(POP_JUMP_IF_FALSE) "\t") {}

    explicit PopJumpIfFalse(long long offset) : JumpInstruction(NAME(POP_JUMP_IF_FALSE) "\t", offset) {}
};

class PopJumpIfTrue : public JumpInstruction {
public:
    PopJumpIfTrue() : JumpInstruction(NAME(POP_JUMP_IF_TRUE) "\t") {}

    explicit PopJumpIfTrue(long long offset) : JumpInstruction(NAME(POP_JUMP_IF_TRUE) "\t", offset) {}
};

class CallFunction : public Instruction {
    FuncObjectP func;
public:
    CallFunction() : Instruction(NAME(CALL_FUNCTION) "\t\t") {}

    explicit CallFunction(const FuncObjectP &func) : Instruction(NAME(CALL_FUNCTION) "\t\t") { set_func(func); }

    inline void set_func(const FuncObjectP &f) {
        func = f;
        name << f->ident_info->name << "\t\t(" << f->params.size() << " args, offset " << f->code_offset
             << ", declared in line " << f->ident_info->line << " at " << f << ")\n";
    }

    inline FuncObjectP get_func() const { return func; }
};

class ReturnValue : public Instruction {
public:
    explicit ReturnValue() : Instruction(NAME(RETURN_VALUE) "\n") {}
};

class UnaryOperation : public Instruction {
public:
    UnaryOpCode unary_opcode;

    explicit UnaryOperation(UnaryOpCode opcode) : Instruction(NAME(UNARY_OP) "\t\t"), unary_opcode(opcode) {
        switch (opcode) {
            case UNARY_POSITIVE:
                name << '+';
                break;
            case UNARY_NEGATIVE:
                name << '-';
                break;
            case UNARY_NOT:
                name << '!';
                break;
        }
    }
};

class BinaryOperation : public Instruction {
public:
    BinaryOpCode binary_opcode;

    explicit BinaryOperation(BinaryOpCode opcode) : Instruction(NAME(BINARY_OP) "\t\t"), binary_opcode(opcode) {
        switch (opcode) {
            case BINARY_ADD:
                name << '+';
                break;
            case BINARY_SUB:
                name << '-';
                break;
            case BINARY_MUL:
                name << '*';
                break;
            case BINARY_DIV:
                name << '/';
                break;
            case BINARY_MOD:
                name << '%';
                break;
            case BINARY_EQ:
                name << "==";
                break;
            case BINARY_NE:
                name << "!=";
                break;
            case BINARY_LT:
                name << '<';
                break;
            case BINARY_LE:
                name << "<=";
                break;
            case BINARY_GT:
                name << '>';
                break;
            case BINARY_GE:
                name << ">=";
                break;
            case BINARY_LOGICAL_AND:
                name << "&&";
                break;
            case BINARY_LOGICAL_OR:
                name << "||";
                break;
            default:
                cerr << "In " << __func__ << " line " << __LINE__
                     << " source code line, NOTHING binary operation shouldn't be generated." << endl;
        }
    }
};

#endif //CODE_INSTRUCTION_H
