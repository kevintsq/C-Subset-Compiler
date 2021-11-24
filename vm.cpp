//
// Created by Kevin Tan on 2021/11/20.
//

#include "vm.h"

void StackMachine::run() {
    auto pc = instructions.begin();
    while (pc < instructions.end()) {
        switch ((*pc)->opcode) {
            case LOAD_OBJECT:
                stack->push_back(cast<LoadObject>(*pc)->object);
                ++pc;
                break;
            case STORE_OBJECT: {
                IntObjectP o = cast<IntObject>(stack->back());
                stack->pop_back();
                if (cast<StoreObject>(*pc)->object->is_const) {
                    ERROR_NOT_SUPPORTED(modifing const);
                } else {
                    cast<IntObject>(cast<StoreObject>(*pc)->object)->value = o->value;
                }
                ++pc;
                break;
            }
            case POP_TOP:
                stack->pop_back();
                ++pc;
                break;
            case BUILD_ARRAY: {
                long long size = cast<IntObject>(stack->back())->value;
                stack->pop_back();
                ArrayObjectP array = cast<ArrayObject>(stack->back());
                if (!array->is_const) {
                    array->data = make_shared<Array>();
                    for (long long i = 0; i < size; i++) {
                        array->data->push_back(make_shared<IntObject>());
                    }
                }
                stack->pop_back();
                ++pc;
                break;
            }
            case INIT_ARRAY: {
                auto &array = cast<InitArray>(*pc)->array->data;
                for (auto i = (long long) array->size() - 1; i >= 0; i--) {
                    (*array)[i] = stack->back();
                    stack->pop_back();
                }
                ++pc;
                break;
            }
            case STORE_SUBSCR: {
                long long value = cast<IntObject>(stack->back())->value;
                stack->pop_back();
                cast<IntObject>(stack->back())->value = value;
                stack->pop_back();
                ++pc;
                break;
            }
            case SUBSCR_ARRAY: {
                long long index = cast<IntObject>(stack->back())->value;
                stack->pop_back();
                ArrayObjectP array = cast<ArrayObject>(stack->back());
                stack->pop_back();
                if (array->dims.size() == 1) {
                    stack->push_back((*array->data)[index]);
                } else {
                    ArrayObjectP sliced = make_shared<ArrayObject>();
                    long long begin = index, end = index + 1;
                    for (auto it = array->dims.begin() + 1; it != array->dims.end(); ++it) {
                        sliced->dims.push_back(*it);
                        begin *= *it;
                        end *= *it;
                    }
                    sliced->data = make_shared<Array>();
                    for (long long i = begin; i < end; i++) {
                        sliced->data->push_back((*array->data)[i]);
                    }
                    stack->push_back(sliced);
                }
                ++pc;
                break;
            }
            case CALL_GETINT: {
                int n;
                cin >> n;
                stack->push_back(make_shared<IntObject>(n));
                ++pc;
                break;
            }
            case JUMP_IF_FALSE_OR_POP:
                break;
            case JUMP_IF_TRUE_OR_POP:
                break;
            case CALL_PRINTF: {
                FormatStringP fmt_str = cast<PrintF>(*pc)->format_string;
                auto seg_it = fmt_str->segments.begin();
                outs << *seg_it;
                for (auto it = stack->end() - fmt_str->fmt_char_cnt; it != stack->end(); ++it) {
                    outs << cast<IntObject>(*it)->value << *++seg_it;
                }
                for (int i = 0; i < fmt_str->fmt_char_cnt; i++) {
                    stack->pop_back();
                }
                ++pc;
                break;
            }
            case EXIT_INTERP:
                return;
            case CALL_FUNCTION: {  // 由于编译时已为所有变量分配好空间，且和 Python 一样根据名字找变量，函数传参放入变量池而非栈
                StackP new_frame = make_shared<Stack>();
                new_frame->push_back(make_shared<IntObject>(pc - instructions.begin() + 1));
                FuncObjectP func = cast<CallFunction>(*pc)->get_func();
                auto &params = func->params;
                for (auto i = (long long) params.size() - 1; i >= 0; i--) {
                    ObjectP o = stack->back();
                    switch (o->type) {
                        case INT:
                            cast<IntObject>(params[i])->value = cast<IntObject>(o)->value;
                            break;
                        case INT_ARRAY:
                            cast<ArrayObject>(params[i])->data = cast<ArrayObject>(o)->data;
                            break;  // note the dimension difference when addressing
                        default:
                            ERROR_LIMITED_SUPPORT(INT or INT_ARRAY function arguments);
                    }
                    stack->pop_back();
                }
                stack = new_frame;
                frames.push_back(new_frame);
                pc = instructions.begin() + func->code_offset;
                break;
            }
            case RETURN_VALUE: {
                if (stack->size() > 1) {
                    assert(stack->size() == 2);
                    ObjectP value = stack->back();
                    stack->pop_back();
                    pc = instructions.begin() + cast<IntObject>(stack->back())->value;
                    frames.pop_back();
                    stack = frames.back();
                    stack->push_back(value);
                } else {
                    pc = instructions.begin() + cast<IntObject>(stack->back())->value;
                    frames.pop_back();
                    stack = frames.back();
                }
                break;
            }
            case UNARY_OP: {
                long long value = cast<IntObject>(stack->back())->value;
                stack->pop_back();
                stack->push_back(make_shared<IntObject>(util::unary_operation(cast<UnaryOperation>(*pc)->unary_opcode, value)));
                ++pc;
                break;
            }
            case BINARY_OP: {
                long long right = cast<IntObject>(stack->back())->value;
                stack->pop_back();
                long long left = cast<IntObject>(stack->back())->value;
                stack->pop_back();
                stack->push_back(make_shared<IntObject>(util::binary_operation(cast<BinaryOperation>(*pc)->binary_opcode, left, right)));
                ++pc;
                break;
            }
            default:
                ++pc;
        }
    }
}
