//
// Created by Kevin Tan on 2021/11/20.
//

#include "vm.h"

StackMachine::StackMachine(vector<InstructionP> &instructions) : instructions(instructions) {
    auto pc = instructions.begin();
    while (pc < instructions.end()) {
        switch ((*pc)->opcode) {
            case LOAD_OBJECT:
                stack.push_back(cast<LoadObject>(*pc)->object);
                break;
            case STORE_OBJECT:
                cast<IntObject>(cast<LoadObject>(*pc)->object)->value = cast<IntObject>(stack.back())->value;
                // must fall through
            case POP_TOP:
                stack.pop_back();
                break;
            case BUILD_ARRAY: {
                ArrayObjectP array = make_shared<ArrayObject>();
                for (auto &o : stack) {
                    array->value.push_back(o);
                }
                stack.clear();  // FIXME: check
                stack.push_back(array);
                break;
            }
            case GETINT: {
                int n;
                cin >> n;
                stack.push_back(make_shared<IntObject>(n));
                break;
            }
            case JUMP_ABSOLUTE:
            case JUMP_IF_FALSE_OR_POP:
            case JUMP_IF_TRUE_OR_POP:
            case PRINTF:
            case CALL_FUNCTION:
            case RETURN_VALUE:
            case UNARY_OP: {
                int value = dynamic_pointer_cast<IntObject>(stack.back())->value;
                stack.pop_back();
                switch (dynamic_pointer_cast<UnaryOperation>(*pc)->unary_opcode) {
                    case UNARY_POSITIVE:
                        stack.push_back(make_shared<IntObject>(+value));
                        break;
                    case UNARY_NEGATIVE:
                        stack.push_back(make_shared<IntObject>(-value));
                        break;
                    case UNARY_NOT:
                        stack.push_back(make_shared<IntObject>(!value));
                        break;
                }
                break;
            }
            case BINARY_OP: {
                int value1 = dynamic_pointer_cast<IntObject>(stack.back())->value;
                stack.pop_back();
                int value2 = dynamic_pointer_cast<IntObject>(stack.back())->value;
                stack.pop_back();
                switch (dynamic_pointer_cast<BinaryOperation>(*pc)->binary_opcode) {
                    case BINARY_ADD:
                        stack.push_back(make_shared<IntObject>(value1 + value2));
                        break;
                    case BINARY_SUB:
                        stack.push_back(make_shared<IntObject>(value1 - value2));  // FIXME: 表达式1-2，假定load指令2在1前
                        break;
                    case BINARY_MUL:
                        stack.push_back(make_shared<IntObject>(value1 * value2));
                        break;
                    case BINARY_DIV:
                        stack.push_back(make_shared<IntObject>(value1 / value2));  // FIXME: 表达式1/2，假定load指令2在1前
                        break;
                    case BINARY_MOD:
                        stack.push_back(make_shared<IntObject>(value1 % value2));  // FIXME: 表达式1/2，假定load指令2在1前
                        break;
                    case BINARY_EQ:
                        stack.push_back(make_shared<IntObject>(value1 == value2));
                        break;
                    case BINARY_LT:
                        stack.push_back(make_shared<IntObject>(value1 < value2));
                        break;
                    case BINARY_LE:
                        stack.push_back(make_shared<IntObject>(value1 <= value2));
                        break;
                    case BINARY_GT:
                        stack.push_back(make_shared<IntObject>(value1 > value2));
                        break;
                    case BINARY_GE:
                        stack.push_back(make_shared<IntObject>(value1 >= value2));
                        break;
                    case BINARY_LOGICAL_AND:
                        stack.push_back(make_shared<IntObject>(value1 && value2));
                        break;
                    case BINARY_LOGICAL_OR:
                        stack.push_back(make_shared<IntObject>(value1 || value2));
                        break;
                }
                break;
            }
            default:
                pc++;
        }
    }
}
